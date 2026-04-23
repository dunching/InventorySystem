#include "ItemProxy_State.h"
#include "GameplayTagsLibrary.h"

bool FItemProxy_State::NetSerialize(
	FArchive& Ar,
	class UPackageMap* Map,
	bool& bOutSuccess
	)
{
	// 通用状态代理仅复用基础序列化：
	// ProxyId / ItemTag / Count / ValuesMap。
	return FBasicProxy::NetSerialize(Ar, Map, bOutSuccess);
}

void FItemProxy_State::SetStateValue(
	const FGameplayTag& StateTag,
	const float Value
	)
{
	if (!StateTag.IsValid())
	{
		return;
	}

	ValuesMap.Add(StateTag, Value);
}

float FItemProxy_State::GetStateValue(
	const FGameplayTag& StateTag,
	const float DefaultValue
	) const
{
	if (!StateTag.IsValid())
	{
		return DefaultValue;
	}

	if (const float* FoundValue = ValuesMap.Find(StateTag))
	{
		return *FoundValue;
	}

	return DefaultValue;
}

void FItemProxy_State::SetStateBool(
	const FGameplayTag& StateTag,
	const bool bValue
	)
{
	SetStateValue(StateTag, bValue ? 1.0f : 0.0f);
}

bool FItemProxy_State::GetStateBool(
	const FGameplayTag& StateTag
	) const
{
	return GetStateValue(StateTag, 0.0f) > 0.0f;
}

FGameplayTag FProxy_StateStrategy::GetTag() const
{
	return USmartCitySuiteTags::Item_State;
}

TSharedPtr<FBasicProxy> FProxy_StateStrategy::GetProxy() const
{
	return MakeShared<FItemProxy_State>();
}
