
#pragma once

#include "CoreMinimal.h"

#include "InventoryActor.generated.h"

class UInventoryComponent;

/**
 * 复制用库存承载 Actor。
 * 将库存组件从 Pawn/Controller 生命周期中解耦。
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
	// 供玩法逻辑与控制台命令访问库存入口。
	virtual AInventoryActor* GetInventory() const= 0;

protected:

private:
};
