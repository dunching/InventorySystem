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
	// 库存条目的核心状态。
	Ar << ProxyId;
	Ar << ItemTag;
	Ar << Count;

	int32 ValueNum = ValuesMap.Num();
	Ar << ValueNum;

	if (Ar.IsLoading())
	{
		// 反序列化时重建键值数据。
		ValuesMap.Empty(ValueNum);
		for (int32 Index = 0; Index < ValueNum; ++Index)
		{
			FGameplayTag Tag;
			float Value = 0.f;
			Ar << Tag;
			Ar << Value;
			ValuesMap.Add(Tag, Value);
		}
	}
	else
	{
		// 序列化时写出所有键值对。
		for (const TPair<FGameplayTag, float>& Iter : ValuesMap)
		{
			FGameplayTag Tag = Iter.Key;
			float Value = Iter.Value;
			Ar << Tag;
			Ar << Value;
		}
	}

	if (!ProxyId.IsValid())
	{
		ProxyId = FGuid::NewGuid();
	}
	Count = FMath::Max(1, Count);

	bOutSuccess = true;
	return true;
}

FGameplayTag FProxyStrategy::GetProxySchemaTag() const
{
	return FGameplayTag::EmptyTag;
}

TSharedPtr<FBasicProxy> FProxyStrategy::GetProxy() const
{
	return nullptr;
}
