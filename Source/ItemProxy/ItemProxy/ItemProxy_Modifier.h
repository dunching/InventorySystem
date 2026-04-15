// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "ItemProxy.h"

#include "ItemProxy_Modifier.generated.h"

/**
 */
USTRUCT()
struct ITEMPROXY_API FItemProxy_Modifier : public FBasicProxy
{
	GENERATED_USTRUCT_BODY()

public:
	virtual bool NetSerialize(
		FArchive& Ar,
		class UPackageMap* Map,
		bool& bOutSuccess
		) override;
};

template <>
struct TStructOpsTypeTraits<FItemProxy_Modifier> :
	public TStructOpsTypeTraitsBase2<FItemProxy_Modifier>
{
	enum
	{
		WithNetSerializer = true,
	};
};

struct ITEMPROXY_API FProxy_ModifierStrategy : public FProxyStrategy
{
	virtual FGameplayTag GetTag()const override;
	
	virtual TSharedPtr<FBasicProxy> GetProxy()const;
};
