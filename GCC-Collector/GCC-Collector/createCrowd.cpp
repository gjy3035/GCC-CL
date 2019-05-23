#include "createCrowd.h"
#include "infoIO.h"
#include "camera.h"
#include "defineArea.h"
#include "animation.h"
#include "setLevel.h"
#include <string>
#include <fstream>
#include <algorithm>
#include <set>
#include <time.h>
#include <atlimage.h>
#include <chrono>

/***** set random weather and time  *****/

char weathers[][12] = {
	"CLEAR", 		// 13 
	"RAIN", 		// 1
	"THUNDER", 		// 1
	"FOGGY", 		// 1
	"OVERCAST", 	// 1
	"CLOUDS", 		// 2
	"EXTRASUNNY"	// 5
};

std::string setTimeAndWeather() {
	// random seed
	srand((unsigned)time(NULL));
	
	// set random hour
	const double timeSeg[] = { 
		0.05,	// [0, 3)
		0.1, 	// [3, 6)
		0.3, 	// [6, 9)
		0.5,	// [9, 12)
		0.7, 	// [12, 15)
		0.9, 	// [15, 18)
		0.95, 	// [18, 21)
		1.00 	// [21, 24)
	};
	const double eps = 1e-7;

	double hourSelect = rand() * 1.0 / RAND_MAX;
	int hour = 0, hourSelectNum = sizeof(timeSeg) / sizeof(timeSeg[0]);
	for (int &i = hour; i < hourSelectNum; i++) {
		if ((hourSelect - timeSeg[i]) < eps) break;
	}
	log_to_file("selected hour in timeSeg[" + std::to_string(hour) + "]");
	hour = (hour * 3 + rand() % 3) % 24;

	while (TIME::GET_CLOCK_HOURS() != hour) {
		TIME::ADD_TO_CLOCK_TIME(1, 0, 0);
		WAIT(100);
	}
	log_to_file("hour = " + std::to_string(hour));
	int m = TIME::GET_CLOCK_MINUTES();
	int s = TIME::GET_CLOCK_SECONDS();

	// set random weather
	const int weatherSeg[] = { 13, 14, 15, 16, 17, 19, 24 };
	const int wsgLen = sizeof(weatherSeg) / sizeof(weatherSeg[0]);
	int weatherselect = rand() % weatherSeg[wsgLen - 1];
	int wNo = 0;
	for (int &i = wNo; i < wsgLen; i++) {
		if (weatherselect < weatherSeg[i]) break;
	}
	GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST(weathers[wNo]);
	WAIT(2000);
	log_to_file("hour = " + std::to_string(hour) + ", " + std::string(weathers[wNo]));
	
	return std::string(weathers[wNo]);
}


/***** with all setted parameter creating one crowd image  *****/

// pedestrian struct
struct pedestrian {
	static float camX, camY;
	float x, y, z, heading;
	Hash pedhash;
	Ped pedActor;
	Animation anim;
	int animid;

	bool operator < (const pedestrian &t)const {
		float d1 = (x - camX) * (x - camX) + (y - camY) * (y - camY);
		float d2 = (t.x - camX) * (t.x - camX) + (t.y - camY) * (t.y - camY);
		return d1 > d2;
	}
};
float pedestrian::camX = 0;
float pedestrian::camY = 0;
// peds' size must be greater than the maximum maxNum in level[] in setLevel.h
pedestrian peds[4000 + 5];

