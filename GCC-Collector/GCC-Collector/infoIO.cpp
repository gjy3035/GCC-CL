#include "types.h"
#include "infoIO.h"
#include "camera.h"
#include "setLevel.h"
#include <fstream>
#include <string>
#include <io.h>
#include <filesystem>
#include <sstream>
#include <vector>
#include <Windows.h>

void set_status_text(std::string text)
{
	UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING((LPSTR)text.c_str());
	UI::_DRAW_NOTIFICATION(1, 1);
}

void log_to_pedTxt(std::string message, char *fileName) {
	std::ofstream logfile(fileName, std::ios_base::app);
	logfile << message + "\n";
	logfile.close();
}

void log_to_file(std::string msg)
{
	char *logfile = "GCC-CL.log";
	std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
		std::chrono::system_clock::now().time_since_epoch()
		);
	char sec[20];
	sprintf(sec, "[%I64d] : ", ms);
	msg = std::string(sec) + msg;
	log_to_pedTxt(msg, logfile);
}

bool initDataDir()
{
	LPCSTR dataroot = "data";
	return GetFileAttributesA(dataroot) == dirMark || CreateDirectory(dataroot, NULL);
}

char nowFold[fileLength] = "data\\";
char nowFolds[4][fileLength] = { "data\\", "data\\", "data\\", "data\\" };

std::vector<int> parts, subParts, imageNums;
bool InitPartNo()
{
	parts.clear(); subParts.clear(); imageNums.clear();

	std::string root_dir = ".\\data\\", subFold;
	for (auto & p : std::experimental::filesystem::v1::directory_iterator(root_dir)) {
		std::stringstream conv;
		conv << p; conv >> subFold;
		int st = subFold.find("part_");
		if (st != -1) {
			st += 5;
			int foldLen = subFold.length();
			int subLen = foldLen - 2 - st;
			if (subFold.find("_", st + 5) == -1) subLen += 2;
			
			parts.push_back(std::stoi(subFold.substr(st, subLen)));
			subParts.push_back(std::stoi(subFold.substr(subFold.length() - 1)));
			std::string imgFold;
			int thisImgNum = 0;
			log_to_file("subFold = " + subFold);
			for (auto & subP : std::experimental::filesystem::v1::directory_iterator(subFold)) {
				std::stringstream subconv;
				subconv << subP; subconv >> imgFold;
				log_to_file(imgFold);
				if(GetFileAttributesA(imgFold.c_str()) == dirMark) thisImgNum ++;
			}
			imageNums.push_back(thisImgNum);
			log_to_file(subFold.substr(st, subLen) + " " + subFold.substr(subFold.length() - 1) + " " + std::to_string(thisImgNum));
		}
	}
	return true;
}

int workId = -1;
int defaultFold()
{
	int imgNum = readImgNum(), sceneNum = parts.size();
	if(workId == -1) {
		for(workId = 0; workId < sceneNum; workId++) {
			if(imageNums[workId] < imgNum) break;
		}
		if(workId >= sceneNum) return -1;
	}

	std::string foldName = "part_" + std::to_string(parts[workId]) + "_" + std::to_string(subParts[workId]);
	strcpy(nowFold, "data\\");
	strcat(nowFold, foldName.c_str());
	log_to_file("deault fold is " + std::string(foldName));
	return imageNums[workId];
}

void markAddOneImage() {
	int thisImgNum = ++ imageNums[workId];
	if(readImgNum() == thisImgNum) workId = -1;
}

void createNewFold()
{
	std::string root_dir = ".\\data\\", subFold;
	bool exist[partLength] = {false};
	for (auto & p : std::experimental::filesystem::v1::directory_iterator(root_dir)) {
		std::stringstream conv;
		conv << p; conv >> subFold;
		int st = subFold.find("part_");
		if (st != -1) {
			st += 5;

			int subLen = subFold.length() - 2 - st;
			if (subFold.find("_", st + 5) == -1) subLen += 2;
			int foldNo = std::stoi(subFold.substr(st, subLen));
			exist[foldNo] = true;
		}
	}

	auto createFoldExe = [](int part) {
		for (int subpart = 0; subpart < 4; subpart++) {
			strcpy(nowFolds[subpart], "data\\");
			strcat(nowFolds[subpart], ("part_" + std::to_string(part)).c_str());
			strcat(nowFolds[subpart], ("_" + std::to_string(subpart)).c_str());
			CreateDirectory(nowFolds[subpart], NULL);
			log_to_file("create new fold = " + std::string(nowFold));
		}
	};

	for (int i = partLength - 1; i > 0; i--) {
		if (exist[i]) {
			i = i + 1;
			createFoldExe(i);
			strcpy(nowFold, nowFolds[0]);
			return;
		}
	}
	createFoldExe(1);
}

void changeFoldNo(int No)
{
	strcpy(nowFold, nowFolds[No]);
}

void foldCat(char *subString, char *useFold)
{
	char newString[fileLength] = "\\";
	strcat(newString, subString);
	strcpy(subString, useFold);
	strcat(subString, newString);
}

void foldCat(char *subString)
{
	foldCat(subString, nowFold);
}

void foldCat(WCHAR *substring, char *useFold)
{
	WCHAR newString[fileLength];
	swprintf(newString, fileLength, L"%hs\\", useFold);
	wcscat(newString, substring);
	wcscpy(substring, newString);
}

 void foldCat(WCHAR *substring)
 {
 	foldCat(substring, nowFold);
 }

