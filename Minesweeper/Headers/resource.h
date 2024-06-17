#ifndef __RESOURCE_H_
#define __RESOURCE_H_

#define UNICODE
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"

#define CLASS_NAME TEXT("Minesweeper")

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_BUILD 1
#define VERSION_PATCH 1

#define STR(str) #str
#define TOVERSION(major, minor, build, patch) STR(major.minor.build.patch)
#define FULLVERSION TOVERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, VERSION_PATCH)

#define IDW_STATUS		10000

#define ID_SYS_ABOUT	40000
#define ID_SYS_NEWGAME	40001
#define ID_SYS_RESIZE1	40002
#define ID_SYS_RESIZE2	40003
#define ID_SYS_RESIZE3	40004

#define IDB_NORMAL		50000
#define IDB_PRESS		50001
#define IDB_PRESSING	50002
#define IDB_BLOCK		50003
#define IDB_HOT			50004

#define IDB_EMPTY		60000
#define IDB_MINE		60001
#define IDB_ONE			60002
#define IDB_TWO			60003
#define IDB_THREE		60004
#define IDB_FOUR		60005
#define IDB_FIVE		60006
#define IDB_SIX			60007
#define IDB_SEVEN		60008
#define IDB_EIGHT		60009

typedef struct tag_MSGMAP{
	UINT iMessage;
	LRESULT (*lpfnWndProc)(HWND, WPARAM, LPARAM);
}MSGMAP;

LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnRButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

void DrawBitmap(HDC, LONG, LONG, HBITMAP);
void SetClientRect(HWND hWnd, int Width, int Height);
void SetStatusText(HWND hWnd);

class Button;
Button** CreateButtons(int W, int H);
void InitButtons(HWND hWnd, Button** Btns, int W, int H);
void DestroyButtons(Button** Target, int W, int H);
void OnDrawButtons(HDC hdc, Button** Btns, int W, int H);
void GetIndex(LPARAM, UINT*, UINT*);
void RandomizeSet();
void ExploreSurround();
#endif
