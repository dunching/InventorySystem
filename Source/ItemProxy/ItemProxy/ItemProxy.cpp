#include "ItemProxy.h"

namespace
{
	TMap<FGameplayTag, TSharedPtr<FProxyStrategy>>& GetGlobalProxyStrategies()
	{
		static TMap<FGameplayTag, TSharedPtr<FProxyStrategy>> Strategies;
		return Strategies;
	}
}

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

bool FBasicProxy::IsInUse() const
{
	return false;
}

bool FBasicProxy::IsEquipmentProxy() const
{
	return false;
}

void FBasicProxy::SetInstalledModifierProxyIds(const TArray<FGuid>& InModifierProxyIds)
{
}

const TArray<FGuid>& FBasicProxy::GetInstalledModifierProxyIds() const
{
	static const TArray<FGuid> EmptyModifierProxyIds;
	return EmptyModifierProxyIds;
}

FGameplayTag FProxyStrategy::GetProxySchemaTag() const
{
	return FGameplayTag::EmptyTag;
}

TSharedPtr<FBasicProxy> FProxyStrategy::GetProxy() const
{
	return nullptr;
}

void FProxyStrategy::RegisterGlobalStrategy(const TSharedPtr<FProxyStrategy>& Strategy)
{
	if (!Strategy.IsValid())
	{
		return;
	}

	const FGameplayTag SchemaTag = Strategy->GetProxySchemaTag();
	if (!SchemaTag.IsValid())
	{
		return;
	}

	GetGlobalProxyStrategies().Add(SchemaTag, Strategy);
}

void FProxyStrategy::UnregisterGlobalStrategy(const FGameplayTag& ProxySchemaTag)
{
	if (!ProxySchemaTag.IsValid())
	{
		return;
	}

	GetGlobalProxyStrategies().Remove(ProxySchemaTag);
}

TSharedPtr<FBasicProxy> FProxyStrategy::CreateProxyBySchemaTag(const FGameplayTag& ProxySchemaTag)
{
	if (!ProxySchemaTag.IsValid())
	{
		return MakeShared<FBasicProxy>();
	}

	if (const TSharedPtr<FProxyStrategy>* FoundStrategy = GetGlobalProxyStrategies().Find(ProxySchemaTag))
	{
		if (FoundStrategy->IsValid())
		{
			TSharedPtr<FBasicProxy> Proxy = (*FoundStrategy)->GetProxy();
			if (Proxy.IsValid())
			{
				Proxy->ProxySchemaTag = ProxySchemaTag;
				return Proxy;
			}
		}
	}

	TSharedPtr<FBasicProxy> FallbackProxy = MakeShared<FBasicProxy>();
	FallbackProxy->ProxySchemaTag = ProxySchemaTag;
	return FallbackProxy;
}
