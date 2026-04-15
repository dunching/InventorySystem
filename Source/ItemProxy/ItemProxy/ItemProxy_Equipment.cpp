#include "ItemProxy_Equipment.h"

#include "GameplayTagsLibrary.h"

bool FItemProxy_Equipment::NetSerialize(
	FArchive& Ar,
	class UPackageMap* Map,
	bool& bOutSuccess
	)
{
	return FBasicProxy::NetSerialize(Ar, Map, bOutSuccess);
}

FGameplayTag FProxy_EquipmentStrategy::GetTag() const
{
	return USmartCitySuiteTags::Item_Equipment_Test;
}

TSharedPtr<FBasicProxy> FProxy_EquipmentStrategy::GetProxy() const
{
	return MakeShared<FItemProxy_Equipment>();
}
