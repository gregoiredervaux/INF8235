#pragma once
#include "CoreGlobals.h"

class UpdateController
{
public:
	UpdateController();
	void addAgent();
	bool canUpdate(uint64);
private:
	const double ALLOWED_EXECUTION_TIME = 1.0f / 30.0f; //30 fps
	const double EXECUTE_TIME = 0.005f;
	int nbAgents = 0;
	int framesPerExecution; //minimum delay between executions
};

