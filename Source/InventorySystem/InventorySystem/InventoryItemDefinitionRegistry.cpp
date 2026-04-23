#include "InventoryItemDefinitionRegistry.h"
#include "Engine/AssetManager.h"
#include "ItemDefine.h"

FInventoryItemDefinitionRegistry& FInventoryItemDefinitionRegistry::Get()
{
	static FInventoryItemDefinitionRegistry Instance;
	return Instance;
}

void FInventoryItemDefinitionRegistry::Initialize()
{
	if (bInitializationStarted)
	{
		return;
	}

	bInitializationStarted = true;

	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(ItemDefineDataAssetType, AssetIds);

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		AssetManager.LoadPrimaryAsset(
			AssetId,
			{},
			FStreamableDelegate::CreateRaw(this, &FInventoryItemDefinitionRegistry::HandlePrimaryAssetLoaded, AssetId));
	}
}

const UItemDefine* FInventoryItemDefinitionRegistry::FindItemDefineByTag(const FGameplayTag& ItemTag) const
{
	if (!ItemTag.IsValid())
	{
		return nullptr;
	}

	if (const TWeakObjectPtr<UItemDefine>* Found = LoadedItemDefines.Find(ItemTag))
	{
		return Found->Get();
	}

	return nullptr;
}

FDelegateHandle FInventoryItemDefinitionRegistry::AddOnItemDefineLoaded(const FOnInventoryItemDefineLoaded::FDelegate& Delegate)
{
	return OnItemDefineLoaded.Add(Delegate);
}

void FInventoryItemDefinitionRegistry::RemoveOnItemDefineLoaded(FDelegateHandle DelegateHandle)
{
	OnItemDefineLoaded.Remove(DelegateHandle);
}

void FInventoryItemDefinitionRegistry::HandlePrimaryAssetLoaded(const FPrimaryAssetId AssetId)
{
	if (UObject* AssetObject = UAssetManager::Get().GetPrimaryAssetObject(AssetId))
	{
		if (UItemDefine* ItemDefine = Cast<UItemDefine>(AssetObject))
		{
			LoadedItemDefines.Add(ItemDefine->ItemTag, ItemDefine);
			OnItemDefineLoaded.Broadcast(ItemDefine->ItemTag, ItemDefine);
		}
	}
}
