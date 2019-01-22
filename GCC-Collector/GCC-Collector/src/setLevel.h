#pragma once
#include <string>
#include "keyboard.h"
extern bool setLevelMode;

struct levelStruct {
	static int now;
	int lev, maxNum;
	std::string levShow;
	keyInfo *setKey;

	levelStruct();
	levelStruct(int _lev, int _maxNum, std::string _levShow);
};

const levelStruct level[] = {
	levelStruct(), // no use
	levelStruct(1, 10, "1-(1:10)"),
	levelStruct(2, 20, "2-(1:20)"),
	levelStruct(3, 50, "3-(1:50)"),
	levelStruct(4, 100, "4-(1:100)"),
	levelStruct(5, 300, "5-(1:300)"),
	levelStruct(6, 600, "6-(1:600)"),
	levelStruct(7, 1000, "7-(1:1000)"),
	levelStruct(8, 2000, "8-(1:2000)"),
	levelStruct(9, 4000, "9-(1:4000)")
};

const int levelNum = int(sizeof(level) / sizeof(levelStruct));

void createLevelSaveFile();
void setSceneLevel();
void stopSetLevel();
