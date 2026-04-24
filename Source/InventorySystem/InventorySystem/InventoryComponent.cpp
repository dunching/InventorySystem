#include "InventoryComponent.h"
#include "InventoryItemDefinitionRegistry.h"
#include "InventoryProxyRegistry.h"
#include "ItemDefine.h"
#include "ItemProxy.h"
#include "Net/UnrealNetwork.h"

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

	FInventoryItemDefinitionRegistry& ItemRegistry = FInventoryItemDefinitionRegistry::Get();
	ItemDefineLoadedHandle = ItemRegistry.AddOnItemDefineLoaded(
		FOnInventoryItemDefineLoaded::FDelegate::CreateUObject(this, &ThisClass::HandleItemDefineLoaded));
	ItemRegistry.Initialize();

	// 具体 Proxy 策略由外部模块注册；库存系统只消费注册表。
	for (const TSharedPtr<FProxyStrategy>& Strategy : FInventoryProxyRegistry::Get().GetStrategies())
	{
		AddGetProxyMetaStrategy(Strategy);
	}
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ItemDefineLoadedHandle.IsValid())
	{
		FInventoryItemDefinitionRegistry::Get().RemoveOnItemDefineLoaded(ItemDefineLoadedHandle);
		ItemDefineLoadedHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
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

	if (const UItemDefine* ItemDefine = FInventoryItemDefinitionRegistry::Get().FindItemDefineByTag(ProxyType))
	{
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
	return FInventoryItemDefinitionRegistry::Get().FindItemDefineByTag(ItemTag);
}

void UInventoryComponent::AddGetProxyMetaStrategy(
	const TSharedPtr<FProxyStrategy>& ProxyMetaStrategyFunc
	)
{
	if (ProxyMetaStrategyFunc.IsValid())
	{
		ProxyStrategiesBySchema.Add(ProxyMetaStrategyFunc->GetProxySchemaTag(), ProxyMetaStrategyFunc);
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
	if (const TSharedPtr<FProxyStrategy>* Iter = ProxyStrategiesBySchema.Find(ProxyType))
	{
		return *Iter;
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
	if (TSharedPtr<FProxyStrategy> Strategy = FindProxyMetaStrategy(ItemDefine->GetInventoryProxySchemaTag()))
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
	ItemProxySPtr->ProxySchemaTag = ItemDefine->GetInventoryProxySchemaTag();
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

	if (const UItemDefine* ItemDefine = FInventoryItemDefinitionRegistry::Get().FindItemDefineByTag(ProxySPtr->ItemTag))
	{
		ProxySPtr->ItemDefine = const_cast<UItemDefine*>(ItemDefine);
	}
}

void UInventoryComponent::HandleItemDefineLoaded(const FGameplayTag& ItemTag, const UItemDefine* ItemDefine)
{
	if (!ItemTag.IsValid() || !ItemDefine)
	{
		return;
	}

	for (const TSharedPtr<FBasicProxy>& ProxySPtr : ProxysAry)
	{
		SyncProxyDefine(ProxySPtr);
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		ReplayPendingAddRequests(ItemTag);
	}
}

void UInventoryComponent::ReplayPendingAddRequests(const FGameplayTag& ItemTag)
{
	int32 PendingCount = 0;
	if (!PendingAddRequests.RemoveAndCopyValue(ItemTag, PendingCount) || PendingCount <= 0)
	{
		return;
	}

	while (PendingCount > 0)
	{
		const int32 BatchNum = FMath::Min(PendingCount, static_cast<int32>(TNumericLimits<uint8>::Max()));
		AddProxy(ItemTag, static_cast<uint8>(BatchNum));
		PendingCount -= BatchNum;
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
