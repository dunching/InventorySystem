#include "InventoryComponent.h"

#include "Engine/AssetManager.h"

#include "ItemDefine.h"

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	auto & AssetManager = UAssetManager::Get();

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
			                                                                UObject* Obj = UAssetManager::Get().
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
		                                                               ));
	}
}

TSharedPtr<FBasicProxy> UInventoryComponent::AddProxy(
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

	return nullptr;
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
