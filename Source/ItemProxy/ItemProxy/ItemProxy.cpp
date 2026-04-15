#include "ItemProxy.h"

FBasicProxy::~FBasicProxy()
{
}

bool FBasicProxy::NetSerialize(
	FArchive& Ar,
	class UPackageMap* Map,
	bool& bOutSuccess
	)
{
	if (Ar.IsLoading())
	{
		auto Num = ValuesMap.Num();
		Ar << Num;

		for (auto Iter : ValuesMap)
		{
			Ar << Iter;
		}
	}
	else
	{
		auto Num = ValuesMap.Num();
		Ar << Num;
		
	}
	bOutSuccess = true;
	return true;
}

FGameplayTag FProxyStrategy::GetTag() const
{
	return FGameplayTag::EmptyTag;
}

TSharedPtr<FBasicProxy> FProxyStrategy::GetProxy() const
{
	return nullptr;
}
