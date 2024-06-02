#ifndef __BUTTON_H_
#define __BUTTON_H_

/*
	확장을 고려하여 여러 스타일의 열거형 타입을 만들어뒀으나 굳이 이럴 필요 없으므로 추후 수정하기로 한다.
	또한, 멤버 변수와 함수, 초기화 방식도 수정이 필요하다.
*/

#define IDW_BUTTON 0

typedef enum { PUSH = 0x1, CHECK = 0x2, RADIO = 0x4 } TYPE;
typedef enum { NORMAL = 0x1, PRESSED = 0x2, BLOCK = 0x4 } STATE;
typedef enum { CIRCLE = 0x1, TRIANGLE = 0x2, RECTANGLE = 0x4 } SHAPE;

class Button {
	static UINT ButtonCount;

private:
	TYPE _Type;
	LONG _x, _y, _Width, _Height;
	UINT _ID;
	HWND _hParent;
	STATE _State;
	SHAPE _Shape;
	BOOL _bCapture;
	HBITMAP _hBitmap;

public:
	VOID DisplayState();

public:
	VOID DrawBitmap(HDC);
	BOOL IsPtOnMe(POINT);
	BOOL IsPtOnMe(LONG, LONG);

public:
	VOID OnPaint(HDC);
	VOID OnPressed(LPARAM, BOOL bLeft = TRUE);
	VOID OnReleased(LPARAM, BOOL bLeft = TRUE);

public:
	VOID ChangeParent(HWND hNewParent) { _hParent = hNewParent; }
	VOID ChangeState(STATE CurrentState) { _State = CurrentState; DrawBitmap(NULL); }
public:
	VOID SetID(UINT NewID) { _ID = NewID; }
	VOID SetType(TYPE NewType) { _Type = NewType; }
	VOID SetState(STATE NewState) { _State = NewState; }
	VOID SetShape(SHAPE NewShape) { _Shape = NewShape; }

public:
	VOID SetX(LONG x) { _x = x; }
	VOID SetY(LONG y) { _y = y; }
	VOID SetWidth(LONG Width) { _Width = Width; }
	VOID SetHeight(LONG Height) { _Height = Height; }

public:
	UINT GetID() { return _ID; }
	TYPE GetType() { return _Type; }
	HWND GetParent() { return _hParent; }
	STATE GetState() { return _State; }
	SHAPE GetShape() { return _Shape; }

public:
	LONG GetX() { return _x; }
	LONG GetY() { return _y; }
	LONG GetWidth() { return _Width; }
	LONG GetHeight() { return _Height; }

public:
	Button(TYPE Type = PUSH, LONG x = 0, LONG y = 0, LONG Width = 0, LONG Height = 0, UINT ID = (UINT)(IDW_BUTTON + Button::ButtonCount), HWND hParent = NULL)
		: _Type(Type), _x(x), _y(y), _Width(Width), _Height(Height), _ID(ID), _hParent(hParent), _Color(), _bCapture(FALSE), _hBitmap(NULL)
	{
		ButtonCount++;
		_State = NORMAL;
		_Shape = RECTANGLE;
	}

	~Button(){
		ButtonCount--;

		if(_hBitmap != NULL) { DeleteObject(_hBitmap); }
	}
};

#endif
