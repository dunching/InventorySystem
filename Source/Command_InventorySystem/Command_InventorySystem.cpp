#include "Command_InventorySystem.h"

#include "InventoryActor.h"
#include "InventoryComponent.h"
#include "Tools.h"

namespace GameplayCommand
{
	void AddProxy(
		const TArray<FString>& Args
		)
	{
		if (!Args.IsValidIndex(0))
		{
			return;
		}

		auto GetInventoryInterface = Cast<IGetInventoryInterface>(
																  GEngine->GetFirstLocalPlayerController(
																	   GetWorldImp()
																	  )
																 );
		
		if (GetInventoryInterface)
		{
			GetInventoryInterface->GetInventory()->GetInventoryComponent()->AddProxy(
				 FGameplayTag::RequestGameplayTag(*Args[0]),
				 1
				);
		}
	}
}