bool createPeds(int &n, float &sx, float &dx, float &sy, float &dy) {
	float x, y, avgz = readZheight() + 0.5;
	log_to_file("average height = " + std::to_string(avgz));
	
	bool noAnim = false;
	if (n >= readLevelFile() * 0.5) noAnim = true;
	console_log(std::string("no anims : ") + (noAnim ? "true" : "false"));

	srand((unsigned)time(NULL));

	int animsNum = animNum();
	for (int i = 0, j; i < n; i++) {
		j = 0;
		do {
			x = dx * rand() / RAND_MAX + sx;
			y = dy * rand() / RAND_MAX + sy;
			j++;
		} while (!inCircle(x, y));

		peds[i].x = x, peds[i].y = y, peds[i].z = avgz;
		j = rand() % 265;
		peds[i].pedhash = pedHashes[j];
		peds[i].heading = 360.0f * rand() / RAND_MAX;
		peds[i].animid = rand() % animsNum;
		peds[i].anim = getAnim(peds[i].animid);
	}
	std::sort(peds, peds + n);
	log_to_file("finished create " + std::to_string(n) + " Points ...");
	return noAnim;
}

void resetPedLoc(int n) {
	// create the pedestrian with the specific id
	auto createPed = [](int id) {
		STREAMING::REQUEST_MODEL(peds[id].pedhash);
		while (!STREAMING::HAS_MODEL_LOADED(peds[id].pedhash)) WAIT(0);
		peds[id].pedActor = PED::CREATE_PED(4, peds[id].pedhash, peds[id].x, peds[id].y, peds[id].z, peds[id].heading, false, true);
	};

	// delete the pedestrian with the specific id
	auto deletePed = [](int id) {
		bool isAlive = ENTITY::IS_ENTITY_DEAD(peds[id].pedActor);
		Vector3 position = ENTITY::GET_ENTITY_COORDS(peds[id].pedActor, isAlive);
		peds[id].x = position.x, peds[id].y = position.y, peds[id].z = position.z;
		PED::DELETE_PED(&(peds[id].pedActor));
	};

	// reset pedestrian location to avoid collision
	int tail = 0, head = 0, step = 20;
	while (head < min(100, n)) createPed(head++);
	WAIT(1000);
	while (head < n)
	{
		step = min(20, n - head);
		for (int i = 0; i < step; i++) {
			createPed(head++);
			deletePed(tail++);
		}
		WAIT(1000);
	}
	while (tail < head) deletePed(tail++);
	WAIT(10);
	std::sort(peds, peds + n);

	log_to_file("reset pedestrian positions done.");
}

void createImgFold(char *foldName, char* pedInfoFile, char* rawPathTemp, WCHAR* imgPathTemp) {
	time_t foldNo = time(NULL);
	char foldNameTemp[fileLength] = "%I64d\\";
	
	// create fold
	foldCat(foldNameTemp);
	sprintf(foldName, foldNameTemp, foldNo);
	CreateDirectory(foldName, NULL);
	log_to_file("fold path : " + std::string(foldName));

	// raw file name
	strcpy(rawPathTemp, foldName);
	strcat(rawPathTemp, "part_%d.raw");

	// xml file name
	strcpy(pedInfoFile, foldName);
	strcat(pedInfoFile, "pedInfo.xml");
	log_to_file("pedInfo path: " + std::string(pedInfoFile));

	// image file name
	WCHAR imgPathTempTemp[fileLength] = L"\\%I64d\\part_%%d_%%d.bmp";
	foldCat(imgPathTempTemp);
	swprintf(imgPathTemp, fileLength, imgPathTempTemp, foldNo);
}

void screenCap(int id, WCHAR* imgPathTemp, char* rawPathTemp) {
	WCHAR imgPath[fileLength];
	swprintf(imgPath, fileLength, imgPathTemp, id, 0);

	char rawPath[fileLength];
	sprintf(rawPath, rawPathTemp, id);

	int t = 0;
	do {
		catchCurveAndScreen(imgPath, rawPath, true, true);
		WAIT(1); t++;
		if (t > 400) {
			float mix, mxx, miy, mxy;
			readAreaBorder(mix, mxx, miy, mxy);

			float cx = (mix + mxx) / 2, cy = (miy + mxy) / 2, camX, camY;
			getCameraLoc(camX, camY);

			Ped myPed = PLAYER::PLAYER_PED_ID();
			Vector3 playerLoc = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(myPed, 0.0, 0.0, 0.0);

			cx = playerLoc.x + 5 * (camX < playerLoc.x ? 1 : -1);
			cy = playerLoc.y + 5 * (camY < playerLoc.y ? 1 : -1);

			ENTITY::SET_ENTITY_COORDS_NO_OFFSET(PLAYER::PLAYER_PED_ID(), cx, cy, 1000, 0, 0, 1);
			WAIT(20000);
			log_to_file(std::to_string(cx) + " : " + std::to_string(cy));
			t = 0;
		}
		if (t % 10 == 0) {
			log_to_file("t = " + std::to_string(t));
		}
	} while (!fileExist(imgPath));
	std::wstring ws1(imgPath);
	log_to_file(std::string(ws1.begin(), ws1.end()) + ", time = " + std::to_string(t));
}

