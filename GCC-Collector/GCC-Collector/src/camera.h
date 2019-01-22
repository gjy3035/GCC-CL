#pragma once

#include "natives.h"
#include "types.h"
#include "enums.h"
#include "main.h"
#include <cmath>

const float PI = acos(1.0) * 2;
const float cameraSpeedFactor = 0.1;

extern bool CameraMode;
extern int adjustCameraFinished;

void startNewCamera();
void adjustCamera();
void StopCamera(int foldNo = 0);

bool showCamera();
void showCamera(float &camX, float &camY);

void getCameraLoc(float &camX, float &camY);
void getCameraLoc(float &camX, float &camY, float &camZ);

void show2False();
void showCamera4(int No = adjustCameraFinished);
bool saveCamera4(int No = adjustCameraFinished);
void gobackcamera();
//bool cameraToOrigin();