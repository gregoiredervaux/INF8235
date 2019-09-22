// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include <cmath>
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/PhysicalMaterials/PhysicalMaterial.h"
#include "SDTUtils.h"
#include <vector>
#include <string>
#include <typeinfo>
#include "SDTCollectible.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "PhysicsHelpers.h"
#include <algorithm>

const float DISTANCE_CHECK = 250.0f; //Max distance at which the agent will detect obstacles
const float DISTANCE_COLLECTIBLE_CHECK = 250.0f; //Max distance at which the agent will detect collectibles
const float MAX_ANGLE_CHECK = 130; //Only paths with [MAX_ANGLE_CHECK or less] rotation necessary will be considered.
const float MAX_ROTATION_PER_TICK = 10; //if [newDirection.Yaw > MAX_ROTATION_PER_TICK], the pawn's current rotation will increase of MAX_ROTATION_PER_TICK every tick until it reaches newDirection.Yaw

const FVector INITIAL_SPEED(0, 0, 0);
const float MAX_SPEED = 500.0f;
const float ACCELERATION = 500.0f;

ASDTAIController::ASDTAIController() : speed(INITIAL_SPEED), vitesseMax(MAX_SPEED), acc(ACCELERATION),currentRotation(0){
	
}
void ASDTAIController::Tick(float deltaTime)
{
	UWorld * World = GetWorld();
	
	APawn* pawn = GetPawn();
	if (currentRotation == 0 && !HandleCollect(pawn->GetActorLocation())) {
		HandleCollision(pawn->GetActorLocation());		
	}	
	FVector accVector = pawn->GetActorForwardVector() * acc; 
	FVector newSpeed = GetSpeedVector(speed, accVector, deltaTime); //Get new speed vector based on acc

	pawn->SetActorLocation(pawn->GetActorLocation() + newSpeed * deltaTime);
	
	if (FVector::Dist(newSpeed.GetSafeNormal(), pawn->GetActorForwardVector()) > 0.1f) {
		FRotator curr = pawn->GetActorForwardVector().Rotation();
		FRotator dir = newSpeed.GetSafeNormal2D().Rotation();
		float angle = currentRotation ? currentRotation : dir.Yaw - curr.Yaw;
		angle = angle > 180 ? -(360 - angle) : angle;

		if (std::abs(angle) > MAX_ROTATION_PER_TICK) {
			currentRotation = angle > 0 ? angle - MAX_ROTATION_PER_TICK : angle + MAX_ROTATION_PER_TICK;
			pawn->SetActorRotation(FQuat(pawn->GetActorRotation() + FRotator(0.0f, angle > 0 ? MAX_ROTATION_PER_TICK :-MAX_ROTATION_PER_TICK, 0.0f)));
			
		}
		else {
			pawn->SetActorRotation(FQuat(pawn->GetActorRotation() + FRotator(0.0f, angle, 0.0f)));
			currentRotation = 0;
		}
	}
	else {
		currentRotation = 0;
	}
	speed = pawn->GetActorForwardVector().GetSafeNormal2D() * newSpeed.Size2D();

}

FVector ASDTAIController::GetSpeedVector(FVector speedVector, FVector accVector, float deltaTime) {
	if (speedVector.Size2D() > vitesseMax) {
		return speedVector.GetSafeNormal2D() * vitesseMax;
	}
	return speedVector + accVector * deltaTime;
}

