#include "InventoryComponent.h"

#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"

#include "ItemDefine.h"
#include "ItemProxy.h"
#include "ItemProxy_Equipment.h"
#include "ItemProxy_Modifier.h"
#include "ItemProxy_State.h"

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 绑定 FastArray 增量回调：
	// - 服务端改容器
	// - 客户端靠回调增量更新 ProxysAry
	Proxy_FASI_Container.OnAdd.RemoveAll(this);
	Proxy_FASI_Container.OnChange.RemoveAll(this);
	Proxy_FASI_Container.OnRemove.RemoveAll(this);
	Proxy_FASI_Container.OnAdd.AddUObject(this, &ThisClass::HandleProxyAdded);
	Proxy_FASI_Container.OnChange.AddUObject(this, &ThisClass::HandleProxyChanged);
	Proxy_FASI_Container.OnRemove.AddUObject(this, &ThisClass::HandleProxyRemoved);

	// 预加载 ItemDefine 原型数据（后续用于 ItemTag -> Define 映射）。
	auto& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(ItemDefineDataAssetType, AssetIds);

	for (const FPrimaryAssetId& Id : AssetIds)
	{
		AssetManager.LoadPrimaryAsset(
			Id,
			{},
			FStreamableDelegate::CreateLambda(
				[Id, this]()
				{
					if (UObject* Obj = UAssetManager::Get().GetPrimaryAssetObject(Id))
					{
						if (UItemDefine* Data = Cast<UItemDefine>(Obj))
						{
							AllItemDefineMap.Add(Data->ItemTag, Data);

							// Define 迟加载完成后，补齐已存在 Proxy 的 ItemDefine 指针。
							for (const TSharedPtr<FBasicProxy>& ProxySPtr : ProxysAry)
							{
								SyncProxyDefine(ProxySPtr);
							}

							// 服务端在定义到位后回放排队的 Add 请求，避免早期调用丢失。
							if (GetOwner() && GetOwner()->HasAuthority())
							{
								int32 PendingCount = 0;
								if (PendingAddRequests.RemoveAndCopyValue(Data->ItemTag, PendingCount) && PendingCount > 0)
								{
									while (PendingCount > 0)
									{
										const int32 BatchNum = FMath::Min(PendingCount, static_cast<int32>(TNumericLimits<uint8>::Max()));
										AddProxy(Data->ItemTag, static_cast<uint8>(BatchNum));
										PendingCount -= BatchNum;
									}
								}
							}
						}
					}
				}
			)
		);
	}

	// 注册默认策略：装备类、强化类。
	AddGetProxyMetaStrategy(MakeShared<FProxy_EquipmentStrategy>());
	AddGetProxyMetaStrategy(MakeShared<FProxy_ModifierStrategy>());
	AddGetProxyMetaStrategy(MakeShared<FProxy_StateStrategy>());
}

void UInventoryComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
	) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, Proxy_FASI_Container);
}

TWeakPtr<FBasicProxy> UInventoryComponent::AddProxy(
	const FGameplayTag& ProxyType,
	uint8 Num
	)
{
	// 库存改动由服务端权威执行，客户端仅发请求。
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		AddProxy_Server(ProxyType, Num);
		return nullptr;
	}

	if (!ProxyType.IsValid() || Num == 0)
	{
		return nullptr;
	}

	if (TObjectPtr<UItemDefine>* ItemDefinePtr = AllItemDefineMap.Find(ProxyType))
	{
		UItemDefine* ItemDefine = *ItemDefinePtr;
		if (!ItemDefine)
		{
			return nullptr;
		}

		const int32 StackLimit = ItemDefine->GetStackLimit();
		int32 PendingNum = Num;
		TSharedPtr<FBasicProxy> FirstChangedProxy = nullptr;

		if (StackLimit > 1)
		{
			// 先填已有堆叠，降低条目数量和复制开销。
			for (const TSharedPtr<FBasicProxy>& Iter : ProxysAry)
			{
				if (!Iter.IsValid() || !Iter->ItemTag.MatchesTagExact(ProxyType) || Iter->Count >= StackLimit)
				{
					continue;
				}

				const int32 CanAdd = FMath::Min(StackLimit - Iter->Count, PendingNum);
				if (CanAdd <= 0)
				{
					continue;
				}

				Iter->Count += CanAdd;
				PendingNum -= CanAdd;
				Proxy_FASI_Container.UpdateItem(Iter); // 标记变更复制

				if (!FirstChangedProxy.IsValid())
				{
					FirstChangedProxy = Iter;
				}

				if (PendingNum <= 0)
				{
					NotifyInventoryChanged();
					return FirstChangedProxy;
				}
			}

			// 余量拆新堆叠。
			while (PendingNum > 0)
			{
				const int32 NewCount = FMath::Min(StackLimit, PendingNum);
				TSharedPtr<FBasicProxy> ItemProxySPtr = CreateProxyInstance(ItemDefine);
				if (!ItemProxySPtr.IsValid())
				{
					break;
				}

				ItemProxySPtr->Count = NewCount;
				ProxysAry.Add(ItemProxySPtr);
				Proxy_FASI_Container.AddItem(ItemProxySPtr); // 标记新增复制

				if (!FirstChangedProxy.IsValid())
				{
					FirstChangedProxy = ItemProxySPtr;
				}

				PendingNum -= NewCount;
			}

			NotifyInventoryChanged();
			return FirstChangedProxy;
		}

		// 不可堆叠：逐个创建条目。
		for (int32 Index = 0; Index < PendingNum; ++Index)
		{
			TSharedPtr<FBasicProxy> ItemProxySPtr = CreateProxyInstance(ItemDefine);
			if (!ItemProxySPtr.IsValid())
			{
				break;
			}

			ProxysAry.Add(ItemProxySPtr);
			Proxy_FASI_Container.AddItem(ItemProxySPtr);

			if (!FirstChangedProxy.IsValid())
			{
				FirstChangedProxy = ItemProxySPtr;
			}
		}

		NotifyInventoryChanged();
		return FirstChangedProxy;
	}

	// ItemDefine 仍未完成异步加载时，先记录请求并等待回放。
	int32& PendingCount = PendingAddRequests.FindOrAdd(ProxyType);
	PendingCount += Num;

	return nullptr;
}

