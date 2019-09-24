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
#include <list>

const float DISTANCE_CHECK = 250.0f; //Max distance at which the agent will detect obstacles
const float DISTANCE_COLLECTIBLE_CHECK = 800.0f; //Max distance at which the agent will detect collectibles
const float MAX_ANGLE_CHECK = 130; //Only paths with [MAX_ANGLE_CHECK or less] rotation necessary will be considered.
const float MAX_ROTATION_PER_TICK = 15; //if [newDirection.Yaw > MAX_ROTATION_PER_TICK], the pawn's current rotation will increase of MAX_ROTATION_PER_TICK every tick until it reaches newDirection.Yaw
const float MIN_PLAYER_DISTANCE_FOR_COLLECTIBLE = 500.0f; //if [distanceToPlayer <= MIN_PLAYER_DISTANCE_FOR_COLLECTIBLE], the pawn will ignore collectibles and only react to the player movement

const FVector INITIAL_SPEED(0, 0, 0);
const float MAX_SPEED = 650.0f;
const float ACCELERATION = 350.0f;
const int MIN_DISTANCE_TRAPS = 380.0f;


ASDTAIController::ASDTAIController() : speed(INITIAL_SPEED), vitesseMax(MAX_SPEED), acc(ACCELERATION),currentRotation(0),targetCollectibleLocation(0,0,0),deathFloorLocations() {
	
}
void ASDTAIController::Tick(float deltaTime)
{
	UWorld * World = GetWorld();	
	APawn* pawn = GetPawn();

	// GET DEATH FLOORS
	if (deathFloorLocations.size() == 0) {
		FindDeathFloors();
	}
	//

	if (currentRotation == 0 && !HandleCollect(pawn->GetActorLocation())) {
		HandleCollision(pawn->GetActorLocation()); //Only handle collision if no rotation is occuring and no collectible is in sight
		
	}	
	//FVector accVector = pawn->GetActorForwardVector() * acc;
//	FVector newSpeed = GetSpeedVector(speed, accVector, deltaTime); //Get new speed vector based on acc

	pawn->SetActorLocation(pawn->GetActorLocation() + speed * deltaTime);
	
	if (FVector::Dist(speed.GetSafeNormal(), pawn->GetActorForwardVector()) > 0.1f) { //If the speed vector is not pointing in the same direction as the orientation
		FRotator curr = pawn->GetActorForwardVector().Rotation();
		FRotator dir = speed.GetSafeNormal2D().Rotation();
		float angle = currentRotation ? currentRotation : dir.Yaw - curr.Yaw; //Only update if there is not a current rotation
		angle = angle > 180 ? -(360-angle) : angle;

		int nbTicksForRotation = std::abs(angle) / MAX_ROTATION_PER_TICK;
		//acc -= nbTicksForRotation * ACCELERATION; //Slow down depending on the scale of the rotation
		if (std::abs(angle) > MAX_ROTATION_PER_TICK) { //Set a current rotation that will occur on more than 1 tick
			currentRotation = angle > 0 ? angle - MAX_ROTATION_PER_TICK : angle + MAX_ROTATION_PER_TICK;
			pawn->SetActorRotation(FQuat(pawn->GetActorRotation() + FRotator(0.0f, angle > 0 ? MAX_ROTATION_PER_TICK :-MAX_ROTATION_PER_TICK, 0.0f)));
			
		}
		else { //Do the full rotation at once since the angle is small
			pawn->SetActorRotation(FQuat(pawn->GetActorRotation() + FRotator(0.0f, angle, 0.0f)));
			currentRotation = 0;
			acc = ACCELERATION;
		}
	}
	else {
		currentRotation = 0;
		acc = ACCELERATION;
		
	}
	FVector accVector = pawn->GetActorForwardVector() * acc;
	FVector newSpeed = GetSpeedVector(speed, accVector, deltaTime); //Get new speed vector based on acc
	speed = pawn->GetActorForwardVector().GetSafeNormal2D() * newSpeed.Size2D();	//Update the speed vector

}
void ASDTAIController::FindDeathFloors() { //Fill deathFloorLocations with the position of all death floors
	
	TSubclassOf<UStaticMeshComponent> classes;
	TArray<AActor*> outActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), outActors);

	for (int i = 0; i < outActors.Num(); i++) {
		if (outActors[i]->GetName().Contains("DeathFloor")) {
			deathFloorLocations.push_front(outActors[i]->GetActorLocation());
		}
	}
}

