// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "AI/Navigation/NavigationSystem.h"
#include "SDTUtils.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BTTask_IsTargetPoweredUp.h"

EBTNodeResult::Type UBTTask_IsTargetPoweredUp::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (SDTUtils::IsPlayerPoweredUp(GetWorld()))
		return EBTNodeResult::Succeeded;
	return EBTNodeResult::Failed;
}
