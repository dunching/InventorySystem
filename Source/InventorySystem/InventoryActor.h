
#pragma once

#include "CoreMinimal.h"

#include "InventoryActor.generated.h"

class UInventoryComponent;

/**
 *
 */
UCLASS()
class INVENTORYSYSTEM_API AInventoryActor : public AInfo
{
	GENERATED_BODY()
public:
	static FName ComponentName;
	
	AInventoryActor(const FObjectInitializer& ObjectInitializer);

	TObjectPtr<UInventoryComponent> GetInventoryComponent()const;
	
protected:
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<UInventoryComponent> InventoryComponentPtr = nullptr;

};

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UGetInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYSYSTEM_API IGetInventoryInterface
{
	GENERATED_BODY()

public:
	virtual AInventoryActor* GetInventory() const= 0;

protected:

private:
};
