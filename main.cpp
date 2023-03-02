#include <Windows.h>
#include<stdio.h>
#include "PlayerCheat.h"
void check()
{
	while (1) {
		//Menu();
		if (GetAsyncKeyState(VK_F1) & 1) {
			modeESP = !modeESP;
			if (modeESP == true) {
				InitWindow();
			}
			else{
				DeleteWindow();
			}
		}
		if (modeESP) 
			ESP();
		if (GetAsyncKeyState(VK_F2) & 1) {
			modeAutoAim = !modeAutoAim;
		}
		if (modeAutoAim)
			AutoAim();
		Sleep(1);
	}
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		InitPlayer();
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)check, NULL, 0, NULL);
	}
	return true;
}