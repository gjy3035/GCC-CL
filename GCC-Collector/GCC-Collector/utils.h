/*
	THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2015
*/

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include "types.h"
#include "natives.h"

// returns module load path with trailing slash
std::string GetCurrentModulePath();
std::string roundNumber(float number);

std::string actionInputString(int maxLength);
DWORD actionInputDword();
float actionInputFloat();


class StringUtils {
public:
	static void split(const std::string &s, char delim, std::vector<std::string> &elems);
	static std::vector<std::string> split(const std::string &s, char delim);
};



class MathUtils {
public:
	static Vector3 rotationToDirection(Vector3 rotation);
	static Vector3 crossProduct(Vector3 a, Vector3 b);
};