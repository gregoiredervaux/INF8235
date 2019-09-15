// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
	FVector GetSpeedVector(FVector speedVector, FVector accVector, float deltaTime);
	void DrawCharacterAxes(UWorld * world, APawn * pawn);
	void HandleCollision(FVector currentLocation);
	
protected:
	float acc;
	float vitesseMax;
	FVector speed;
};
