#pragma once
#include "natives.h"
#include "types.h"
#include "enums.h"
#include "main.h"

enum scriptStatusEnum {
	scriptStart,
	scriptStop,
	scriptReady,
	scriptReadyCamera,
	cameraMode,
	cameraModeEnd,
	scriptReadyDefineArea,
	defineArea,
	defineAreaEnd,
	scriptReadySetLevel,
	setLevel,
	setLevelEnd,
	scriptEndReady
};
extern scriptStatusEnum scriptStatus;
void scriptMain();