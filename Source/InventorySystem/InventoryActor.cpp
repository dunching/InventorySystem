#include "InventoryActor.h"

#include "InventoryComponent.h"

FName AInventoryActor::ComponentName = TEXT("InventoryActor");

AInventoryActor::AInventoryActor(
	const FObjectInitializer& ObjectInitializer
	):
	Super(ObjectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicatingMovement(false);
	SetNetUpdateFrequency(1.f);
	
	InventoryComponentPtr = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

TObjectPtr<UInventoryComponent> AInventoryActor::GetInventoryComponent() const
{
	return InventoryComponentPtr;
}
