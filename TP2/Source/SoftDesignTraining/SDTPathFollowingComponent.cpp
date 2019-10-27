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
	const FNavPathPoint& segmentEnd = points[MoveSegmentEndIndex];
	ASDTAIController* Controller = dynamic_cast<ASDTAIController*>(GetOwner());
	//DrawDebugSphere(GetWorld(), segmentEnd.Location, 50.0f, 50, FColor::Blue);
	//DrawDebugSphere(GetWorld(), segmentStart.Location, 50.0f, 50, FColor::Green);
	/*for (int i = MoveSegmentEndIndex + 1; i < points.Num(); i++) {
		DrawDebugSphere(GetWorld(), points[i].Location, 50.0f, 50, FColor::Yellow);
	}*/

    if (SDTUtils::HasJumpFlag(segmentStart) || Controller->AtJumpSegment)
    {
        //update jump
		Controller->AtJumpSegment = true;
		float totalDist = (segmentStart.Location - segmentEnd.Location).Size2D();
		FVector Pos = Controller->GetPawn()->GetActorLocation();
		DrawDebugSphere(GetWorld(), Pos, 500.0f, 50, FColor::Blue);
		FVector2D Direction = FVector2D(segmentEnd.Location - Pos).GetSafeNormal();
		Controller->GetPawn()->SetActorRotation(FMath::Lerp(Controller->GetPawn()->GetActorRotation(), FVector(Direction, 0.f).Rotation(), 0.1f));
		float dist2D = (Pos - segmentStart.Location).Size2D() + Controller->JumpSpeed * DeltaTime *10;
		float ZValue = Controller->JumpCurve->GetFloatValue(dist2D/totalDist);
		if (ZValue > 0.25f)
			Controller->InAir = true;
		if (ZValue < 0.25f && Controller->InAir)
			Controller->Landing = true;
		Controller->GetPawn()->SetActorLocation(Pos + FVector(Direction*dist2D, Controller->JumpApexHeight * ZValue), true);
		Controller->GetPawn()->AddMovementInput(FVector(Direction, 0.0f),100.0f);
		if ((Pos - segmentEnd.Location).Size2D() < 10.0f) {
			Controller->AtJumpSegment = false;
			Controller->InAir = false;
			Controller->Landing = false;
		}
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

    if ((SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink()) || dynamic_cast<ASDTAIController*>(GetOwner())->AtJumpSegment)
    {
        //Handle starting jump
		Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Flying);
    }
    else
    {
        //Handle normal segments
		Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Walking);
    }
}

