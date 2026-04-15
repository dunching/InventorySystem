#include "InventoryComponent.h"

#include "Engine/AssetManager.h"

#include "ItemDefine.h"
#include "ItemProxy.h"
#include "ItemProxy_Equipment.h"
#include "ItemProxy_Modifier.h"

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
			                                                                auto Obj = UAssetManager::Get().
				                                                                GetPrimaryAssetObject(Id);

			                                                                if (Obj)
			                                                                {
				                                                                if (auto Data = Cast<UItemDefine>(Obj))
				                                                                {
					                                                                AllItemDefineMap.Add(
						                                                                 Data->ItemTag,
						                                                                 Data
						                                                                );
				                                                                }
			                                                                }
		                                                                }
		                                                               )
		                             );
	}

	AddGetProxyMetaStrategy(MakeShared<FProxy_EquipmentStrategy>());
	AddGetProxyMetaStrategy(MakeShared<FProxy_ModifierStrategy>());
}

TWeakPtr<FBasicProxy> UInventoryComponent::AddProxy(
	const FGameplayTag& ProxyType,
	uint8 Num
	)
{
#if UE_EDITOR || UE_CLIENT
	if (GetOwnerRole() < ROLE_Authority)
	{
		AddProxy_Server(ProxyType, Num);
		return nullptr;
	}
#endif

	if (auto ItemDefinePtr = AllItemDefineMap.Find(ProxyType))
	{
		if ((*ItemDefinePtr)->StatckCount > 1)
		{
			for (auto& Iter : ProxysAry)
			{
				if (Iter->ItemDefine->ItemTag.MatchesTag(ProxyType))
				{
					return Iter;
				}
			}
		}
		else
		{
			TSharedPtr<FBasicProxy> ItemProxySPtr = nullptr;
			if (auto Iter = GetProxyMetaStrategies.Find(ProxyType))
			{
				ItemProxySPtr = (*Iter)->GetProxy();
			}
			else
			{
				ItemProxySPtr = MakeShared<FBasicProxy>();
			}

			ItemProxySPtr->ItemDefine = *ItemDefinePtr;

			ProxysAry.Add(ItemProxySPtr);
		}
	}

	return nullptr;
}

void UInventoryComponent::AddGetProxyMetaStrategy(
	const TSharedPtr<FProxyStrategy>& ProxyMetaStrategyFunc
	)
{
	GetProxyMetaStrategies.Add(ProxyMetaStrategyFunc->GetTag(), ProxyMetaStrategyFunc);
}

#if WITH_EDITORONLY_DATA || UE_CLIENT
void UInventoryComponent::AddProxy_Server_Implementation(
	const FGameplayTag& ProxyType,
	uint8 Num
	)
{
	AddProxy(ProxyType, Num);
}
#endif
