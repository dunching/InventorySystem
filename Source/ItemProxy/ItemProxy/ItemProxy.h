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

	// FastArray 增删改时使用的稳定运行时 ID。
	FGuid ProxyId = FGuid::NewGuid();

	// 道具类型标签，用于查询与堆叠。
	FGameplayTag ItemTag;

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
	virtual FGameplayTag GetTag()const;
	
	virtual TSharedPtr<FBasicProxy> GetProxy()const;
};
