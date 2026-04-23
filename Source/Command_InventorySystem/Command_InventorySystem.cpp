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
		// Args[0] 约定为道具 GameplayTag 字符串。
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
			// 调用库存组件添加 1 个条目（堆叠由组件内部处理）。
			GetInventoryInterface->GetInventory()->GetInventoryComponent()->AddProxy(
				 FGameplayTag::RequestGameplayTag(*Args[0]),
				 1
				);
		}
	}
}
