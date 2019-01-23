#pragma once
#include "natives.h"
#include "infoIO.h"
#include <vector>

extern bool defineAreaMode;

bool setPoint();
bool resetPoint();
int saveOneArea();
void deleteBddefer();

struct pedLocation {
	float x, y;

	pedLocation();
	pedLocation(float _x, float _y);
};

void startDefineArea();
float cross(pedLocation &p0, pedLocation &p1, pedLocation &p2);
bool inCircle(float x, float y);
void showArea();