bool UInventoryComponent::RemoveProxy(
	const FGameplayTag& ProxyType,
	uint8 Num
	)
{
	// 库存改动由服务端权威执行，客户端仅发请求。
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		RemoveProxy_Server(ProxyType, Num);
		return false;
	}

	if (!ProxyType.IsValid() || Num == 0)
	{
		return false;
	}

	int32 PendingNum = Num;
	for (int32 Index = ProxysAry.Num() - 1; Index >= 0 && PendingNum > 0; --Index)
	{
		const TSharedPtr<FBasicProxy>& ProxySPtr = ProxysAry[Index];
		if (!ProxySPtr.IsValid() || !ProxySPtr->ItemTag.MatchesTagExact(ProxyType))
		{
			continue;
		}

		// 当前堆叠够减：仅更新数量并标记变更复制。
		if (ProxySPtr->Count > PendingNum)
		{
			ProxySPtr->Count -= PendingNum;
			Proxy_FASI_Container.UpdateItem(ProxySPtr);
			PendingNum = 0;
			NotifyInventoryChanged();
			break;
		}

		// 当前堆叠被清空：删除条目并标记数组变化复制。
		PendingNum -= ProxySPtr->Count;
		Proxy_FASI_Container.RemoveItem(ProxySPtr);
		ProxysAry.RemoveAt(Index);
	}

	const bool bFullyRemoved = PendingNum == 0;
	if (bFullyRemoved)
	{
		NotifyInventoryChanged();
	}

	return bFullyRemoved;
}

int32 UInventoryComponent::GetProxyCount(
	const FGameplayTag& ProxyType
	) const
{
	if (!ProxyType.IsValid())
	{
		return 0;
	}

	int32 TotalCount = 0;
	for (const TSharedPtr<FBasicProxy>& ProxySPtr : ProxysAry)
	{
		if (ProxySPtr.IsValid() && ProxySPtr->ItemTag.MatchesTagExact(ProxyType))
		{
			TotalCount += ProxySPtr->Count;
		}
	}

	return TotalCount;
}

const TArray<TSharedPtr<FBasicProxy>>& UInventoryComponent::GetAllProxyList() const
{
	return ProxysAry;
}

const UItemDefine* UInventoryComponent::FindItemDefineByTag(const FGameplayTag& ItemTag) const
{
	if (!ItemTag.IsValid())
	{
		return nullptr;
	}

	if (const TObjectPtr<UItemDefine>* Found = AllItemDefineMap.Find(ItemTag))
	{
		return *Found;
	}
	return nullptr;
}

void UInventoryComponent::AddGetProxyMetaStrategy(
	const TSharedPtr<FProxyStrategy>& ProxyMetaStrategyFunc
	)
{
	if (ProxyMetaStrategyFunc.IsValid())
	{
		GetProxyMetaStrategies.Add(ProxyMetaStrategyFunc->GetTag(), ProxyMetaStrategyFunc);
	}
}

void UInventoryComponent::AddProxy_Server_Implementation(
	const FGameplayTag& ProxyType,
	uint8 Num
	)
{
	AddProxy(ProxyType, Num);
}

void UInventoryComponent::RemoveProxy_Server_Implementation(
	const FGameplayTag& ProxyType,
	uint8 Num
	)
{
	RemoveProxy(ProxyType, Num);
}

