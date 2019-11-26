#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_Collect.generated.h"

/**
 *
 */
UCLASS()
class SOFTDESIGNTRAINING_API UBTTask_Collect : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

		virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
