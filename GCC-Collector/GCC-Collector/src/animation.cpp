#include "animation.h"
#include <string>
#include <vector>
#include <fstream>
#include <random>
#include "natives.h"

std::vector<Animation> gtaAnimations;

bool initAnimations(std::string fileName) {
	gtaAnimations.reserve(97101 + 10);


	std::ifstream animationsFile;
	animationsFile.open(fileName);

	if (!animationsFile) {
		return false;
	}
	int index = 1;
	std::string strShortcutIndex;
	std::string animLibrary;
	std::string animName;
	int duration;

	std::ofstream ss("exist.txt", std::ios_base::app);
	while (animationsFile >> strShortcutIndex >> animLibrary >> animName >> duration)
	{
		gtaAnimations.push_back({ index,strShortcutIndex,_strdup(animLibrary.c_str()), _strdup(animName.c_str()),duration });
		index++;
		bool ex = STREAMING::DOES_ANIM_DICT_EXIST(_strdup(animLibrary.c_str()));
		ss << ex << std::endl;
	}
	ss.close();

	return true;
}

Animation randomAnimation()
{
	int l = gtaAnimations.size();
	return gtaAnimations[rand() % l];
}

int animNum()
{
	return gtaAnimations.size();
}

Animation getAnim(int id)
{
	return gtaAnimations[id];
}