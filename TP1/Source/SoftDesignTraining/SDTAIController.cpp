// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include <cmath>
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/PhysicalMaterials/PhysicalMaterial.h"
#include "SDTUtils.h"
#include <vector>
#include <string>

ASDTAIController::ASDTAIController() : speed(5,0, 0), vitesseMax(600.0f), acc(500.0f){
	
}

void ASDTAIController::Tick(float deltaTime)
{
	UWorld * World = GetWorld();
	APawn* pawn = GetPawn();
	DrawCharacterAxes(World, pawn);
	HandleCollision(pawn->GetActorLocation());
	FVector accVector = pawn->GetActorForwardVector() * acc; 
	FVector newSpeed = GetSpeedVector(speed, accVector, deltaTime); //Get new speed vector based on acc
	

	DrawDebugDirectionalArrow(World, pawn->GetActorLocation(), pawn->GetActorLocation() + newSpeed * 10000.0f, 100, FColor::Red);
	pawn->SetActorLocation(pawn->GetActorLocation() + newSpeed * deltaTime);
	
	if (FVector::Dist(newSpeed.GetSafeNormal(), pawn->GetActorForwardVector()) > 0.10f) {
		float angle = std::acos(FVector::DotProduct(newSpeed.GetSafeNormal(), pawn->GetActorForwardVector().GetSafeNormal()));
		pawn->AddActorWorldRotation(FRotator(0.0f, angle * 180 / PI, 0.0f)); //Rotate pawn to it's forward vector
	}
	speed = newSpeed;

}

FVector ASDTAIController::GetSpeedVector(FVector speedVector, FVector accVector, float deltaTime) {
	if (speedVector.Size() > vitesseMax) {
		return speedVector.GetSafeNormal() * vitesseMax;
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
void ASDTAIController::HandleCollision(FVector currentLocation)
{
	const float DISTANCE_CHECK = 250.0f;
	int angle = 10; //	Angle in degrees that will be used to raycast on the left/right
	FRotator leftVector = speed.GetSafeNormal().Rotation();
	FRotator rightVector = speed.GetSafeNormal().Rotation();
	leftVector.Yaw -= angle;
	rightVector.Yaw += angle;
	bool wallAhead = SDTUtils::Raycast(GetWorld(), currentLocation, currentLocation + speed.GetSafeNormal() * DISTANCE_CHECK)
		|| SDTUtils::Raycast(GetWorld(), currentLocation, currentLocation + leftVector.Vector().GetSafeNormal() *DISTANCE_CHECK)
		|| SDTUtils::Raycast(GetWorld(), currentLocation, currentLocation + rightVector.Vector().GetSafeNormal() * DISTANCE_CHECK);
					
	
	bool leftClear = false;
	bool rightClear = false;
	while (wallAhead) { //As long as the current direction leads to a wall
		leftVector.Yaw -= 1;
		rightVector.Yaw += 1;
		leftClear = !SDTUtils::Raycast(GetWorld(), currentLocation, currentLocation + leftVector.Vector().GetSafeNormal() * DISTANCE_CHECK);
		rightClear = !SDTUtils::Raycast(GetWorld(), currentLocation, currentLocation+ rightVector.Vector().GetSafeNormal() * DISTANCE_CHECK);
		wallAhead = !leftClear && !rightClear;
	}
	if (leftClear || rightClear) { //Means we had to do a rotation
		speed = (leftClear ? leftVector.Vector().GetSafeNormal() : rightVector.Vector().GetSafeNormal()) * (sqrt(speed.X*speed.X + speed.Y*speed.Y + speed.Z*speed.Z));		
	}

	
}