FVector ASDTAIController::GetSpeedVector(FVector speedVector, FVector accVector, float deltaTime) {
	if ((speedVector + accVector*deltaTime).Size2D() > vitesseMax) {
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
bool ASDTAIController::isOnDeathFloor(FVector start, FVector end) { //Returns true if the path from [start] to [end] is close or on a death floor
	FVector path = end - start;
	for (std::list<FVector>::iterator it = deathFloorLocations.begin(); it != deathFloorLocations.end(); it++) {
		FVector dir = *it - start;
		dir.Z = 0;
		float dist = (*it - end).Size2D(); //distance between the end of the vector and the death floor
		float angleTo = std::acos(FVector::DotProduct(dir.GetSafeNormal2D(), path.GetSafeNormal2D()));
		if (angleTo < 30 && dist < MIN_DISTANCE_TRAPS) {
			return true;
		}
	}
	return false;
}
bool ASDTAIController::HandleCollect(FVector currentLocation) { //Returns true if a collectible is in sight (no obstacle) and close enough
	
	//ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	//ASoftDesignTrainingMainCharacter* main = Cast<ASoftDesignTrainingMainCharacter>(playerCharacter);
	
	float distanceToPlayer = GetMain() ? (currentLocation - GetMain()->GetActorLocation()).Size2D() : 99999;

	TSubclassOf<ASDTCollectible> classes;
	TArray<AActor*> outActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), outActors); //fetch all ASDTCollectibles
	int minDistance = 99999;
	for (int i = 0; i < outActors.Num(); i++) {
		FVector pos = outActors[i]->GetActorLocation();
		int norm = (currentLocation-pos).Size2D();
		
		if (norm < DISTANCE_COLLECTIBLE_CHECK && !((ASDTCollectible*)outActors[i])->IsOnCooldown()) {
			bool isInSight = !SDTUtils::Raycast(GetWorld(), currentLocation,pos) && !isOnDeathFloor(currentLocation,pos);
			if (isInSight) {
				if (norm < distanceToPlayer && distanceToPlayer > MIN_PLAYER_DISTANCE_FOR_COLLECTIBLE) { //If the player is too close, ignore the collectible
					speed = (pos - currentLocation).GetSafeNormal2D() * speed.Size();
					targetCollectibleLocation = pos;
					return true;
				}
			}
		}
	}
	return false;
}
ASoftDesignTrainingMainCharacter* ASDTAIController::GetMain() { //Returns the main character
	
	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	return Cast<ASoftDesignTrainingMainCharacter>(playerCharacter);
}
void ASDTAIController::HandleCollision(FVector currentLocation)	//Handles walls and chasing the main character
{	
	ASoftDesignTrainingMainCharacter* main = GetMain();

	TArray<FHitResult> hits;
	FVector start = currentLocation;
	FVector end = start + speed.GetSafeNormal()* DISTANCE_CHECK;
	FVector bestDir = GetPawn()->GetActorForwardVector();
	if (main) { //If the main character exists, we want to choose a direction which helps us follow or run away from him
		bestDir = main->IsPoweredUp() ? start - main->GetActorLocation() : main->GetActorLocation() - start; 
	}
	FCollisionShape shape = FCollisionShape::MakeCapsule(42, 96); //Shape representing the pawn

	int norm = bestDir.Size2D();
	if (main) {
		norm = main->IsPoweredUp() ? DISTANCE_CHECK : std::min(norm, (int)DISTANCE_CHECK); //Only raycast at the distance from main if it's closer than DISTANCE_CHECK
	}
	if(main && !GetWorld()->SweepMultiByChannel(hits, start, start + bestDir.GetSafeNormal2D() * norm, speed.GetSafeNormal2D().ToOrientationQuat(),ECC_Pawn, shape)
		&& !isOnDeathFloor(start, start + bestDir.GetSafeNormal2D() * norm)){ //Path to bestDir is clear
		speed = bestDir.GetSafeNormal2D() * speed.Size2D();
		return;
	}
	FRotator leftVector = speed.GetSafeNormal2D().Rotation();
	FRotator rightVector = speed.GetSafeNormal2D().Rotation();
	bool wallAhead = GetWorld()->SweepMultiByChannel(hits, start, end, speed.GetSafeNormal2D().ToOrientationQuat(), ECC_Pawn, shape)
		|| isOnDeathFloor(start,end);
	bool leftClear = false;
	bool rightClear = false;
	int angle = 0;
	while (wallAhead && angle < 180) { //As long as the current direction leads to a wall
		if (!leftClear) { //Raycast to the left and see if it's clear
			leftVector.Yaw -= 1;
			leftClear = !GetWorld()->SweepMultiByChannel(hits, start, start + leftVector.Vector().GetSafeNormal2D() * DISTANCE_CHECK, leftVector.Vector().GetSafeNormal2D().ToOrientationQuat(), ECC_Pawn, shape)
				&& !isOnDeathFloor(start, start + leftVector.Vector().GetSafeNormal2D()*DISTANCE_CHECK);
		}
		if (!rightClear) { //Raycast to the right and see if it's clear
			rightVector.Yaw += 1;
			rightClear = !GetWorld()->SweepMultiByChannel(hits, start, start + rightVector.Vector().GetSafeNormal2D() * DISTANCE_CHECK, rightVector.Vector().GetSafeNormal2D().ToOrientationQuat(), ECC_Pawn, shape)
				&& !isOnDeathFloor(start, start + rightVector.Vector().GetSafeNormal2D()*DISTANCE_CHECK);
		}		
		bool bothCleared = leftClear && rightClear;
		bool atLeastOneClear = leftClear || rightClear;
		wallAhead = !bothCleared && !(atLeastOneClear && angle >= MAX_ANGLE_CHECK); //We try to clear both sides, but leave the loop if we reached the MAX_ANGLE + 1 side is clear
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

	if (leftClear && (rightClear ? angleLeft< angleRight: true)) { 
		
		speed = leftVector.Vector().GetSafeNormal2D() * speed.Size2D();
	}
	else if (rightClear && (leftClear ? angleRight < angleLeft:true)) {
		
		speed = rightVector.Vector().GetSafeNormal2D() * speed.Size2D();
	}
	else if (wallAhead) { //Means we did not find a clear path
		speed = -speed;
	}
}




