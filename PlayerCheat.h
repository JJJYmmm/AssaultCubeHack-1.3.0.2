#pragma once
#include <Windows.h>
#include "offset.h"

extern bool modeESP;
extern bool modeAutoAim;

struct Player {
	char unknown1[4];
	float x;
	float y;
	float z;
	char unknown2[0x24];
	float yaw;
	float pitch;
	char unknown3[0x1C9];
	char name[16];
	char unknown4[0xF7];
	int team;
	char unknown5[0x8];
	int dead;
};

struct ViewMatrix {
	// ”Ω«æÿ’Û     X,   Y,   Z,   W
	float m11, m12, m13, m14; //00, 01, 02, 03
	float m21, m22, m23, m24; //04, 05, 06, 07
	float m31, m32, m33, m34; //08, 09, 10, 11
	float m41, m42, m43, m44; //12, 13, 14, 15
};

void Draw();

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void InitWindow();

void DeleteWindow();

void InitPlayer();
float euclidean_distance(float, float);
bool Seen(Player* target);
void ESP();
void AutoAim();
void UpdateAim();

void Menu();