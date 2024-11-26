#ifndef __BUTTON_H_
#define __BUTTON_H_
#define IDW_BUTTONCNT	0

typedef enum { PUSH, CHECK, RADIO } TYPE;
typedef enum { CIRCLE, TRIANGLE, RECTANGLE } SHAPE;
typedef enum { NORMAL, PRESS, PRESSING, BLOCK, HOT, STATE_LAST_COUNT } STATE;
typedef enum { EMPTY, MINE, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, DATA_LAST_COUNT } DATA;

class Button {
	static UINT ButtonCount;

private:
	TYPE _Type;
	LONG _x, _y, _Width, _Height;
	UINT _ID;
	HWND _hParent;
	STATE _State;
	SHAPE _Shape;
	BOOL _bCapture, _bTimer, _bLeft;

public:
	DATA _Data;
	HBITMAP _hBitmapData[DATA_LAST_COUNT];
	HBITMAP _hBitmapState[STATE_LAST_COUNT];
	VOID DeleteBitmap(){
		for(int i=0; i<DATA_LAST_COUNT; i++){
			if(_hBitmapData[i]){
				DeleteObject(_hBitmapData[i]);
			}
		}

		for(int j=0; j<STATE_LAST_COUNT; j++){
			if(_hBitmapState[j]){
				DeleteObject(_hBitmapState[j]);
			}
		}
	}

public:
	VOID DrawBitmap(HDC);
	BOOL IsPtOnMe(POINT);
	BOOL IsPtOnMe(LONG, LONG);

public:
	VOID OnPaint(HDC);
	VOID OnLPressed();
	VOID OnRPressed();
	VOID OnReleased();

public:
	VOID ChangeState(STATE CurrentState) { _State = CurrentState; DrawBitmap(NULL); }

public:
	VOID SetID(UINT NewID) { _ID = NewID; }
	VOID SetType(TYPE NewType) { _Type = NewType; }
	VOID SetState(STATE NewState) { _State = NewState; }
	VOID SetShape(SHAPE NewShape) { _Shape = NewShape; }
	VOID SetParent(HWND hNewParent) { _hParent = hNewParent; }
	VOID SetBitmap(HBITMAP* hBitmapData, HBITMAP* hBitmapState) {
		for(int i=0; i<DATA_LAST_COUNT; i++){
			if(hBitmapData[i]){
				_hBitmapData[i] = hBitmapData[i];
			}
		}

		for(int j=0; j<STATE_LAST_COUNT; j++){
			if(hBitmapState[j]){ 
				_hBitmapState[j] = hBitmapState[j];
			}
		}
	}

public:
	VOID SetX(LONG x) { _x = x; }
	VOID SetY(LONG y) { _y = y; }
	VOID SetWidth(LONG Width) { _Width = Width; }
	VOID SetHeight(LONG Height) { _Height = Height; }
	VOID SetData(DATA Data) { _Data = Data; }

public:
	UINT GetID() { return _ID; }
	TYPE GetType() { return _Type; }
	HWND GetParent() { return _hParent; }
	DATA GetData() { return _Data; }
	STATE GetState() { return _State; }
	SHAPE GetShape() { return _Shape; }
	BOOL IsCapture() { return _bCapture;}

public:
	LONG GetX() { return _x; }
	LONG GetY() { return _y; }
	LONG GetWidth() { return _Width; }
	LONG GetHeight() { return _Height; }

public:
	Button(TYPE Type = PUSH, LONG x = 0, LONG y = 0, LONG Width = 16, LONG Height = 16, UINT ID = (UINT)(IDW_BUTTONCNT + ButtonCount), HWND hParent = NULL)
		: _Type(Type), _x(x), _y(y), _Width(Width), _Height(Height), _ID(ID), _hParent(hParent), _bCapture(FALSE), _bLeft(FALSE)
	{
		ButtonCount++;
		_Data = EMPTY;
		_State = NORMAL;
		_Shape = RECTANGLE;
	}

	~Button(){
		DeleteBitmap();
		--ButtonCount;
	}
};

#endif
