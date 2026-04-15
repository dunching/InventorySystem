#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "ItemProxy_Container.h"

#include "InventoryComponent.generated.h"

class UItemDefine;

struct FBasicProxy;
struct FProxyStrategy;

/**
 *
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	TWeakPtr<FBasicProxy> AddProxy(
		const FGameplayTag& ProxyType,
		uint8 Num
		);

	void AddGetProxyMetaStrategy(
		const TSharedPtr<FProxyStrategy>& ProxyMetaStrategyFunc
		);

#if WITH_EDITORONLY_DATA || UE_CLIENT
	UFUNCTION(Server, Reliable)
	void AddProxy_Server(
		const FGameplayTag& ProxyType,
		uint8 Num
		);
#endif

	TMap<FGameplayTag, TSharedPtr<FProxyStrategy>> GetProxyMetaStrategies;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UItemDefine>> AllItemDefineMap;

	TArray<TSharedPtr<FBasicProxy>> ProxysAry;

	FProxy_FASI_Container Proxy_FASI_Container;
};
