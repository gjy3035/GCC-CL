#pragma once

#include <string>

struct Animation
{
	int shortcutIndex;
	std::string strShortcutIndex;
	char* animLibrary;
	char* animName;
	int duration;

	std::string toString() {
		return strShortcutIndex + " " + std::string(animLibrary) + " " + std::string(animName) + " " + std::to_string(duration);
	}
};

bool initAnimations(std::string fileName);

Animation randomAnimation();
int animNum();

Animation getAnim(int id);