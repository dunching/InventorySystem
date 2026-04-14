// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ItemProxy.generated.h"

/**
 */
USTRUCT()
struct INVENTORYSYSTEM_API FBasicProxy
{
	GENERATED_USTRUCT_BODY()

public:
	virtual bool NetSerialize(
		FArchive& Ar,
		class UPackageMap* Map,
		bool& bOutSuccess
		);
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
