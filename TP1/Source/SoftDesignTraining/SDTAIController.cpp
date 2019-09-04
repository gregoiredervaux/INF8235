// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include <cmath>
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/PhysicalMaterials/PhysicalMaterial.h"

ASDTAIController::ASDTAIController() : speed(0, 0, 0), vitesseMax(200.0f), acc(10.0f){
	
}

void ASDTAIController::Tick(float deltaTime)
{
	UWorld * World = GetWorld();
	APawn* pawn = GetPawn();
	DrawCharacterAxes(World, pawn);
	
	FVector accVector = pawn->GetActorForwardVector() * acc;
	FVector newSpeed = GetSpeedVector(speed, accVector, deltaTime);

	DrawDebugDirectionalArrow(World, pawn->GetActorLocation(), pawn->GetActorLocation() + newSpeed * 10000.0f, 100, FColor::Red);
	pawn->AddMovementInput(newSpeed);
	float angle = std::acos(FVector::DotProduct(newSpeed.GetSafeNormal(), speed.GetSafeNormal()));
	pawn->AddActorWorldRotation(FRotator(0.0f, angle, 0.0f));
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




