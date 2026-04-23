#pragma once

#include "CoreMinimal.h"

struct FProxyStrategy;

struct INVENTORYSYSTEM_API FInventoryProxyRegistryEntry
{
	FName OwnerName;
	TSharedPtr<FProxyStrategy> Strategy;
};

class INVENTORYSYSTEM_API FInventoryProxyRegistry
{
public:
	static FInventoryProxyRegistry& Get();

	void RegisterStrategy(FName OwnerName, const TSharedPtr<FProxyStrategy>& Strategy);
	void UnregisterStrategies(FName OwnerName);
	TArray<TSharedPtr<FProxyStrategy>> GetStrategies() const;

private:
	TArray<FInventoryProxyRegistryEntry> Entries;
};
