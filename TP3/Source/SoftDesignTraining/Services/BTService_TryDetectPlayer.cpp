// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BTService_TryDetectPlayer.h"

UBTService_TryDetectPlayer::UBTService_TryDetectPlayer()
{
	bCreateNodeInstance = true;
}

void UBTService_TryDetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
	{

		aiController->DetectPlayer();
		if (aiController->IsTargetPlayerSeen())
		{
			//write to bb that the player is seen
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiController->GetTargetSeenKeyID(), true);

			//write to bb the position of the target
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(aiController->GetTargetPosBBKeyID(), aiController->GetTargetPlayerPos());
		}
		else
		{
			//write to bb that the player is not seen
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiController->GetTargetSeenKeyID(), false);
		}

	}
}