void ASDTAIController::DrawCharacterAxes(UWorld * world, APawn * pawn)
{
	FVector const playerLocation = pawn->GetActorLocation();
	DrawDebugDirectionalArrow(world, playerLocation, playerLocation + pawn->GetActorRightVector() * 100.0f, 100, FColor::Blue);
	DrawDebugDirectionalArrow(world, playerLocation, playerLocation + pawn->GetActorForwardVector() * 100.0f, 100, FColor::Blue);
	DrawDebugDirectionalArrow(world, playerLocation, playerLocation + pawn->GetActorUpVector() * 100.0f, 100, FColor::Blue);
}
bool ASDTAIController::HandleCollect(FVector currentLocation) {
	
	TSubclassOf<ASDTCollectible> classes;
	TArray<AActor*> outActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), outActors);
	
	for (int i = 0; i < outActors.Num(); i++) {
		FVector pos = outActors[i]->GetActorLocation();
		int norme = (currentLocation-pos).Size2D();
		if (norme < DISTANCE_COLLECTIBLE_CHECK && !((ASDTCollectible*)outActors[i])->IsOnCooldown()) {
			bool isOk = !SDTUtils::Raycast(GetWorld(), currentLocation,pos);
			if (isOk) {
				speed = (pos - currentLocation).GetSafeNormal2D() * speed.Size();
				return true;
			}
		}
	}
	return false;
}
void ASDTAIController::HandleCollision(FVector currentLocation)
{	
	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	ASoftDesignTrainingMainCharacter* main = Cast<ASoftDesignTrainingMainCharacter>(playerCharacter);

	TArray<FHitResult> hits;
	FVector start = currentLocation;
	FVector end = start + speed.GetSafeNormal()* DISTANCE_CHECK;
	FVector bestDir = main->IsPoweredUp() ? start- main->GetActorLocation() : main->GetActorLocation() - start; //Direction we would like the pawn to use
	FCollisionShape shape = FCollisionShape::MakeCapsule(42, 96);

	int norm = bestDir.Size2D();
	norm = main->IsPoweredUp() ? DISTANCE_CHECK : std::min(norm,(int)DISTANCE_CHECK);
	if(!GetWorld()->SweepMultiByChannel(hits, start, start + bestDir.GetSafeNormal2D() * norm, speed.GetSafeNormal2D().ToOrientationQuat(),ECC_Pawn, shape)){
		/*int diffAngle = std::acos(FVector::DotProduct(bestDir.GetSafeNormal2D(), speed.GetSafeNormal2D())) * 180 / PI;
		int scalar = FVector::DotProduct(bestDir.GetSafeNormal2D(), speed.GetSafeNormal2D());
		if (std::abs(diffAngle) <= 30) {
			speed = bestDir.GetSafeNormal2D() * speed.Size2D();
		}
		else {
			FRotator dir = speed.Rotation();
			if (scalar > 0) {
				dir.Yaw -= std::min(30,diffAngle);
			}
			else {
				dir.Yaw += std::min(30,diffAngle);
			}
			speed = dir.Vector().GetSafeNormal2D() * speed.Size2D();
		}
		return;*/
		speed = bestDir.GetSafeNormal2D() * speed.Size2D();
		return;
	}
	int angle = 0; //Starting angle used to find which side is cleared
	FRotator leftVector = speed.GetSafeNormal2D().Rotation();
	FRotator rightVector = speed.GetSafeNormal2D().Rotation();
	leftVector.Yaw -= angle;
	rightVector.Yaw += angle;

	bool wallAhead = GetWorld()->SweepMultiByChannel(hits, start, end, speed.GetSafeNormal2D().ToOrientationQuat(), ECC_Pawn, shape);
	DrawDebugCapsule(GetWorld(), end, shape.GetCapsuleHalfHeight(), shape.GetCapsuleRadius(), FQuat::Identity, FColor::Green);
		
	bool leftClear = false;
	bool rightClear = false;
	while (wallAhead && angle < MAX_ANGLE_CHECK) { //As long as the current direction leads to a wall
		if (!leftClear) {
			leftVector.Yaw -= 1;
		}
		if (!rightClear) {
			rightVector.Yaw += 1;
		}
		leftClear = !GetWorld()->SweepMultiByChannel(hits, start, start + leftVector.Vector().GetSafeNormal2D() * DISTANCE_CHECK, leftVector.Vector().GetSafeNormal2D().ToOrientationQuat(), ECC_Pawn, shape);			
		rightClear = !GetWorld()->SweepMultiByChannel(hits, start, start + rightVector.Vector().GetSafeNormal2D() * DISTANCE_CHECK, rightVector.Vector().GetSafeNormal2D().ToOrientationQuat(), ECC_Pawn, shape);
		wallAhead = !leftClear || !rightClear;
		angle++;
		
	}
	//Find direction closest to bestDir
	float angleLeft = 9999;
	float angleRight = 9999;
	if (leftClear) {
		angleLeft = std::acos(FVector::DotProduct(leftVector.Vector().GetSafeNormal2D(), bestDir.GetSafeNormal2D()));
	}
	if (rightClear) {
		angleRight = std::acos(FVector::DotProduct(rightVector.Vector().GetSafeNormal2D(), bestDir.GetSafeNormal2D()));
	}
	FRotator dir = speed.GetSafeNormal2D().Rotation();
	if (leftClear && (rightClear ? angleLeft< angleRight: true)) {
		dir.Yaw -= 5;//std::min(angle,5);
		speed = leftVector.Vector().GetSafeNormal2D() * speed.Size2D();
	}
	else if (rightClear && (leftClear ? angleRight < angleLeft:true)) {
		dir.Yaw += 5; // std::min(angle, 5);
		speed = rightVector.Vector().GetSafeNormal2D() * speed.Size2D();
	}	
}




