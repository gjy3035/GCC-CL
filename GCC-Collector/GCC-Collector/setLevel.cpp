#include "setLevel.h"
#include "infoIO.h"
#include "keyboard.h"
#include <string>

bool setLevelMode = false;
int levelStruct::now = 0;

void showLevel()
{
	for (int i = 1; i < levelNum; i++) {
		set_status_text("level " + std::to_string(level[i].lev) + " : " + level[i].levShow);
	}
}

void createLevelSaveFile()
{
	levelStruct::now = 0;
	set_status_text("select scene level...");
	showLevel();
	set_status_text("press down number key 1 to " + std::to_string(levelNum) + " to select one");
	set_status_text("press down '0' to show level table again.");
}

void setSceneLevel()
{
	if (numKey[0].isKeyDown()) {
		showLevel();
	}
	for (int i = 1; i < levelNum; i++) {
		if (numKey[i].isKeyDown()) {
			levelStruct::now = i;
			set_status_text("now the level is " + level[i].levShow);
			set_status_text("press F12 to save, or press another number for another level");
			break;
		}
	}
}

void stopSetLevel()
{
	writeLeveFile(levelStruct::now);
	setLevelMode = false;
}

levelStruct::levelStruct() {
	lev = 0, maxNum = 0, levShow = "", setKey = NULL;
}

levelStruct::levelStruct(int _lev, int _maxNum, std::string _levShow)
	:lev(_lev), maxNum(_maxNum), levShow(_levShow)
{
	setKey = lev < 10 ? numKey + lev : NULL;
}