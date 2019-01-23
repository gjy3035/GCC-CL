#include "camera.h"
#include "script.h"
#include "utils.h"
#include "infoIO.h"
#include "keyboard.h"
#include "natives.h"
#include <string>

int adjustCameraFinished = 0;
bool CameraMode = false;

static Any cameraHandle;

void disableControls() {
	std::vector<int> disabledControls = {
		0,2,3,4,5,6,16,17,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,44,45,71,72,75,140,141,142,143,156,243,257,261,262,263,264,267,268,269,270,271,272,273
	};

	for (auto & controlCode : disabledControls) {
		CONTROLS::DISABLE_CONTROL_ACTION(0, controlCode, 1);
	}
}

void startNewCamera()
{
	//Find the location of our camera based on the current actor
	Ped actorPed = PLAYER::PLAYER_PED_ID();
	Vector3 startLocation = ENTITY::GET_ENTITY_COORDS(actorPed, true);
	float startHeading = ENTITY::GET_ENTITY_HEADING(actorPed);
	log_to_file("heading : " + std::to_string(startHeading));
	
	Vector3 camOffset;
	camOffset.x = (float)sin((startHeading *PI / 180.0f))*10.0f;
	camOffset.y = (float)cos((startHeading *PI / 180.0f))*10.0f;
	camOffset.z = 6.4;
	if (startLocation.x < 0) {
		camOffset.x = -camOffset.x;
	}
	if (startLocation.y < 0) {
		camOffset.y = -camOffset.y;
	}

	log_to_file("actor location (" + std::to_string(startLocation.x) + ", " + std::to_string(startLocation.y) + ", " + std::to_string(startLocation.z) + ")");
	log_to_file("Camera offset (" + std::to_string(camOffset.x) + ", " + std::to_string(camOffset.y) + ", " + std::to_string(camOffset.z) + ")");

	Vector3 camLocation = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(actorPed, camOffset.x, camOffset.y, camOffset.z);
	log_to_file("Camera location (" + std::to_string(camLocation.x) + ", " + std::to_string(camLocation.y) + ", " + std::to_string(camLocation.z) + ")");
	cameraHandle = CAM::CREATE_CAM_WITH_PARAMS("DEFAULT_SCRIPTED_CAMERA", camLocation.x, camLocation.y, camLocation.z, 0.0, 0.0, 0.0, 40.0, 1, 2);

	CAM::POINT_CAM_AT_ENTITY(cameraHandle, actorPed, 0.0f, 0.0f, 0.0f, true);
	WAIT(100);
	CAM::STOP_CAM_POINTING(cameraHandle);

	CAM::RENDER_SCRIPT_CAMS(true, 1, 1800, 1, 0);
	WAIT(2000);

	CameraMode = true;
}

void adjustCamera()
{
	disableControls();

	// movement
	Vector3 camDelta = {};
	float nfov = 0.0;
	bool isMovement = false;
	if (W.isKeyDown()) {
		camDelta.x = 1.0;
		isMovement = true;
	}
	if (S.isKeyDown()) {
		camDelta.x = -1.0;
		isMovement = true;
	}
	if (A.isKeyDown()) {
		camDelta.y = -1.0;
		isMovement = true;
	}
	if (D.isKeyDown()) {
		camDelta.y = 1.0;
		isMovement = true;
	}
	if (CONTROLS::IS_DISABLED_CONTROL_PRESSED(2, 329)) {//LMouseBtn
		camDelta.z = 0.3;
		isMovement = true;
	}
	if (CONTROLS::IS_DISABLED_CONTROL_PRESSED(2, 330)) {//RMouseBtn
		camDelta.z = -0.3;
		isMovement = true;
	}
	if (oemPlus.isKeyDown()) {
		nfov = 1.0;
		isMovement = true;
	}
	if (oemMinus.isKeyDown()) {
		nfov = -1.0;
		isMovement = true;
	}
	if (isMovement) {
		if (shift.isKeyDown()) {
			camDelta.x *= 3;
			camDelta.y *= 3;
			camDelta.z *= 3;
			nfov *= 3;
		}
		else if (ctrl.isKeyDown()) {
			camDelta.x /= 20;
			camDelta.y /= 20;
			camDelta.z /= 20;
			nfov /= 20;
		}

		Vector3 camNewPos = CAM::GET_CAM_COORD(cameraHandle);
		float fov = CAM::GET_CAM_FOV(cameraHandle);
		/*camLastPos.x = camNewPos.x;
		camLastPos.y = camNewPos.y;
		camLastPos.z = camNewPos.z;*/

		Vector3 camRot = {};
		camRot = CAM::GET_CAM_ROT(cameraHandle, 2);
		//camera rotation is not as expected. .x value is rotation in the z-plane (view up/down) and third paramter is the rotation in the x,y plane.

		Vector3 direction = {};
		direction = MathUtils::rotationToDirection(camRot);

		//forward motion
		if (camDelta.x != 0.0) {
			camNewPos.x += direction.x * camDelta.x * cameraSpeedFactor;
			camNewPos.y += direction.y * camDelta.x * cameraSpeedFactor;
			camNewPos.z += direction.z * camDelta.x * cameraSpeedFactor;
		}

		//sideways motion
		if (camDelta.y != 0.0) {
			//straight up
			Vector3 b = {};
			b.z = 1.0;

			Vector3 sideWays = {};
			sideWays = MathUtils::crossProduct(direction, b);

			camNewPos.x += sideWays.x * camDelta.y * cameraSpeedFactor;
			camNewPos.y += sideWays.y * camDelta.y * cameraSpeedFactor;
		}

		//up/down
		if (camDelta.z != 0.0) {
			camNewPos.z += camDelta.z * cameraSpeedFactor;
		}

		if (nfov != 0.0) {
			fov += nfov;
		}

		CAM::SET_CAM_COORD(cameraHandle, camNewPos.x, camNewPos.y, camNewPos.z);
		CAM::SET_CAM_FOV(cameraHandle, fov);
	}
	
	// rotation
	float rightAxisX = CONTROLS::GET_DISABLED_CONTROL_NORMAL(0, 220);
	float rightAxisY = CONTROLS::GET_DISABLED_CONTROL_NORMAL(0, 221);

	if (rightAxisX != 0.0 || rightAxisY != 0.0) {
		//Rotate camera - Multiply by sensitivity settings
		Vector3 currentRotation = CAM::GET_CAM_ROT(cameraHandle, 2);
		currentRotation.x += rightAxisY * -5.0f;
		currentRotation.z += rightAxisX * -10.0f;
		CAM::SET_CAM_ROT(cameraHandle, currentRotation.x, currentRotation.y, currentRotation.z, 2);
	}
}

