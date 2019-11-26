// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTBaseAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SDTAIController.generated.h"

/**
 *
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public ASDTBaseAIController
{
	GENERATED_BODY()

public:
	ASDTAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float m_DetectionCapsuleHalfLength = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float m_DetectionCapsuleRadius = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float m_DetectionCapsuleForwardStartingOffset = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		UCurveFloat* JumpCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float JumpApexHeight = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float JumpSpeed = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
		bool AtJumpSegment = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
		bool InAir = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
		bool Landing = false;

	virtual void BeginPlay() override;

	UBehaviorTreeComponent* GetBehaviorTreeComponent() const { return m_behaviorTreeComponent; }
	UBlackboardComponent*   GetBlackBoardComponent() const { return m_blackboardComponent; }

	uint8                   GetTargetPosBBKeyID() const { return m_targetPosBBKeyID; }
	uint8                   GetTargetSeenKeyID() const { return m_isTargetSeenBBKeyID; }
	bool					HasReachedDestination() const { return m_ReachedTarget; }

	bool					IsTargetPlayerSeen() const { return m_IsPlayerDetected; }
	FVector					GetTargetPlayerPos() const { return m_targetPlayerPos; }

	void                    StartBehaviorTree();
	void                    StopBehaviorTree();

	virtual void DetectPlayer();
	void MoveToRandomCollectible();
	void MoveToPlayer();
	void MoveAroundPlayer();
	void MoveToBestFleeLocation();

protected:

	enum PlayerInteractionBehavior
	{
		PlayerInteractionBehavior_Collect,
		PlayerInteractionBehavior_Chase,
		PlayerInteractionBehavior_Flee
	};

	void GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit);
	void UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime);
	PlayerInteractionBehavior GetCurrentPlayerInteractionBehavior(const FHitResult& hit);
	bool HasLoSOnHit(const FHitResult& hit);
	void PlayerInteractionLoSUpdate();
	void OnPlayerInteractionNoLosDone();
	void OnMoveToTarget();

	virtual void Possess(APawn* pawn) override;

public:
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
	void RotateTowards(const FVector& targetLocation);
	void SetActorLocation(const FVector& targetLocation);
	void SetSurrendingPos(FVector targetPos);
	void AIStateInterrupted();
	FVector m_surroundingPos;
	int lastUpdate;

private:
	virtual void GoToBestTarget(float deltaTime) override;
	virtual void UpdatePlayerInteraction(float deltaTime) override;
	virtual void ShowNavigationPath() override;

	UPROPERTY(transient)
	UBehaviorTreeComponent* m_behaviorTreeComponent;

	UPROPERTY(transient)
	UBlackboardComponent* m_blackboardComponent;

	uint8   m_targetPosBBKeyID;
	uint8   m_isTargetSeenBBKeyID;

	bool m_IsPlayerDetected;
	FVector m_targetPlayerPos;

protected:
	FVector m_JumpTarget;
	FRotator m_ObstacleAvoidanceRotation;
	FTimerHandle m_PlayerInteractionNoLosTimer;
	PlayerInteractionBehavior m_PlayerInteractionBehavior;
};
