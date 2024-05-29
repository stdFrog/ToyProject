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

#define ID_SYS_ABOUT	40000
#define ID_SYS_NEWGAME	40001
#define ID_SYS_RESIZE1	40002
#define ID_SYS_RESIZE2	40003
#define ID_SYS_RESIZE3	40004

#define IDW_STATUS		10000

/* Msg Map */
typedef struct tag_MSGMAP{
	UINT iMessage;
	LRESULT (*lpfnWndProc)(HWND, WPARAM, LPARAM);
}MSGMAP;

/* Procedure */
LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnRButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnRButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnExitSizeMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnInitMenu(HWND hWnd, WPARAM wParam, LPARAM lParam);

/* Utility */
void DrawBitmap(HDC, LONG, LONG, HBITMAP);
void SetClientRect(HWND hWnd, int Width, int Height);
void SetStatusText(HWND hWnd);

/* Game Settings */
class Button;
Button** CreateButton(int w, int h);
void DestroyButton(Button** Btns);
BOOL InitializeButton(HWND hWnd, Button** Btns);

/* OnFunction */
void DisplayButtons(HDC, Button**);

#endif
