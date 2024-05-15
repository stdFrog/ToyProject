#ifndef __CONTROL_H_
#define __CONTROL_H_

#include <windows.h>

#define GBS_PUSH 0
#define GBS_CHECK 1
#define GBN_CLICKED 0

typedef enum { GB_NORMAL, GB_HOT, GB_PRESSED, GB_DISABLE, GB_HIDDEN } STATE;

class Button {
	int _x,_y,_Width,_Height;
	DWORD _Style;
	UINT _ID;
	HWND _hParent;
	STATE _State;
	BOOL _bCapture;
	BOOL _bTimer;

private:
	void ChangeState(STATE State);
	void DrawBitmap(HDC hdc);
	BOOL IsPtOnMe(POINT pt);
	BOOL IsPtOnMe(int x,int y);

public:
	void ChangeParent(HWND hParent) { _hParent = hParent; }
	void SetImage(WORD Normal,WORD Hot,WORD Down,WORD Disable);
	void SetState(eState State);

public:
	void OnDown(LPARAM lParam);
	void OnUp(LPARAM lParam);
	void OnMove(LPARAM lParam);
	void OnPaint(HDC hdc);
	void OnTimer();

public:
	void Enable(BOOL bEnable);
	void Show(BOOL bShow);

public:
	int GetX() { return x; }
	int GetY() { return y; }
	int GetWidth() { return w; }
	int GetHeight() { return h; }
	STATE GetState() { return State; }
	BOOL IsShow() { return (_State != GB_HIDDEN); }
	BOOL IsEnabled() { return (_State != GB_DISABLE); }

public:
	Button(DWORD Style, int x, int y, int Width, int Height, UINT ID, HWND hParent);
	~Button();
};

#endif
