#pragma once
#include "types.h"
#include "natives.h"
#include "defineArea.h"
#include <string>
#include <vector>

#define console_log log_to_file

const size_t fileLength(50);
const int partLength = 1000;

void set_status_text(std::string text);

void log_to_file(std::string msg);
void log_to_pedTxt(std::string message, char *fileName);

const unsigned long dirMark = 16;
bool initDataDir();
bool InitPartNo();

int defaultFold();
void markAddOneImage();
void createNewFold();
void changeFoldNo(int No);
void foldCat(char *subString);
void foldCat(WCHAR *substring);

void writeCamInfo(const Vector3 &loc, const Vector3 &rot, const float &fov, int foldNo);
void writeCamInfo(const Vector3 &loc, const Vector3 &rot, const float &fov);
void wriet4Camera();
void readCamInfo(float &locx, float &locy, float &locz, float rotx, float &roty, float &rotz, float &fov);
void readCamInfo(Vector3 &loc, Vector3 &rot, float &fov);

void writeAreaInfo(const Vector3 &loc);
void writeAreaInfo(const int &n);
void writeZheight(float z);
float readZheight();
void readAreaInfo(int &n, std::vector<struct pedLocation> &pedLocations);
void readAreaBorder(float &mix, float &mxx, float &miy, float &mxy);

bool fileExist(WCHAR *path);
bool fileExist(char *path);

void writeLeveFile(int level = 1);
int readLevelFile();

int readImgNum();