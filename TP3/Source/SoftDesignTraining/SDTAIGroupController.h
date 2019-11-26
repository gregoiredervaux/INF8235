#pragma once
#include "SDTAIController.h"
#include "CoreMinimal.h"


class SOFTDESIGNTRAINING_API SDTAIGroupController
{

public:
	static SDTAIGroupController* GetGroup();
	void addAgent(ASDTAIController* aiCharacter);
	void removeAgent(ASDTAIController* aiCharacter);
	void CreateSurroundingPos();

private:
	SDTAIGroupController();
	static SDTAIGroupController* m_GroupInstance;
	TArray<FVector> m_pointOnCircle;
	TArray<ASDTAIController*> m_hunter;
};