void StopCamera(int foldNo)
{
	// write info into file
	Vector3 loc = CAM::GET_CAM_COORD(cameraHandle);
	Vector3 rot = CAM::GET_CAM_ROT(cameraHandle, 2);
	float fov = CAM::GET_CAM_FOV(cameraHandle);
	writeCamInfo(loc, rot, fov, foldNo);

	CAM::RENDER_SCRIPT_CAMS(false, 1, 1500, 1, 0);
	CameraMode = false;
	WAIT(1000);
}

static bool show = false;

bool showCamera()
{
	if (F10.isKeyDown()) {
		if (!show) {
			Vector3 loc, rot;
			float fov;
			readCamInfo(loc, rot, fov);
			log_to_file("get eye info done.");
			log_to_file(std::to_string(loc.x) + " " + std::to_string(loc.y) + " " + std::to_string(loc.z));
			log_to_file(std::to_string(rot.x) + " " + std::to_string(rot.y) + " " + std::to_string(rot.z));
			log_to_file(std::to_string(fov));
			cameraHandle = CAM::CREATE_CAM_WITH_PARAMS("DEFAULT_SCRIPTED_CAMERA", loc.x, loc.y, loc.z, rot.x, rot.y, rot.z, fov, 1, 2);
			WAIT(100);
			CAM::STOP_CAM_POINTING(cameraHandle);
			CAM::RENDER_SCRIPT_CAMS(true, 1, 2000, 1, 0);
			log_to_file("finished keep camera...");
		}
		else {
			CAM::RENDER_SCRIPT_CAMS(false, 1, 2000, 1, 0);
		}
		show = !show;
		return true;
	}
	else {
		return false;
	}
}

void showCamera(float &camX, float &camY)
{
	Vector3 loc, rot;
	float fov;
	readCamInfo(loc, rot, fov);
	cameraHandle = CAM::CREATE_CAM_WITH_PARAMS("DEFAULT_SCRIPTED_CAMERA", loc.x, loc.y, loc.z, rot.x, rot.y, rot.z, fov, 1, 2);
	WAIT(100);
	CAM::STOP_CAM_POINTING(cameraHandle);
	CAM::RENDER_SCRIPT_CAMS(true, 1, 2000, 1, 0);
	camX = loc.x, camY = loc.y;
	WAIT(2000);
	log_to_file("finished keep camera...");
}

void show2False()
{
	show = false;
}

void getCameraLoc(float &camX, float &camY)
{
	Vector3 loc, rot;
	float fov;
	readCamInfo(loc, rot, fov);
	camX = loc.x, camY = loc.y;
}

void getCameraLoc(float &camX, float &camY, float &camZ)
{
	Vector3 loc, rot;
	float fov;
	readCamInfo(loc, rot, fov);
	camX = loc.x, camY = loc.y, camZ = loc.z;
}

void showCamera4(int No)
{
	Vector3 loc, rot;
	float fov;
	changeFoldNo(No);
	readCamInfo(loc, rot, fov);
	log_to_file("get eye info done.");
	log_to_file(std::to_string(loc.x) + " " + std::to_string(loc.y) + " " + std::to_string(loc.z));
	log_to_file(std::to_string(rot.x) + " " + std::to_string(rot.y) + " " + std::to_string(rot.z));
	log_to_file(std::to_string(fov));
	cameraHandle = CAM::CREATE_CAM_WITH_PARAMS("DEFAULT_SCRIPTED_CAMERA", loc.x, loc.y, loc.z, rot.x, rot.y, rot.z, fov, 1, 2);
	WAIT(100);
	CAM::STOP_CAM_POINTING(cameraHandle);
	CAM::RENDER_SCRIPT_CAMS(true, 1, 2000, 1, 0);
	log_to_file("finished keep camera...");
	CameraMode = true;
}

bool saveCamera4(int No)
{
	if (F11.isKeyDown()) {
		CameraMode = false;
		Vector3 loc = CAM::GET_CAM_COORD(cameraHandle);
		Vector3 rot = CAM::GET_CAM_ROT(cameraHandle, 2);
		float fov = CAM::GET_CAM_FOV(cameraHandle);
		writeCamInfo(loc, rot, fov, No);
		adjustCameraFinished++;
		return true;
	}
	return false;
}

void gobackcamera()
{
	CAM::RENDER_SCRIPT_CAMS(false, 1, 2000, 1, 0);
	WAIT(2000);
}
