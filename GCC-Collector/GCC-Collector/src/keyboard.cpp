#include "keyboard.h"
#include "types.h"
#include "script.h"
#include "main.h"
#include "camera.h"
#include "defineArea.h"
#include "infoIO.h"
#include "setLevel.h"

const int keyInfo::MAX_DOWN = 500; //ms

keyInfo::keyInfo() {
	time = 0;
}
bool keyInfo::isKeyDown() {
	return ((GetTickCount() < time + MAX_DOWN) && !isUpNow);
}
void keyInfo::pushDown(BOOL _isUpNow, BOOL _isWithAlt, BOOL _wasDownBefore) {
	time = GetTickCount();
	isWithAlt = _isWithAlt;
	wasDownBefore = _wasDownBefore;
	isUpNow = _isUpNow;
}
keyInfo W, A, S, D, V, shift, ctrl, tab, oemPlus, oemMinus, F10, I, F11, numKey[10];

void OnKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
	bool updown = !wasDownBefore && !isUpNow;
	auto push = [&key, &updown](char mykey) {
		return key == mykey && updown;
	};

	if (scriptStatus == scriptStop && push(VK_F9)) {
		scriptStatus = scriptReady;
	}
	if (scriptStatus == scriptReadyCamera && push(VK_F10)) {
		scriptStatus = cameraMode;
	}
	if (scriptStatus == scriptReadyDefineArea && push(VK_F11)) {
		scriptStatus = defineArea;
	}
	if (scriptStatus == scriptReadySetLevel && push(VK_F12)) {
		scriptStatus = setLevel;
	}
	if (scriptStatus == cameraMode && CameraMode == true) {
		if (key == 'W') W.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == 'A') A.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == 'S') S.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == 'D') D.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == 'V') V.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == VK_OEM_PLUS) oemPlus.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == VK_OEM_MINUS) oemMinus.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == VK_SHIFT) shift.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == VK_CONTROL) ctrl.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (push(VK_F10)) scriptStatus = cameraModeEnd;
		if (key == VK_TAB) tab.pushDown(isUpNow, isWithAlt, wasDownBefore);
	}
	if (scriptStatus == defineArea && defineAreaMode == true) {
		if (key == VK_F10) F10.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (push(VK_F11)) F11.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == 'I') I.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == VK_TAB) tab.pushDown(isUpNow, isWithAlt, wasDownBefore);
	}
	if (scriptStatus == defineAreaEnd && CameraMode == true) {
		if (key == VK_CONTROL) ctrl.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == 'W') W.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == 'A') A.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == 'S') S.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == 'D') D.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == VK_OEM_PLUS) oemPlus.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == VK_OEM_MINUS) oemMinus.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (key == VK_SHIFT) shift.pushDown(isUpNow, isWithAlt, wasDownBefore);
		if (push(VK_F11)) F11.pushDown(isUpNow, isWithAlt, wasDownBefore);
	}
	if (scriptStatus == setLevel && setLevelMode == true) {
		for (char i = '0'; i <= '9'; i++) {
			if (key == i) {
				numKey[i - '0'].pushDown(isUpNow, isWithAlt, wasDownBefore);
				break;
			}
		}
		if (key == VK_F12) {
			scriptStatus = setLevelEnd;
		}
	}
	if(push('L')) {
		scriptStatus = scriptStart;
	}
	if (push(VK_F5)) {
		scriptStatus = scriptStop;
	}
}