void screenStencilCap(int id, WCHAR* imgPathTemp, char* rawPathTemp) {
	WCHAR imgPath[fileLength];
	swprintf(imgPath, fileLength, imgPathTemp, id, 1);
	char  rawPath[fileLength];
	sprintf(rawPath, rawPathTemp, id);
	int t = 0;
	while (!fileExist(rawPath)) {
		catchCurveAndScreen(imgPath, rawPath, false);
		WAIT(1); t++;
	}
	log_to_file("catch stencil and screen, time = " + std::to_string(t));
}

void createOneScene(int hashlen, float sx, float dx, float sy, float dy, std::string weather, int pedNum = 520) {

	log_to_file("the pedestrian number of the scene: " + std::to_string(pedNum));

	// step 1: create pedNum random Points
	bool noAnim = createPeds(pedNum, sx, dx, sy, dy);

	// step 2: reset pedestrians' positions
	resetPedLoc(pedNum);

	// step 3: create crowd image information fold
	char foldName[fileLength], pedInfoFile[fileLength],rawPathTemp[fileLength];
	WCHAR imgPathTemp[fileLength];
	createImgFold(foldName, pedInfoFile, rawPathTemp, imgPathTemp);

	// step 4: create scene and cap screen
	const int maxPedOneTime = 180;
	int m = min(pedNum, maxPedOneTime), ms = 0;

	TIME::PAUSE_CLOCK(true);
	Animation animation;
	log_to_pedTxt("<pedSet>", pedInfoFile);
	while (ms < pedNum) {
		
		// place (m - ms) pedestrians
		log_to_file("create (" + std::to_string(m) + " - " + std::to_string(ms) + ") pedestrians.");
		for (int i = ms; i < m; i++) {
			STREAMING::REQUEST_MODEL(peds[i].pedhash);
			while (!STREAMING::HAS_MODEL_LOADED(peds[i].pedhash)) WAIT(0);
			peds[i].pedActor = PED::CREATE_PED(4, peds[i].pedhash, peds[i].x, peds[i].y, peds[i].z, peds[i].heading, false, true);
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(peds[i].pedhash);
			animation = peds[i].anim;
			bool ex = STREAMING::DOES_ANIM_DICT_EXIST(animation.animLibrary);
			if (ex) {
				STREAMING::REQUEST_ANIM_DICT(animation.animLibrary);
				while (!STREAMING::HAS_ANIM_DICT_LOADED(animation.animLibrary)) WAIT(0);
			}
		}
		WAIT(2000);
		log_to_file("out create pedestrian");
		// perform animation
		int performin = 10000;
		if (!noAnim) {
			for (int i = ms; i < m; i++) {
				animation = peds[i].anim;
				AI::TASK_PLAY_ANIM(peds[i].pedActor, animation.animLibrary, animation.animName, 8.0f, -8.0f, animation.duration, true, 8.0f, 0, 0, 0);
				performin = min(performin, animation.duration);
			}
			log_to_file("perform animation : " + std::to_string(performin));
		}
		WAIT(1000);

		// write ped label infomation xml file
		const int headCode = 0x796E;
		std::string shx, shy;
		auto getBonePixel = [headCode, &shx, &shy](pedestrian thisPed)
		{
			Vector3 boneLoc = PED::GET_PED_BONE_COORDS(thisPed.pedActor, headCode, 0, 0, 0);
			float hx, hy;
			BOOL getCoord = GRAPHICS::_WORLD3D_TO_SCREEN2D(boneLoc.x, boneLoc.y, boneLoc.z, &hx, &hy);
			shx = std::to_string(hx);
			shy = std::to_string(hy);
		};

		for (int i = ms; i < m; i++) {
			std::string pedInfo = "\t<ped id=" + std::to_string(i) + ">\n";
			getBonePixel(peds[i]);
			pedInfo += "\t\t<SKEL_Head x=" + shx + " y=" + shy + " />\n";
			pedInfo += "</ped>";
			log_to_pedTxt(pedInfo, pedInfoFile);
		}

		// capture stencial raw file
		screenStencilCap(ms, imgPathTemp, rawPathTemp);

		// delete pedestrian from screen
		for (int i = ms; i < m; i++) {
			PED::DELETE_PED(&(peds[i].pedActor));
		}
		WAIT(10);
		// capture background
		screenCap(ms, imgPathTemp, rawPathTemp);

		// update m and ms
		m += maxPedOneTime, ms += maxPedOneTime;
		if (m >= pedNum) m = pedNum;
		log_to_file(std::to_string(m) + " / " + std::to_string(pedNum) + " pedestrians render done.");
	}
	log_to_pedTxt("</pedSet>", pedInfoFile);

	// record weather type
	log_to_pedTxt("<weather>" + weather + "</weather>", pedInfoFile);

	// record time information
	int h = TIME::GET_CLOCK_HOURS();
	int mis = TIME::GET_CLOCK_MINUTES();
	log_to_pedTxt("<time hour=" + std::to_string(h) + "minutes=" + std::to_string(mis) + " />", pedInfoFile);

	TIME::PAUSE_CLOCK(false);

	log_to_file("finished create this scene.");
}

