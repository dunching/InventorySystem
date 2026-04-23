#include "InventoryProxyRegistry.h"
#include "ItemProxy.h"

FInventoryProxyRegistry& FInventoryProxyRegistry::Get()
{
	static FInventoryProxyRegistry Registry;
	return Registry;
}

void FInventoryProxyRegistry::RegisterStrategy(const FName OwnerName, const TSharedPtr<FProxyStrategy>& Strategy)
{
	if (OwnerName.IsNone() || !Strategy.IsValid())
	{
		return;
	}

	FInventoryProxyRegistryEntry& Entry = Entries.AddDefaulted_GetRef();
	Entry.OwnerName = OwnerName;
	Entry.Strategy = Strategy;
}

void FInventoryProxyRegistry::UnregisterStrategies(const FName OwnerName)
{
	if (OwnerName.IsNone())
	{
		return;
	}

	Entries.RemoveAll([OwnerName](const FInventoryProxyRegistryEntry& Entry)
	{
		return Entry.OwnerName == OwnerName;
	});
}

TArray<TSharedPtr<FProxyStrategy>> FInventoryProxyRegistry::GetStrategies() const
{
	TArray<TSharedPtr<FProxyStrategy>> Result;
	Result.Reserve(Entries.Num());

	for (const FInventoryProxyRegistryEntry& Entry : Entries)
	{
		if (Entry.Strategy.IsValid())
		{
			Result.Add(Entry.Strategy);
		}
	}

	return Result;
}
