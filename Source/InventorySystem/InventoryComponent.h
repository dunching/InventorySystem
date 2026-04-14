#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "InventoryComponent.generated.h"

struct FBasicProxy;

class UItemDefine;

/**
 *
 */
UCLASS()
class INVENTORYSYSTEM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	TSharedPtr<FBasicProxy> AddProxy(
		const FGameplayTag& ProxyType,
		uint8 Num
		);

#if WITH_EDITORONLY_DATA || UE_CLIENT
	UFUNCTION(Server, Reliable)
	void AddProxy_Server(
		const FGameplayTag& ProxyType,
		uint8 Num
		);
#endif

	UPROPERTY(Transient)
	TMap<FGameplayTag, UItemDefine*> AllItemDefineMap;
};