void createScene(int nowHave) {
	log_to_file("let's create one scene.");

	float mix, mxx, miy, mxy;
	readAreaBorder(mix, mxx, miy, mxy);

	float cx = (mix + mxx) / 2, cy = (miy + mxy) / 2, camX, camY, camZ;
	getCameraLoc(camX, camY, camZ);
	cx = camX + 110 * (cx < camX ? 1 : -1); // -+
											//  | outdoor
	cy = camY + 110 * (cy < camY ? 1 : -1); // -+

	float z = readZheight() + 0.5;
	Ped myPed = PLAYER::PLAYER_PED_ID();
	Vector3 playerLoc = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(myPed, 0.0, 0.0, 0.0);
	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(PLAYER::PLAYER_PED_ID(), cx, cy, 1000, 0, 0, 1); // outdoor
	// ENTITY::SET_ENTITY_COORDS_NO_OFFSET(PLAYER::PLAYER_PED_ID(), camX, camY, camZ - 0.5, 0, 0, 1); // indoor
	WAIT(20000);

	log_to_file(std::to_string(mix) + " " + std::to_string(mxx) + " " + std::to_string(miy) + " " + std::to_string(mxy));
	
	// step 0: put camera
	showCamera(pedestrian::camX, pedestrian::camY);

	log_to_file("start time: " + std::to_string(time(NULL)));

	auto runSceneCreator = [&](int avg2) {
		int m = readImgNum();
		for (int i = nowHave; i < m; i++) {
			log_to_file("m = " + std::to_string(m));
			std::string weather = setTimeAndWeather();
			log_to_file("avg2 = " + std::to_string(avg2));
			int pedNum = avg2 * 0.25 + rand() % int(avg2 * 0.75) + 1;
			createOneScene(281, mix, mxx - mix, miy, mxy - miy, weather, pedNum);
		}
	};
	runSceneCreator(readLevelFile());
	log_to_file("end time: " + std::to_string(NULL));
	log_to_file("one scene created end..");
	gobackcamera();
}
