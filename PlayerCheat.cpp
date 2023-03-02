#include <Windows.h>
#include <math.h>
#include "PlayerCheat.h"
#define M_PI 3.14159265358979323846
#define MAX_PLAYERS 100

DWORD* player_base_ptr;
DWORD* player_list;
int* player_num;

Player* player;
ViewMatrix* view;

float closest_dis = -1.0f;
float closest_yaw = -1.0f;
float closest_pitch = -1.0f;

float val_xhead[MAX_PLAYERS];
float val_yhead[MAX_PLAYERS];
float val_xfoot[MAX_PLAYERS];
float val_yfoot[MAX_PLAYERS];
bool is_enemy[MAX_PLAYERS];
char name[MAX_PLAYERS][16];

//屏幕大小
int GameWidth;
int GameHeight;

HWND Window;
HWND NewWindow;
RECT rc;
HDC hDC;
HDC memDC;
HFONT font;
const COLORREF rgbRed = 0x000000FF;
const COLORREF rgbGreen = 0x0000FF00;

bool modeESP = false;
bool modeAutoAim = false;

void InitPlayer() {
	//player
	player_base_ptr = (DWORD *)offset_Player_base_ptr;
	player = (Player*)(*player_base_ptr);
	player_list = (DWORD*)offset_Player_list;
	player_num = (int *)offset_Player_num;
	//viewmatrix
	view = (ViewMatrix*)offset_viewMatrix;
	//size
	GameWidth = *(int*)offset_GameWidth;
	GameHeight = *(int*)offset_GameHeight;
}

// Function to calculate the euclidean distance between two points
float euclidean_distance(float x, float y) {
	return sqrtf((x * x) + (y * y));
}

void ESP() {
	// 遍历所有玩家 注意i从1开始
	memset(val_xhead, 0, sizeof val_xhead);
	memset(val_yhead, 0, sizeof val_yhead);
	memset(val_xfoot, 0, sizeof val_xfoot);
	memset(val_yfoot, 0, sizeof val_yfoot);
	memset(is_enemy, false, sizeof is_enemy);
	memset(name, 0, sizeof name);
	for (int i = 1; i < *player_num; i++) {
		DWORD* target_offset = (DWORD*)(*player_list + (i * 4));
		Player* target = (Player*)(*target_offset);
		float dis = -1.0f;
		// 保证敌人存活
		if (player != NULL && target != NULL && view != NULL && !target->dead && Seen(target)) {
			//头部坐标target->x,y,z
			//得到相机xyw
			float screenX = (view->m11 * target->x) + (view->m21 * target->y) + (view->m31 * target->z) + view->m41;
			float screenY = (view->m12 * target->x) + (view->m22 * target->y) + (view->m32 * target->z) + view->m42;
			float screenW = (view->m14 * target->x) + (view->m24 * target->y) + (view->m34 * target->z) + view->m44;

			//准心在屏幕的位置
			float camX = GameWidth / 2.0f;
			float camY = GameHeight / 2.0f;

			//转换成敌人在屏幕的坐标
			float xhead = camX + (camX * screenX / screenW);
			float yhead = camY - (camY * screenY / screenW);

			//脚部坐标target->x,y,z-4.5
			//得到相机xyw
			screenX = (view->m11 * target->x) + (view->m21 * target->y) + (view->m31 * (target->z - 4.5)) + view->m41;
			screenY = (view->m12 * target->x) + (view->m22 * target->y) + (view->m32 * (target->z - 4.5)) + view->m42;
			screenW = (view->m14 * target->x) + (view->m24 * target->y) + (view->m34 * (target->z - 4.5)) + view->m44;

			//转换成敌人在屏幕的坐标
			float xfoot = camX + (camX * screenX / screenW);
			float yfoot = camY - (camY * screenY / screenW);

			strcpy_s(name[i], target->name);
			val_xhead[i] = xhead;
			val_yhead[i] = yhead;
			val_xfoot[i] = xfoot;
			val_yfoot[i] = yfoot;
			is_enemy[i] = target->team == player->team ? false : true;
		}
	}
	UpdateWindow(NewWindow);
}


bool Seen(Player* target) {
	//得到相机xyw
	float screenX = (view->m11 * target->x) + (view->m21 * target->y) + (view->m31 * target->z) + view->m41;
	float screenY = (view->m12 * target->x) + (view->m22 * target->y) + (view->m32 * target->z) + view->m42;
	float screenW = (view->m14 * target->x) + (view->m24 * target->y) + (view->m34 * target->z) + view->m44;
	if (screenW <= 0.001f) {
		return false;
	}
	//准心在屏幕的位置
	float camX = GameWidth / 2.0f;
	float camY = GameHeight / 2.0f;

	//转换成敌人在屏幕的坐标
	float deltax = (camX * screenX / screenW);
	float deltay = (camY * screenY / screenW);
	closest_dis = euclidean_distance(deltax, deltay);
	return true;
}


