#ifndef __RESOURCE_H_
#define __RESOURCE_H_

#define UNICODE
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#define CLASS_NAME TEXT("SpyTool")
#define CHILD_CLASS_NAME TEXT("CapturePopup")

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wconversion-null"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_BUILD 1
#define VERSION_PATCH 1

#define STR(str) #str
#define TOVERSION(major, minor, build, patch) STR(major.minor.build.patch)
#define FULLVERSION TOVERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, VERSION_PATCH)

#define IDR_MENU			10001
#define ID_EXIT				10010
#define	ID_ABOUT			10011
#define	ID_CAPTURE			10012

#define	IDW_LISTBOX			20001
#define	IDW_EDIT			20002
#define	IDW_CAPTURE			20003

typedef struct tag_MSGMAP{
	UINT iMessage;
	LRESULT (*lpfnWndProc)(HWND, WPARAM, LPARAM);
}MSGMAP;

LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

LRESULT OnChildCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnChildDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnChildLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnChildLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnChildPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnChildTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK EnumProc(HWND, LPARAM);
BOOL CALLBACK MonitorEnumProc(HMONITOR, HDC, LPRECT, LPARAM); 

void SetClientRect(HWND, int, int);
void SetChildRect(HWND, int, int, int, int);
void DrawBitmap(HDC, LONG, LONG, HBITMAP);
void GetInfo(TCHAR*, TCHAR*);
void InfoFromPoint(HWND, HDC);
int GetCurrentMonitor();

#endif