void UInventoryComponent::OnRep_ProxyContainer()
{
	// 兜底同步：保证本地缓存与容器一致（防增量回调漏处理）。
	RefreshProxyCacheFromContainer();
}

void UInventoryComponent::HandleProxyAdded(
	const TSharedPtr<FBasicProxy>& ProxySPtr
	)
{
	if (!ProxySPtr.IsValid())
	{
		return;
	}

	SyncProxyDefine(ProxySPtr);

	// 去重：同 ProxyId 不重复加入缓存。
	for (const TSharedPtr<FBasicProxy>& Iter : ProxysAry)
	{
		if (Iter.IsValid() && Iter->ProxyId == ProxySPtr->ProxyId)
		{
			return;
		}
	}

	ProxysAry.Add(ProxySPtr);
	NotifyInventoryChanged();
}

void UInventoryComponent::HandleProxyChanged(
	const TSharedPtr<FBasicProxy>& ProxySPtr
	)
{
	if (!ProxySPtr.IsValid())
	{
		return;
	}

	SyncProxyDefine(ProxySPtr);

	for (TSharedPtr<FBasicProxy>& Iter : ProxysAry)
	{
		if (Iter.IsValid() && Iter->ProxyId == ProxySPtr->ProxyId)
		{
			Iter = ProxySPtr;
			NotifyInventoryChanged();
			return;
		}
	}

	// 容错：若本地未命中，按新增处理，避免状态丢失。
	ProxysAry.Add(ProxySPtr);
	NotifyInventoryChanged();
}

void UInventoryComponent::HandleProxyRemoved(
	const TSharedPtr<FBasicProxy>& ProxySPtr
	)
{
	if (!ProxySPtr.IsValid())
	{
		return;
	}

	ProxysAry.RemoveAll([&ProxySPtr](const TSharedPtr<FBasicProxy>& Iter)
	{
		return Iter.IsValid() && Iter->ProxyId == ProxySPtr->ProxyId;
	});
	NotifyInventoryChanged();
}

TSharedPtr<FProxyStrategy> UInventoryComponent::FindProxyMetaStrategy(
	const FGameplayTag& ProxyType
	) const
{
	// 优先精确匹配。
	if (const TSharedPtr<FProxyStrategy>* Iter = GetProxyMetaStrategies.Find(ProxyType))
	{
		return *Iter;
	}

	// 其次父标签匹配（允许用更宽泛标签覆盖多个子类道具）。
	for (const TPair<FGameplayTag, TSharedPtr<FProxyStrategy>>& Iter : GetProxyMetaStrategies)
	{
		if (ProxyType.MatchesTag(Iter.Key))
		{
			return Iter.Value;
		}
	}

	return nullptr;
}

TSharedPtr<FBasicProxy> UInventoryComponent::CreateProxyInstance(
	const UItemDefine* ItemDefine
	) const
{
	if (!ItemDefine)
	{
		return nullptr;
	}

	TSharedPtr<FBasicProxy> ItemProxySPtr = nullptr;
	if (TSharedPtr<FProxyStrategy> Strategy = FindProxyMetaStrategy(ItemDefine->ItemTag))
	{
		ItemProxySPtr = Strategy->GetProxy();
	}

	// 无策略时使用基础 Proxy。
	if (!ItemProxySPtr.IsValid())
	{
		ItemProxySPtr = MakeShared<FBasicProxy>();
	}

	ItemProxySPtr->ItemDefine = const_cast<UItemDefine*>(ItemDefine);
	ItemProxySPtr->ItemTag = ItemDefine->ItemTag;
	ItemProxySPtr->Count = 1;
	if (!ItemProxySPtr->ProxyId.IsValid())
	{
		ItemProxySPtr->ProxyId = FGuid::NewGuid();
	}

	return ItemProxySPtr;
}

void UInventoryComponent::SyncProxyDefine(
	const TSharedPtr<FBasicProxy>& ProxySPtr
	)
{
	if (!ProxySPtr.IsValid() || ProxySPtr->ItemDefine)
	{
		return;
	}

	if (TObjectPtr<UItemDefine>* ItemDefinePtr = AllItemDefineMap.Find(ProxySPtr->ItemTag))
	{
		ProxySPtr->ItemDefine = *ItemDefinePtr;
	}
}

void UInventoryComponent::RefreshProxyCacheFromContainer()
{
	ProxysAry.Reset(Proxy_FASI_Container.Items.Num());
	for (FProxy_FASI& Item : Proxy_FASI_Container.Items)
	{
		if (!Item.ProxySPtr.IsValid())
		{
			continue;
		}

		SyncProxyDefine(Item.ProxySPtr);
		ProxysAry.Add(Item.ProxySPtr);
	}

	NotifyInventoryChanged();
}

void UInventoryComponent::NotifyInventoryChanged()
{
	InventoryProxyStateChangedEvent.Broadcast();
}
