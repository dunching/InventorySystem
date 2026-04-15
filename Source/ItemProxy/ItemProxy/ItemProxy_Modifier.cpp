#include "ItemProxy_Modifier.h"

#include "GameplayTagsLibrary.h"

bool FItemProxy_Modifier::NetSerialize(
	FArchive& Ar,
	class UPackageMap* Map,
	bool& bOutSuccess
	)
{
	bOutSuccess = true;
	return true;
}

FGameplayTag FProxy_ModifierStrategy::GetTag() const
{
	return USmartCitySuiteTags::Item_Modifier_Test;
}

TSharedPtr<FBasicProxy> FProxy_ModifierStrategy::GetProxy() const
{
	return MakeShared<FItemProxy_Modifier>();
}
