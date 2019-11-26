// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SoftDesignTrainingCharacter.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "SDTAIController.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "SDTAIGroupController.h"
#include "UpdateController.h"
#include "SoftDesignTrainingGameMode.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent"))), m_isTargetSeenBBKeyID(0)
	, m_targetPosBBKeyID(0)
{
	m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
	m_behaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	m_blackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void ASDTAIController::BeginPlay()
{
	Super::BeginPlay();

	StartBehaviorTree();
	lastUpdate = 0;
	UpdateController* updateController = ((ASoftDesignTrainingGameMode*)GetWorld()->GetAuthGameMode())->updateController;
	updateController->addAgent(); //count the new agent
}

void ASDTAIController::StartBehaviorTree()
{
	if (ASoftDesignTrainingCharacter* aiCharacter = Cast<ASoftDesignTrainingCharacter>(GetPawn()))
	{
		if (aiCharacter->GetBehaviorTree())
		{
			m_behaviorTreeComponent->StartTree(*aiCharacter->GetBehaviorTree());
		}
	}
}

void ASDTAIController::StopBehaviorTree()
{
	m_behaviorTreeComponent->StopTree();
}

void ASDTAIController::Possess(APawn* pawn)
{
	Super::Possess(pawn);

	if (ASoftDesignTrainingCharacter* aiCharacter = Cast<ASoftDesignTrainingCharacter>(pawn))
	{
		if (aiCharacter->GetBehaviorTree())
		{
			m_blackboardComponent->InitializeBlackboard(*aiCharacter->GetBehaviorTree()->BlackboardAsset);

			m_targetPosBBKeyID = m_blackboardComponent->GetKeyID("TargetPos");
			m_isTargetSeenBBKeyID = m_blackboardComponent->GetKeyID("TargetIsSeen");

			//Set this agent in the BT
			m_blackboardComponent->SetValue<UBlackboardKeyType_Object>(m_blackboardComponent->GetKeyID("SelfActor"), pawn);
		}
	}
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{
	switch (m_PlayerInteractionBehavior)
	{
	case PlayerInteractionBehavior_Collect:

		MoveToRandomCollectible();

		break;

	case PlayerInteractionBehavior_Chase:

		MoveToPlayer();

		break;

	case PlayerInteractionBehavior_Flee:

		MoveToBestFleeLocation();

		break;
	}
}

void ASDTAIController::MoveToRandomCollectible()
{
	UpdateController* updateController = ((ASoftDesignTrainingGameMode*)GetWorld()->GetAuthGameMode())->updateController;
	if (!updateController->canUpdate(lastUpdate)) {
		return;
	}

	lastUpdate = GFrameCounter;

	double initialTime = FPlatformTime::Seconds(); //used to measure cpu time

	float closestSqrCollectibleDistance = 18446744073709551610.f;
	ASDTCollectible* closestCollectible = nullptr;

	TArray<AActor*> foundCollectibles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), foundCollectibles);

	while (foundCollectibles.Num() != 0)
	{
		int index = FMath::RandRange(0, foundCollectibles.Num() - 1);

		ASDTCollectible* collectibleActor = Cast<ASDTCollectible>(foundCollectibles[index]);
		if (!collectibleActor)
			return;

		if (!collectibleActor->IsOnCooldown())
		{
			MoveToLocation(foundCollectibles[index]->GetActorLocation(), 0.5f, false, true, true, NULL, false);
			OnMoveToTarget();
			
			double totalTime = FPlatformTime::Seconds() - initialTime;
			DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.0f), "[Collectible] cpu time : " + FString::SanitizeFloat(totalTime) + "s", GetPawn(), FColor::Red, 1.0f);
			return;
		}
		else
		{
			foundCollectibles.RemoveAt(index);
		}
	}
}

void ASDTAIController::MoveToPlayer()
{
	ACharacter * playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!playerCharacter)
		return;

	MoveToActor(playerCharacter, 0.5f, false, true, true, NULL, false);
	OnMoveToTarget();
	return;
}

void ASDTAIController::SetSurrendingPos(FVector targetPos)
{
	m_surroundingPos = targetPos;
}

void ASDTAIController::MoveAroundPlayer()
{
	UpdateController* updateController = ((ASoftDesignTrainingGameMode*)GetWorld()->GetAuthGameMode())->updateController;
	if (!updateController->canUpdate(lastUpdate)) {
		return;
	}

	lastUpdate = GFrameCounter;

	double initialTime = FPlatformTime::Seconds(); //used to measure cpu time

	ACharacter * playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!playerCharacter)
		return;
	SDTAIGroupController* grpHunt = SDTAIGroupController::GetGroup();
	grpHunt->CreateSurroundingPos();

	MoveToLocation(m_surroundingPos, 0.5f, false, true, true, NULL, false);
	OnMoveToTarget();

	double totalTime = FPlatformTime::Seconds() - initialTime;
	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.0f), "[Chase] cpu time : " + FString::SanitizeFloat(totalTime) + "s", GetPawn(), FColor::Red, 1.0f);
	return;
}

