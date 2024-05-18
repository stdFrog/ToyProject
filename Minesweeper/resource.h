#ifndef __RESOURCE_H_
#define __RESOURCE_H_

#define _WIN32_WINNT 0x0A00
#include <windows.h>
#define CLASS_NAME TEXT("Minesweeper")

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_BUILD 1
#define VERSION_PATCH 1

#define STR(str) #str
#define TOVERSION(major, minor, build, patch) STR(major.minor.build.patch)
#define FULLVERSION TOVERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, VERSION_PATCH)

#define IDR_MENU			10001
#define ID_EXIT				10010
#define	ID_DESCRIPTOR		10011

typedef enum {NORMAL, HOT, PRESSED, RELEASED} STATE;

class Control{
	UINT ID;
	LONG x,y,Width,Height;
	HWND hParent;

public:
	UINT GetControlID() const {return ID;}
	HWND GetParent() const {return hParent;}

public:
	LONG GetWidth() const {return Width;}
	LONG GetHeight() const {return Height;}
	POINT GetPosition() const {return POINT{ x, y };}

public:
	VOID SetControlID(UINT _ID) { ID = _ID; }
	VOID SetParent(HWND _hParent) { hParent= _hParent; }

public:
	VOID SetWidth(LONG _Width) { Width = _Width; }
	VOID SetHeight(LONG _Height) { Height = _Height; }
	VOID SetPosition(POINT _Position) { x = _Position.x; y = _Position.y; }

public:
	Control(LONG _x, LONG _y, LONG _Width, LONG _Height, UINT _ID, HWND _hParent = NULL);
	virtual ~Control();
};

class Button : public Control {
	/* 
		TODO : Interface - Recv Message for WndProc
		TODO : Interface - Set Info, Get Info, action

		TODO : Image { NORMAL, HOT, PRESSED, RELEASED } 
								OR
		TODO : 3D Button : intaglio/shaded --------------------------> 16x16 size 음각 공식 만들어서 일반 모드 표현, 눌림 상태는 일반 사각형으로 표현
	*/

	RECT Edge;						// Intaglio / shaded
	STATE State;					// Button State
	HBITMAP hBitmap[4];				// Image(Number, Mine, flag, ...
	/* 이미지가 편할지도?.. */

public:
	Button(LONG _x, LONG _y, LONG _Width, LONG _Height, UINT _ID, HWND _hParent = NULL);
	~Button();
};

typedef struct tag_MSGMAP{
	UINT iMessage;
	LRESULT (*lpfnWndProc)(HWND, WPARAM, LPARAM);
}MSGMAP;

LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnRButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnRButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

void* loadbmp(BITMAPINFOHEADER* ih);
#endif
