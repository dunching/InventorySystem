#include "ItemProxy_Modifier.h"
#include "GameplayTagsLibrary.h"

bool FItemProxy_Modifier::NetSerialize(
	FArchive& Ar,
	class UPackageMap* Map,
	bool& bOutSuccess
	)
{
	// 强化 Proxy 目前沿用基础序列化逻辑。
	return FBasicProxy::NetSerialize(Ar, Map, bOutSuccess);
}

FGameplayTag FProxy_ModifierStrategy::GetTag() const
{
	return USmartCitySuiteTags::Item_Modifier_Test;
}

TSharedPtr<FBasicProxy> FProxy_ModifierStrategy::GetProxy() const
{
	return MakeShared<FItemProxy_Modifier>();
}
