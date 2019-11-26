// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "DrawDebugHelpers.h"
#include "AI/Navigation/NavigationSystem.h"

#include "BTTask_Chase.h"




EBTNodeResult::Type UBTTask_Chase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
	{
		FVector actorLocation = aiController->GetPawn()->GetActorLocation();
		DrawDebugSphere(aiController->GetWorld(), actorLocation + FVector(0.f, 0.f, 100.f), 25.0f, 32, FColor::Purple);
		aiController->MoveAroundPlayer();
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}