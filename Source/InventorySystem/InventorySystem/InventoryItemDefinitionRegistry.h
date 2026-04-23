#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class UItemDefine;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemDefineLoaded, const FGameplayTag&, const UItemDefine*);

class INVENTORYSYSTEM_API FInventoryItemDefinitionRegistry
{
public:
	static FInventoryItemDefinitionRegistry& Get();

	void Initialize();
	const UItemDefine* FindItemDefineByTag(const FGameplayTag& ItemTag) const;
	FDelegateHandle AddOnItemDefineLoaded(const FOnInventoryItemDefineLoaded::FDelegate& Delegate);
	void RemoveOnItemDefineLoaded(FDelegateHandle DelegateHandle);

private:
	void HandlePrimaryAssetLoaded(FPrimaryAssetId AssetId);

private:
	bool bInitializationStarted = false;
	TMap<FGameplayTag, TWeakObjectPtr<UItemDefine>> LoadedItemDefines;
	FOnInventoryItemDefineLoaded OnItemDefineLoaded;
};
