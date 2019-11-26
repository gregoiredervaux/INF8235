#include "SoftDesignTraining.h"
#include "UpdateController.h"

UpdateController::UpdateController()
{

}
void UpdateController::addAgent()
{
	nbAgents++;

	int nbExecutionsPerFrame = ALLOWED_EXECUTION_TIME / EXECUTE_TIME; //nb of agents that can be updated in a frame
	framesPerExecution = (int)ceil(nbAgents / nbExecutionsPerFrame);
	//UE_LOG(LogTemp, Warning, TEXT("Frames between execution : %s"), *FString::FromInt(framesPerExecution));

}
bool UpdateController::canUpdate(uint64 lastUpdate)
{
	//UE_LOG(LogTemp, Warning, TEXT("lastUpdate = %s"), *FString::FromInt(lastUpdate));
	uint64 framesPassed = GFrameCounter - lastUpdate;

	return framesPassed >= framesPerExecution;
}