void ASDTAIController::PlayerInteractionLoSUpdate()
{
	ACharacter * playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!playerCharacter)
		return;

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

	FHitResult losHit;
	GetWorld()->LineTraceSingleByObjectType(losHit, GetPawn()->GetActorLocation(), playerCharacter->GetActorLocation(), TraceObjectTypes);

	bool hasLosOnPlayer = false;

	if (losHit.GetComponent())
	{
		if (losHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER)
		{
			hasLosOnPlayer = true;
		}
	}

	if (hasLosOnPlayer)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(m_PlayerInteractionNoLosTimer))
		{
			GetWorld()->GetTimerManager().ClearTimer(m_PlayerInteractionNoLosTimer);
			m_PlayerInteractionNoLosTimer.Invalidate();
			DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Got LoS", GetPawn(), FColor::Red, 5.f, false);
		}
	}
	else
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(m_PlayerInteractionNoLosTimer))
		{
			GetWorld()->GetTimerManager().SetTimer(m_PlayerInteractionNoLosTimer, this, &ASDTAIController::OnPlayerInteractionNoLosDone, 3.f, false);
			DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Lost LoS", GetPawn(), FColor::Red, 5.f, false);
		}
	}

}

void ASDTAIController::OnPlayerInteractionNoLosDone()
{
	GetWorld()->GetTimerManager().ClearTimer(m_PlayerInteractionNoLosTimer);
	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "TIMER DONE", GetPawn(), FColor::Red, 5.f, false);

	if (!AtJumpSegment)
	{
		AIStateInterrupted();
		m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
	}
}

void ASDTAIController::MoveToBestFleeLocation()
{
	UpdateController* updateController = ((ASoftDesignTrainingGameMode*)GetWorld()->GetAuthGameMode())->updateController;
	if (!updateController->canUpdate(lastUpdate))
		return;

	lastUpdate = GFrameCounter;

	double initialTime = FPlatformTime::Seconds(); //used to measure cpu time

	float bestLocationScore = 0.f;
	ASDTFleeLocation* bestFleeLocation = nullptr;

	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!playerCharacter)
		return;

	for (TActorIterator<ASDTFleeLocation> actorIterator(GetWorld(), ASDTFleeLocation::StaticClass()); actorIterator; ++actorIterator)
	{
		ASDTFleeLocation* fleeLocation = Cast<ASDTFleeLocation>(*actorIterator);
		if (fleeLocation)
		{
			float distToFleeLocation = FVector::Dist(fleeLocation->GetActorLocation(), playerCharacter->GetActorLocation());

			FVector selfToPlayer = playerCharacter->GetActorLocation() - GetPawn()->GetActorLocation();
			selfToPlayer.Normalize();

			FVector selfToFleeLocation = fleeLocation->GetActorLocation() - GetPawn()->GetActorLocation();
			selfToFleeLocation.Normalize();

			float fleeLocationToPlayerAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(selfToPlayer, selfToFleeLocation)));
			float locationScore = distToFleeLocation + fleeLocationToPlayerAngle * 100.f;

			if (locationScore > bestLocationScore)
			{
				bestLocationScore = locationScore;
				bestFleeLocation = fleeLocation;
			}

			DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), FString::SanitizeFloat(locationScore), fleeLocation, FColor::Red, 5.f, false);
		}
	}

	if (bestFleeLocation)
	{
		MoveToLocation(bestFleeLocation->GetActorLocation(), 0.5f, false, true, false, NULL, false);
		OnMoveToTarget();
		double totalTime = FPlatformTime::Seconds() - initialTime;
		DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.0f), "[Flee] cpu time : " + FString::SanitizeFloat(totalTime) + "s", GetPawn(), FColor::Red, 1.0f);
		return;
	}
}

void ASDTAIController::OnMoveToTarget()
{
	m_ReachedTarget = false;
}

void ASDTAIController::RotateTowards(const FVector& targetLocation)
{
	if (!targetLocation.IsZero())
	{
		FVector direction = targetLocation - GetPawn()->GetActorLocation();
		FRotator targetRotation = direction.Rotation();

		targetRotation.Yaw = FRotator::ClampAxis(targetRotation.Yaw);

		SetControlRotation(targetRotation);
	}
}

