#ifndef __RESOURCE_H_
#define __RESOURCE_H_

#include <windows.h>

class Control{
	static UINT LocalID;

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
	Control(LONG _x, LONG _y, LONG _Width, LONG _Height, UINT _ID = LocalID, HWND _hParent = NULL);
	virtual ~Control();
};

class Button : public Control {
	/* 
		TODO : Interface - Recv Message for WndProc
		TODO : Interface - Set Info, Get Info, action

		TODO : Image { NORMAL, HOT, PRESSED, RELEASED } 
								OR
		TODO : 3D Button : intaglio/shaded
	*/

public:
	Button();
	~Button();
}

#endif
