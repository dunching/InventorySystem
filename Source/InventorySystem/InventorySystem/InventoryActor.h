
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
	AInventoryActor(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
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
