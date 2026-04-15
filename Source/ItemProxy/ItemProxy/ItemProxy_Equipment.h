// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "ItemProxy.h"

#include "ItemProxy_Equipment.generated.h"

/**
 */
USTRUCT()
struct ITEMPROXY_API FItemProxy_Equipment : public FBasicProxy
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
struct TStructOpsTypeTraits<FItemProxy_Equipment> :
	public TStructOpsTypeTraitsBase2<FItemProxy_Equipment>
{
	enum
	{
		WithNetSerializer = true,
	};
};

struct ITEMPROXY_API FProxy_EquipmentStrategy : public FProxyStrategy
{
	virtual FGameplayTag GetTag()const override;
	
	virtual TSharedPtr<FBasicProxy> GetProxy()const;
};


