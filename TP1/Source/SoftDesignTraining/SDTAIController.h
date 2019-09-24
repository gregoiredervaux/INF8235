// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <list>
#include "SoftDesignTrainingMainCharacter.h"
#include "CoreMinimal.h"
#include "AIController.h"
#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
    virtual void Tick(float deltaTime) override;
	ASDTAIController ();
	void FindDeathFloors();
	FVector GetSpeedVector(FVector speedVector, FVector accVector, float deltaTime);
	void DrawCharacterAxes(UWorld * world, APawn * pawn);
	bool isOnDeathFloor(FVector start, FVector end);
	void HandleCollision(FVector currentLocation);
	bool HandleCollect(FVector currentLocation);

	ASoftDesignTrainingMainCharacter * GetMain();
	
protected:
	float acc;
	float vitesseMax;
	FVector speed;
	FVector targetCollectibleLocation;
	float currentRotation;
	std::list<FVector> deathFloorLocations;
};
