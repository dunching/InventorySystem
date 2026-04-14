#include "InventoryComponent.h"

#include "Engine/AssetManager.h"

#include "ItemDefine.h"

TSharedPtr<FBasicProxy> UInventoryComponent::AddProxy(
	const FGameplayTag& ProxyType,
	uint8 Num
	)
{
#if UE_EDITOR || UE_CLIENT
	if (GIsClient)
	{
		AddProxy_Server(ProxyType, Num);
		return nullptr;
	}
#endif

	UAssetManager& AssetManager = UAssetManager::Get();

	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(ItemDefineDataAssetType, AssetIds);

	for (const FPrimaryAssetId& Id : AssetIds)
	{
		UObject* Obj = AssetManager.GetPrimaryAssetObject(Id);
		if (!Obj)
		{
			Obj = AssetManager.LoadPrimaryAsset(Id)->GetLoadedAsset();
		}

		if (auto Data = Cast<UItemDefine>(Obj))
		{
			AllItemDefineMap.Add(Data->ItemTag, Data);
		}
	}

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
