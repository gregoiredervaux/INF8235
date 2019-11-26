#include "SoftDesignTraining.h"
#include "SDTAIGroupController.h"
#include "DrawDebugHelpers.h"
#include <cmath>
#include "SoftDesignTrainingMainCharacter.h"


// singleton (par variable de classe)
SDTAIGroupController* SDTAIGroupController::m_GroupInstance;

SDTAIGroupController::SDTAIGroupController()
{
}

SDTAIGroupController* SDTAIGroupController::GetGroup()
{
	if (!m_GroupInstance)
		m_GroupInstance = new SDTAIGroupController();

	return m_GroupInstance;
}

void SDTAIGroupController::addAgent(ASDTAIController* aiCharacter)
{
	m_hunter.AddUnique(aiCharacter);
}

void SDTAIGroupController::removeAgent(ASDTAIController* aiCharacter)
{
	m_hunter.Remove(aiCharacter);
}

void SDTAIGroupController::CreateSurroundingPos() {
	if (m_hunter.Num() < 1) return;

	m_pointOnCircle.Empty();
	AAIController* aiController = m_hunter[0];
	ACharacter * playerCharacter = UGameplayStatics::GetPlayerCharacter(aiController->GetWorld(), 0);
	FVector playerPosition = playerCharacter->GetActorLocation();

	float r = 150.f;
	float angle = 0.f;
	DrawDebugCircle(aiController->GetWorld(), playerPosition, 2 * r, 20, FColor::Red);

	for (int i = 0; i < m_hunter.Num(); i++) {

		FVector targetPos = playerPosition;
		targetPos.X += r * cos(angle);
		targetPos.Y += r * sin(angle);
		m_pointOnCircle.Add(targetPos);
		angle += 2.f * PI / float(m_hunter.Num());
		DrawDebugSphere(aiController->GetWorld(), targetPos + FVector(0.f, 0.f, 100.f), 25.0f, 32, FColor::Red);
		m_hunter[i]->SetSurrendingPos(targetPos);
	}

}
