/*
	THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
				http://dev-c.com			
			(C) Alexander Blade 2015
*/

/*
	F4					activate
	NUM2/8/4/6			navigate thru the menus and lists (numlock must be on)
	NUM5 				select
	NUM0/BACKSPACE/F4 	back
	NUM9/3 				use vehicle boost when active
	NUM+ 				use vehicle rockets when active
*/

#include "script.h"

void ScriptMain()
{
	while (true) {
		Ped myPed = PLAYER::PLAYER_PED_ID();
		int health = PED::GET_PED_MAX_HEALTH(myPed);
		ENTITY::SET_ENTITY_HEALTH(myPed, health);
		WAIT(0);
	}
}
