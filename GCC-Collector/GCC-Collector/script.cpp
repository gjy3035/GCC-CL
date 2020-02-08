#include "script.h"
#include "export.h"
#include "main.h"
#include "infoIO.h"
#include "camera.h"
#include "defineArea.h"
#include "animation.h"
#include "setLevel.h"
#include "createCrowd.h"
#include <string>
#include <fstream>
#include <algorithm>
#include <set>
#include <atlimage.h>
#include <time.h>
#include <chrono>
#include <cmath>

scriptStatusEnum scriptStatus = scriptStop;

void scriptMain()
{
	initDataDir();

	int sleepTime = 0;
	bool aniGet = initAnimations("SceneDirectorAnim.txt");
	bool partGet = InitPartNo();
	set_status_text("GCC-CL start fine!!!");
	while (true)
	{
		if (scriptStatus == scriptStart) {
			while (1) {
				int nowHave = defaultFold();
				log_to_file("now have = " + std::to_string(nowHave));
				if (nowHave != -1) {
					createScene(nowHave);

				}
				WAIT(1000);
				if (scriptStatus == scriptStop) { break; }
			}
			scriptStatus = scriptStop, sleepTime = 0;
		}
		else if (scriptStatus == scriptReady) {
			createNewFold();
			scriptStatus = scriptReadyCamera;
			set_status_text("scene fold has been ready. Now push F10 to set the camera");
		}
		else if (scriptStatus == cameraMode) {
			if (CameraMode == false) {
				startNewCamera();
				set_status_text("Now you can set the camera location.");
			}
			else {
				adjustCamera();
			}
				
		}
		else if (scriptStatus == cameraModeEnd && CameraMode == true) {
			StopCamera();
			scriptStatus = scriptReadyDefineArea;
		}
		else if (scriptStatus == defineArea) {
			if (defineAreaMode == false) {
				set_status_text("now you can start define the area boundary.");
				startDefineArea();
				defineAreaMode = true;
			}
			if (setPoint() || resetPoint()) {
				WAIT(500);
			}
			if(showCamera()) {
				WAIT(2000);
			}
			showArea();

			int save = saveOneArea();
			if (save > 0) {
				if (save < 3) {
					set_status_text("at least 3 points for boundary defination");
				}
				else {
					scriptStatus = defineAreaEnd;
					show2False();
					set_status_text("finished boundary defination");
				}
				WAIT(500);
			}
		}
		else if (scriptStatus == defineAreaEnd) {
			if (adjustCameraFinished == 0 && CameraMode == false) {
				wriet4Camera();
			}
			if (adjustCameraFinished >= 4) {
				deleteBddefer();
				gobackcamera();
				scriptStatus = scriptReadySetLevel;
				adjustCameraFinished = 0;
				log_to_file("end defineArea");
				set_status_text("end defineArea, push F12 to set crowd number level");
			}
			else {
				if (CameraMode == false) {
					showCamera4();
					set_status_text("adjust No." + std::to_string(adjustCameraFinished + 1) + " camera");
				}
				else {
					adjustCamera();
					if (saveCamera4()) {
						set_status_text("No." + std::to_string(adjustCameraFinished) + " save Ok.");
						WAIT(1000);
					}
				}
				showArea();
			}
		}
		else if (scriptStatus == setLevel) {
			if (!setLevelMode) {
				createLevelSaveFile();
				WAIT(1000);
				setLevelMode = true;
			}
			else setSceneLevel();
		}
		else if (scriptStatus == setLevelEnd) {
			stopSetLevel();
			scriptStatus = scriptEndReady;
			set_status_text("level has been setted, now you can press F5 to end this and go to next scene to create new one.");
		}
		else {
			if (++sleepTime == 2000) {
				log_to_file("sleep 2000 game ms...");
				log_to_file(std::to_string(scriptStatus));
				sleepTime = 0;
			}
			WAIT(1);
		}
		WAIT(0);
		
	}
}