void ASDTAIController::SetActorLocation(const FVector& targetLocation)
{
	GetPawn()->SetActorLocation(targetLocation);
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
	if (UPathFollowingComponent* pathFollowingComponent = GetPathFollowingComponent())
	{
		if (pathFollowingComponent->HasValidPath())
		{
			const FNavPathSharedPtr path = pathFollowingComponent->GetPath();
			TArray<FNavPathPoint> pathPoints = path->GetPathPoints();

			for (int i = 0; i < pathPoints.Num(); ++i)
			{
				DrawDebugSphere(GetWorld(), pathPoints[i].Location, 10.f, 8, FColor::Yellow);

				if (i != 0)
				{
					DrawDebugLine(GetWorld(), pathPoints[i].Location, pathPoints[i - 1].Location, FColor::Yellow);
				}
			}
		}
	}
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
	//finish jump before updating AI state
	if (AtJumpSegment)
		return;

	APawn* selfPawn = GetPawn();
	if (!selfPawn)
		return;

	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!playerCharacter)
		return;

	FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
	FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

	TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
	detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

	TArray<FHitResult> allDetectionHits;
	GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

	FHitResult detectionHit;
	GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

	UpdatePlayerInteractionBehavior(detectionHit, deltaTime);

	if (GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		m_ReachedTarget = true;
	}

	FString debugString = "";

	switch (m_PlayerInteractionBehavior)
	{
	case PlayerInteractionBehavior_Chase:
		debugString = "Chase";
		break;
	case PlayerInteractionBehavior_Flee:
		debugString = "Flee";
		break;
	case PlayerInteractionBehavior_Collect:
		debugString = "Collect";
		break;
	}

	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 5.f), debugString, GetPawn(), FColor::Orange, 0.f, false);

	DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

bool ASDTAIController::HasLoSOnHit(const FHitResult& hit)
{
	if (!hit.GetComponent())
		return false;

	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));

	FVector hitDirection = hit.ImpactPoint - hit.TraceStart;
	hitDirection.Normalize();

	FHitResult losHit;
	FCollisionQueryParams queryParams = FCollisionQueryParams();
	queryParams.AddIgnoredActor(hit.GetActor());

	GetWorld()->LineTraceSingleByObjectType(losHit, hit.TraceStart, hit.ImpactPoint + hitDirection, TraceObjectTypes, queryParams);

	return losHit.GetActor() == nullptr;
}

void ASDTAIController::AIStateInterrupted()
{
	StopMovement();
	m_ReachedTarget = true;
}

ASDTAIController::PlayerInteractionBehavior ASDTAIController::GetCurrentPlayerInteractionBehavior(const FHitResult& hit)
{
	if (m_PlayerInteractionBehavior == PlayerInteractionBehavior_Collect)
	{
		if (!hit.GetComponent())
			return PlayerInteractionBehavior_Collect;

		if (hit.GetComponent()->GetCollisionObjectType() != COLLISION_PLAYER)
			return PlayerInteractionBehavior_Collect;

		if (!HasLoSOnHit(hit))
			return PlayerInteractionBehavior_Collect;

		return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
	}
	else
	{
		PlayerInteractionLoSUpdate();

		return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
	}
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

void ASDTAIController::UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime)
{
	PlayerInteractionBehavior currentBehavior = GetCurrentPlayerInteractionBehavior(detectionHit);

	if (currentBehavior != m_PlayerInteractionBehavior)
	{
		m_PlayerInteractionBehavior = currentBehavior;
		AIStateInterrupted();
	}
}

void ASDTAIController::DetectPlayer() {
	if (AtJumpSegment)
		return;

	APawn* selfPawn = GetPawn();
	if (!selfPawn)
		return;

	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!playerCharacter)
		return;

	UpdateController* updateController = ((ASoftDesignTrainingGameMode*)GetWorld()->GetAuthGameMode())->updateController;
	if (!updateController->canUpdate(lastUpdate))
		return;
	lastUpdate = GFrameCounter;

	double initialTime = FPlatformTime::Seconds(); //used to measure cpu time

	FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
	FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

	TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
	detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

	TArray<FHitResult> allDetectionHits;
	GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

	FHitResult detectionHit;
	GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

	if (detectionHit.GetComponent() && detectionHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER && HasLoSOnHit(detectionHit))
		m_IsPlayerDetected = true;
	else
		m_IsPlayerDetected = false;

	m_targetPlayerPos = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation();
	
	double totalTime = FPlatformTime::Seconds() - initialTime;
	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.0f), "[Detect] cpu time : " + FString::SanitizeFloat(totalTime) + "s", GetPawn(), FColor::Yellow, 1.0f);
}

