#include "defineArea.h"
#include "infoIO.h"
#include "keyboard.h"
#include <vector>

bool defineAreaMode = false;

const Hash pedHashes[] = { 664399832, 4209271110, 3886638041 };
static Ped myPed, bddefer;
std::vector<Vector3> areaPoints;
static int now = 0;

void startDefineArea()
{
	myPed = PLAYER::PLAYER_PED_ID();
	areaPoints.clear();
	areaPoints.resize(0);

	//boundary definer
	Vector3 loc = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(myPed, 0.0, 1.0, 0.0);
	float startHeading = ENTITY::GET_ENTITY_HEADING(myPed);
	const Hash ha = 664399832;
	STREAMING::REQUEST_MODEL(ha);
	while (!STREAMING::HAS_MODEL_LOADED(ha)) WAIT(0);
	bddefer = PED::CREATE_PED(4, ha, loc.x, loc.y, loc.z, startHeading, false, true);
	PLAYER::CHANGE_PLAYER_PED(PLAYER::PLAYER_ID(), bddefer, false, false);
	now = 0;
	areaPoints.push_back(loc);
	log_to_file("set points No." + std::to_string(now));
	set_status_text("set points No." + std::to_string(now + 1));
}

void defineAreaDisableControls()
{
	std::vector<int> disabledControls = {
		17, 18, 19, 20, 21, 22, 42, 58, 59, 61, 104, 105, 120, 121, 162, 163, 164, 165, 166, 167,
		168, 169, 170, 266, 267
	};

	for (auto & controlCode : disabledControls) {
		CONTROLS::DISABLE_CONTROL_ACTION(0, controlCode - 5, 1);
	}
}

bool setPoint()
{
	if (I.isKeyDown()) {
		Vector3 newLoc = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(bddefer, 0.0, 0.0, 0.0);
		areaPoints.push_back(newLoc);
		now++;
		log_to_file("set points No." + std::to_string(now));
		set_status_text("set points No." + std::to_string(now + 1));
		return true;
	}
	return false;
}

bool resetPoint()
{
	defineAreaDisableControls();
	if (tab.isKeyDown()) {
		if(areaPoints.empty()) return false;

		now = (now + 1) % areaPoints.size();
		Vector3 tp = areaPoints[now];
		ENTITY::SET_ENTITY_COORDS_NO_OFFSET(bddefer, tp.x, tp.y, tp.z + 0.2, 0, 0, 1);
		WAIT(500);
		log_to_file("re-set point No." + std::to_string(now) );
		set_status_text("re-set points No." + std::to_string(now + 1));
		return true;
	}
	return false;
}

void deleteBddefer()
{
	PLAYER::CHANGE_PLAYER_PED(PLAYER::PLAYER_ID(), myPed, false, false);
	float h = -1000;
	const int n = areaPoints.size();
	for (int i = 0; i < n; i++) {
		Vector3 vecpo = areaPoints[i];
		h = max(h, vecpo.z);
	}
	PED::DELETE_PED(&bddefer);
	writeZheight(h);
	defineAreaMode = false;
}

int saveOneArea()
{
	if (F11.isKeyDown()) {
		int n = areaPoints.size();
		writeAreaInfo(n);
		for (int i = 0; i < n; i++) {
			Vector3 pointLoc = areaPoints[i];
			writeAreaInfo(pointLoc);
		}
		log_to_file("Points write into file done.");
		return n;
	}
	return -1;
}

void showArea()
{
	int n = areaPoints.size();
	if (n > 0) {
		Vector3 nowLoc = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(bddefer, 0.0, 0.0, 0.0);
		areaPoints[now] = nowLoc;
	}
	if (n < 2) return;
	for (int c = 0; c < n; c++) {
		Vector3 &loc_A = areaPoints[c];
		Vector3 &loc_B = areaPoints[(c + 1) % n];
		GRAPHICS::DRAW_LINE(loc_A.x, loc_A.y, loc_A.z, 
			loc_B.x, loc_B.y, loc_B.z,
			255, 0, 0, 150);
	}
}

/******* pedestrian in defined area judger **********/

pedLocation::pedLocation() {
	x = 0, y = 0;
}

pedLocation::pedLocation(float _x, float _y) : x(_x), y(_y) { }

float cross(pedLocation &p0, pedLocation &p1, pedLocation &p2) {
	return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
}

bool inCircle(float x, float y) {
	int n, crossing = 0;
	std::vector<pedLocation> pedLocations;
	readAreaInfo(n, pedLocations);
	pedLocations.push_back(*pedLocations.begin());
	for (int i = 0; i < n; i++) {
		double slope = (pedLocations[i + 1].y - pedLocations[i].y) / (pedLocations[i + 1].x - pedLocations[i].x);
		bool cond1 = (pedLocations[i].x <= x) && (x < pedLocations[i + 1].x);
		bool cond2 = (pedLocations[i + 1].x <= x) && (x < pedLocations[i].x);
		bool above = (y < slope * (x - pedLocations[i].x) + pedLocations[i].y);
		if ((cond1 || cond2) && above) crossing++;
	}
	return crossing & 1;
}