void AutoAim() {
	//初始化
	closest_yaw = 0.0f;
	closest_pitch = 0.0f;

	//敌人距离准星的距离
	closest_dis = -1.0f;
	float current_dis = -1.0f;

	// 遍历所有玩家 找到距离屏幕准星最近的敌人 注意i从1开始
	for (int i = 1; i < *player_num; i++) {
		DWORD* target_offset = (DWORD*)(*player_list + (i * 4));
		Player* target = (Player*)(*target_offset);

		// 保证敌人存活且在视角范围内
		if (player != NULL && target != NULL && player->team != target->team && !target->dead && view != NULL
			&& Seen(target)) {

			float abspos_x = target->x - player->x;
			float abspos_y = target->y - player->y;
			float abspos_z = target->z + 0.3 - player->z;	//加0.3是为了更好瞄准头部
			float Horizontal_distance = euclidean_distance(abspos_x, abspos_y);	//2D平面距离
			// 如果当前敌人离准星最近，计算偏航角和偏仰角
			if (current_dis == -1.0f || closest_dis < current_dis) {
				current_dis = closest_dis;
				// 计算yaw
				float azimuth_xy = atan2f(abspos_y, abspos_x);
				// 转换成角度制
				float yaw = (float)(azimuth_xy * (180.0 / M_PI));
				// 需要加90度 因为玩家的起始yaw就是90°
				closest_yaw = yaw + 90;

				// 计算pitch
				float azimuth_z = atan2f(abspos_z, Horizontal_distance);
				// 转换成角度制
				closest_pitch = (float)(azimuth_z * (180.0 / M_PI));
			}
		}
	}

	// 将准星移到最近的敌人处
	if (current_dis != -1.0f) {
		UpdateAim();
	}
}

void UpdateAim() {

	player->yaw = closest_yaw;
	player->pitch = closest_pitch;
}


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_PAINT:
		Draw();
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

void InitWindow() {
	Window = FindWindowA("SDL_app", "AssaultCube");
	GetClientRect(Window, &rc);
	WNDCLASSEX wincl;
	wincl.cbSize = sizeof(WNDCLASSEX);
	wincl.style = CS_HREDRAW | CS_VREDRAW;
	wincl.hInstance = 0;
	wincl.lpszClassName = (L"lookaway");
	wincl.lpfnWndProc = WindowProcedure;
	wincl.style = CS_DBLCLKS;
	wincl.hIcon = NULL;
	wincl.hIconSm = NULL;
	wincl.hCursor = NULL;
	wincl.lpszMenuName = NULL;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	RegisterClassEx(&wincl);
	POINT point;
	point.x = rc.left;
	point.y = rc.top;
	ClientToScreen(Window, &point);
	NewWindow = CreateWindowEx(
		(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED),
		(L"lookaway"),
		(L"lookaway"),
		(WS_POPUP | WS_CLIPCHILDREN),
		point.x,
		point.y,
		GameWidth,
		GameHeight,
		0, 0, wincl.hInstance, 0
	);
	hDC = GetDC(NewWindow);
	COLORREF bkcolor = GetBkColor(hDC);
	SetLayeredWindowAttributes(NewWindow, bkcolor, 0, LWA_COLORKEY);
	ShowWindow(NewWindow, SW_SHOWNORMAL);
	UpdateWindow(NewWindow);
	memDC = CreateCompatibleDC(0);
	SetTextAlign(memDC, TA_CENTER | TA_NOUPDATECP);
	SetBkColor(memDC, RGB(0, 0, 0));
	SetBkMode(memDC, TRANSPARENT);
}

void Draw()
{
	HBITMAP hbitmap = CreateCompatibleBitmap(hDC, GameWidth, GameHeight);
	HGDIOBJ oldbitmap = SelectObject(memDC, hbitmap);
	BitBlt(memDC, 0, 0, GameWidth, GameHeight, 0, 0, 0, WHITENESS);
	for (int i = 1; i < *player_num; i++) {
		int offset = (val_yfoot[i] - val_yhead[i]) / 6;
		SelectObject(memDC, GetStockObject(DC_PEN));
		is_enemy[i] ? SetDCPenColor(memDC, rgbRed) : SetDCPenColor(memDC, rgbGreen);
		Rectangle(memDC, val_xhead[i] - 2 * offset, val_yhead[i] - offset, val_xfoot[i] + 2 * offset, val_yfoot[i]);

		is_enemy[i] ? SetTextColor(memDC, rgbRed) : SetTextColor(memDC, rgbGreen);
		SelectObject(memDC, font);
		TextOutA(memDC, val_xhead[i], val_yhead[i], name[i], strlen(name[i]));
	}
	BitBlt(hDC, 0, 0, GameWidth, GameWidth, memDC, 0, 0, SRCCOPY);
	DeleteObject(hbitmap);
}

void DeleteWindow()
{
	DeleteDC(hDC);
	DeleteObject(memDC);
	DestroyWindow(NewWindow);
}

//char STATE[2][4] = { "ON","OFF" };
//void Menu() {
//	HBITMAP hbitmap = CreateCompatibleBitmap(hDC, GameWidth, GameHeight);
//	HGDIOBJ oldbitmap = SelectObject(memDC, hbitmap);
//	BitBlt(memDC, 0, 0, GameWidth, GameHeight, 0, 0, 0, WHITENESS);
//
//	char strESP[] = "ESP Func is";
//	modeESP ? (strcat_s(strESP,STATE[0]),SetTextColor(memDC, rgbGreen)) : (strcat_s(strESP, STATE[1]),SetTextColor(memDC, rgbRed));
//	SelectObject(memDC, font);
//	TextOutA(memDC, 10,10, strESP, strlen(strESP));
//
//	char strAIM[] = "AutoAim Func is";
//	modeAutoAim ? (strcat_s(strAIM, STATE[0]), SetTextColor(memDC, rgbGreen)) : (strcat_s(strAIM, STATE[1]), SetTextColor(memDC, rgbRed));
//	SelectObject(memDC, font);
//	TextOutA(memDC, 10, 30, strAIM, strlen(strAIM));
//
//	BitBlt(hDC, 0, 0, GameWidth, GameWidth, memDC, 0, 0, SRCCOPY);
//	DeleteObject(hbitmap);
//}