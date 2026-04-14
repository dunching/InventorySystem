#include "InventoryActor.h"

#include "InventoryComponent.h"

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

void AInventoryActor::BeginPlay()
{
	Super::BeginPlay();
}

void AInventoryActor::EndPlay(
	const EEndPlayReason::Type EndPlayReason
	)
{
	Super::EndPlay(EndPlayReason);
}

TObjectPtr<UInventoryComponent> AInventoryActor::GetInventoryComponent() const
{
	return InventoryComponentPtr;
}
