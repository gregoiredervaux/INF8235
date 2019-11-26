// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "UpdateController.h"
#include "SoftDesignTrainingGameMode.generated.h"

UCLASS(minimalapi)
class ASoftDesignTrainingGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASoftDesignTrainingGameMode();

    virtual void StartPlay() override;

	UpdateController* updateController;
};



