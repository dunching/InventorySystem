// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemProxy.generated.h"

class UItemDefine;

/**
 */
USTRUCT()
struct ITEMPROXY_API FBasicProxy
{
	GENERATED_USTRUCT_BODY()

public:
	virtual ~FBasicProxy();
	
	virtual bool NetSerialize(
		FArchive& Ar,
		class UPackageMap* Map,
		bool& bOutSuccess
		);

	virtual bool IsInUse() const;

	// FastArray 增删改时使用的稳定运行时 ID。
	FGuid ProxyId = FGuid::NewGuid();

	// 道具类型标签，用于查询与堆叠。
	FGameplayTag ItemTag;

	// 库存实例 schema，用于网络反序列化时恢复具体 Proxy 类型。
	FGameplayTag ProxySchemaTag;

	// 当前条目的数量。
	int32 Count = 1;
	
	TObjectPtr<UItemDefine> ItemDefine;
	
	TMap<FGameplayTag, float> ValuesMap;
};

template <>
struct TStructOpsTypeTraits<FBasicProxy> :
	public TStructOpsTypeTraitsBase2<FBasicProxy>
{
	enum
	{
		WithNetSerializer = true,
	};
};

struct ITEMPROXY_API FProxyStrategy
{
	virtual FGameplayTag GetProxySchemaTag() const;
	
	virtual TSharedPtr<FBasicProxy> GetProxy()const;

	static void RegisterGlobalStrategy(const TSharedPtr<FProxyStrategy>& Strategy);
	static void UnregisterGlobalStrategy(const FGameplayTag& ProxySchemaTag);
	static TSharedPtr<FBasicProxy> CreateProxyBySchemaTag(const FGameplayTag& ProxySchemaTag);
};
