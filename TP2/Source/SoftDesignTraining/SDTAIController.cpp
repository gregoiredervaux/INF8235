// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"
#include "SoftDesignTrainingMainCharacter.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
	
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{
	
    //Move to target depending on current behavior
	TArray<AActor*> outActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTFleeLocation::StaticClass(), outActors);
	MoveToActor(outActors[3]);
	if (Behavior == 1 || Behavior == 2) {
		MoveToActor(Target);
	}
	else if (Behavior == 3) {
		TArray<AActor*> outActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTFleeLocation::StaticClass(), outActors);
		MoveToActor(outActors[0]);
	}
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);
	
    m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
    //Show current navigation path DrawDebugLine and DrawDebugSphere
	DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), Target->GetActorLocation(), FColor::Red);
}

void ASDTAIController::ChooseBehavior(float deltaTime)
{
    UpdatePlayerInteraction(deltaTime);
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    //finish jump before updating AI state
	//AtJumpSegment = false;
	//}
    if (AtJumpSegment)
        return;

    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
		//DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorLocation() + FVector(0, 0, 1000), FColor::Red);
        return;

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_COLLECTIBLE));
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    //Set behavior based on hit
	if (&detectionHit) {
		if (detectionHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER) {
			ASoftDesignTrainingMainCharacter* main = GetPlayer();
			if (main) {
				if (main->IsPoweredUp())
					Behavior = 3;
				else {
					Behavior = 2;
					Target = detectionHit.GetActor();
				}
			}
		} 
		else {
			Behavior = 1;
			Target = detectionHit.GetActor();
		}
	}
	//DrawDebugLine(GetWorld(), selfPawn->GetActorLocation(), Target->GetActorLocation(), FColor::Blue);
    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
{
    for (const FHitResult& hit : hits)
    {
        if (UPrimitiveComponent* component = hit.GetComponent())
        {
            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
            {
                //we can't get more important than the player
                outDetectionHit = hit;
                return;
            }
            else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                outDetectionHit = hit;
            }
        }
    }
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
}

//Returns the main character
ASoftDesignTrainingMainCharacter* ASDTAIController::GetPlayer() {

	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	return Cast<ASoftDesignTrainingMainCharacter>(playerCharacter);
}