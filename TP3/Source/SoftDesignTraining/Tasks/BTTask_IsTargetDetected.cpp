// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SoftDesignTrainingCharacter.h"
#include "SDTAIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "SDTAIGroupController.h"
#include "BTTask_IsTargetDetected.h"


EBTNodeResult::Type UBTTask_IsTargetDetected::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
	{
		if (OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Bool>(aiController->GetTargetSeenKeyID()))
		{
			SDTAIGroupController::GetGroup()->addAgent(aiController);
			return EBTNodeResult::Succeeded;
		}
		else
		{
			SDTAIGroupController::GetGroup()->removeAgent(aiController);
		}
	}

	return EBTNodeResult::Failed;
}