void writeCamInfo(const Vector3 &loc, const Vector3 &rot, const float &fov, int foldNo)
{
	char eyeInfoFile[fileLength] = "eyeInfo.log";
	foldCat(eyeInfoFile, nowFolds[foldNo]);
	std::ofstream info(eyeInfoFile);
	info << loc.x << " " << loc.y << " " << loc.z << std::endl;
	info << rot.x << " " << rot.y << " " << rot.z << std::endl;
	info << fov;
	info.close();
}

void writeCamInfo(const Vector3 &loc, const Vector3 &rot, const float &fov)
{
	writeCamInfo(loc, rot, fov, 0);
}

void wriet4Camera()
{
	char cpNowFold[fileLength];
	strcpy(cpNowFold, nowFold);
	strcpy(nowFold, nowFolds[0]);
	Vector3 loc, rot; float fov;
	readCamInfo(loc, rot, fov);

	float mix, mxx, miy, mxy;
	readAreaBorder(mix, mxx, miy, mxy);
	float cx = (mxx + mix) / 2;
	float cy = (mxy + miy) / 2;
	log_to_file(std::to_string(cx) + " " + std::to_string(cy));

	for (int i = 0; i < 4; i++) {
		writeCamInfo(loc, rot, fov, i);

		float nx = -(loc.y - cy) + cx;
		float ny = cy + loc.x - cx;
		loc.x = nx, loc.y = ny;

		float &r = rot.z;
		r += 90;
		if (r > 180) r - 180 - 180;
	}
	strcpy(nowFold, cpNowFold);
}

void readCamInfo(float &locx, float &locy, float &locz, float rotx, float &roty, float &rotz, float &fov)
{
	char eyeInfoFile[fileLength] = "eyeInfo.log";
	foldCat(eyeInfoFile);
	std::ifstream camInput(eyeInfoFile);
	camInput >> locx >> locy >> locz;
	camInput >> rotx >> roty >> rotz;
	camInput >> fov;
	camInput.close();
}


void readCamInfo(Vector3 &loc, Vector3 &rot, float &fov)
{
	char eyeInfoFile[fileLength] = "eyeInfo.log";
	foldCat(eyeInfoFile);
	log_to_file(eyeInfoFile);
	std::ifstream camInput(eyeInfoFile);
	camInput >> loc.x >> loc.y >> loc.z;
	camInput >> rot.x >> rot.y >> rot.z;
	camInput >> fov;
	camInput.close();
}

void writeAreaInfo(const int &n)
{
	for (int i = 0; i < 4; i++) {
		char areaInfoFile[fileLength] = "areaInfo.log";
		foldCat(areaInfoFile, nowFolds[i]);
		log_to_file(areaInfoFile);
		std::ofstream info(areaInfoFile);
		info << n << std::endl;
		info.close();
	}
}

void writeAreaInfo(const Vector3 &loc)
{
	for (int i = 0; i < 4; i++) {
		char areaInfoFile[fileLength] = "areaInfo.log";
		foldCat(areaInfoFile, nowFolds[i]);
		std::ofstream info(areaInfoFile, std::ios_base::app);
		info << loc.x << ' ' << loc.y << std::endl;
		info.close();
	}
}

void writeZheight(float z)
{
	for (int i = 0; i < 4; i++) {
		char areaInfoFile[fileLength] = "Zheight.log";
		foldCat(areaInfoFile, nowFolds[i]);
		std::ofstream info(areaInfoFile);
		info << z << std::endl;
		info.close();
	}
}


void readAreaInfo(int &n, std::vector<pedLocation> &pedLocations)
{
	char areaInfoFile[fileLength] = "areaInfo.log";
	foldCat(areaInfoFile);
	std::ifstream info(areaInfoFile);
	pedLocation po;
	info >> n;
	for (int i = 0; i < n; i++) {
		info >> po.x >> po.y;
		pedLocations.push_back(po);
	}
	info.close();
}

float readZheight()
{
	float z;
	char areaInfoFile[fileLength] = "Zheight.log";
	foldCat(areaInfoFile);
	std::ifstream info(areaInfoFile);
	info >> z;
	info.close();

	return z;
}

void readAreaBorder(float &mix, float &mxx, float &miy, float &mxy)
{
	char areaInfoFile[fileLength] = "areaInfo.log";
	foldCat(areaInfoFile);
	log_to_file(areaInfoFile);
	std::ifstream circle(areaInfoFile);
	int n; float x, y;
	bool first = true;
	while (circle >> n) {
		while (n--) {
			circle >> x >> y;
			if (first) {
				mix = mxx = x, miy = mxy = y;
				first = false;
			}
			else {
				mix = min(x, mix), mxx = max(x, mxx);
				miy = min(y, miy), mxy = max(y, mxy);
			}
		}
	}
	circle.close();
}

bool fileExist(char *path)
{
	FILE* fh = fopen(path, "r");
	return fh != NULL;
}

bool fileExist(WCHAR *path)
{
	char cpath[fileLength];
	sprintf(cpath, "%ws", path);
	return fileExist(cpath);
}

void writeLeveFile(int level)
{
	for (int i = 0; i < 4; i++) {
		char levelFile[fileLength] = "levelInfo.log";
		foldCat(levelFile, nowFolds[i]);
		std::ofstream levelInfo(levelFile);
		levelInfo << level;
		levelInfo.close();
	}
}

int readLevelFile()
{
	char levelfile[fileLength] = "levelInfo.log";
	foldCat(levelfile);
	std::ifstream info(levelfile);
	int levNo;
	info >> levNo;
	info.close();
	return level[levNo].maxNum;
}

int readImgNum()
{
	int imgNum = 5;
	char* imgNumFile = "imageNum.txt";
	if(fileExist(imgNumFile)) {
		std::ifstream imgf(imgNumFile);
		imgf >> imgNum;
		imgf.close();
	}
	return imgNum;
}
