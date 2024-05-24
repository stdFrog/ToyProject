#ifndef __BUTTON_H_
#define __BUTTON_H_

typedef enum { PUSHBUTTON, CHECKBUTTON, RADIOBUTTON, BUTTON_STYLE_LAST_COUNT } BUTTON_STYLE;
typedef enum {NORMAL, HOT, PRESSED, RELEASE, DISABLE, HIDDEN, STATE_LAST_COUNT} STATE;

class Button{
private:
	LONG _X, _Y, _Width, _Height;
	UINT _ID;
	DWORD _Style;
	HWND _hParent;

private:
	STATE _State;
	BOOL _bCapture;
	BOOL _bTimer;
	HBITMAP _hBitmap[STATE_LAST_COUNT];

private:
	void DrawBitmap(HDC hDC);
	BOOL IsPtOnMe(POINT pt);
	BOOL IsPtOnMe(LONG x, LONG y);

public:
	void SetState(STATE NewState);
	STATE GetState() {return _State;}

public:
	LONG GetX() {return _X;}
	LONG GetY() {return _Y;}
	LONG GetWidth() {return _Width;}
	LONG GetHeight() {return _Height;}

public:
	UINT GetID() {return _ID;}
	DWORD GetStyle() {return _Style;}

public:
	void OnMove(LPARAM);
	void OnPressed(LPARAM);
	void OnReleased(LPARAM);

	void OnPaint(HDC);
	void OnTimer();

public:
	Button(DWORD Style, LONG x, LONG y, LONG w, LONG h, HWND hParent, UINT ID);
	~Button();
};

#endif
