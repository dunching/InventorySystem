#include "InventoryComponent.h"

#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"

#include "ItemDefine.h"
#include "ItemProxy.h"
#include "ItemProxy_Equipment.h"
#include "ItemProxy_Modifier.h"

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

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

	AddGetProxyMetaStrategy(MakeShared<FProxy_EquipmentStrategy>());
	AddGetProxyMetaStrategy(MakeShared<FProxy_ModifierStrategy>());
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
	// 库存修改遵循服务器权威。
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
			// 先填满已有堆叠，减少条目数量。
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
				Proxy_FASI_Container.UpdateItem(Iter);

				if (!FirstChangedProxy.IsValid())
				{
					FirstChangedProxy = Iter;
				}

				if (PendingNum <= 0)
				{
					return FirstChangedProxy;
				}
			}

			while (PendingNum > 0)
			{
				// 剩余数量拆分为新堆叠。
				const int32 NewCount = FMath::Min(StackLimit, PendingNum);
				TSharedPtr<FBasicProxy> ItemProxySPtr = CreateProxyInstance(ItemDefine);
				if (!ItemProxySPtr.IsValid())
				{
					break;
				}

				ItemProxySPtr->Count = NewCount;
				ProxysAry.Add(ItemProxySPtr);
				Proxy_FASI_Container.AddItem(ItemProxySPtr);

				if (!FirstChangedProxy.IsValid())
				{
					FirstChangedProxy = ItemProxySPtr;
				}

				PendingNum -= NewCount;
			}

			return FirstChangedProxy;
		}

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
	// 库存修改遵循服务器权威。
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

		if (ProxySPtr->Count > PendingNum)
		{
			ProxySPtr->Count -= PendingNum;
			Proxy_FASI_Container.UpdateItem(ProxySPtr);
			PendingNum = 0;
			break;
		}

		PendingNum -= ProxySPtr->Count;
		Proxy_FASI_Container.RemoveItem(ProxySPtr);
		ProxysAry.RemoveAt(Index);
	}

	return PendingNum == 0;
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
	// 客户端收到 FastArray 后重建本地缓存。
	RefreshProxyCacheFromContainer();
}

TSharedPtr<FProxyStrategy> UInventoryComponent::FindProxyMetaStrategy(
	const FGameplayTag& ProxyType
	) const
{
	if (const TSharedPtr<FProxyStrategy>* Iter = GetProxyMetaStrategies.Find(ProxyType))
	{
		return *Iter;
	}

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
}
