// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTPathFollowingComponent.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
{
	
}

void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];

    if (SDTUtils::HasJumpFlag(segmentStart))
    {
        //update jump
		Super::FollowPathSegment(DeltaTime);
    }
    else
    {
		Super::FollowPathSegment(DeltaTime);
        //update navigation along path

    }
}

void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{
    Super::SetMoveSegment(segmentStartIndex);

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();

    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];
	ASDTAIController* Controller = dynamic_cast<ASDTAIController*>(GetOwner());

    if (SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
    {
		UCharacterMovementComponent* castedMov = Cast<UCharacterMovementComponent>(MovementComp);		
		//castedMov->SetMovementMode(MOVE_Flying);
		DrawDebugLine(GetWorld(), MovementComp->GetActorLocation(), MovementComp->GetActorLocation() + FVector(0, 0, 1000), FColor::Red);
		Controller->AtJumpSegment = true;
        //Handle starting jump
		//ASoftDesignTrainingCharacter* castedPlayerCharacter = Cast<ASoftDesignTrainingCharacter>(GetOwner());
		
    }
    else
    {
        //Handle normal segments
    }
}

