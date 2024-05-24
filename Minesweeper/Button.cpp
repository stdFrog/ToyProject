#include <windows.h>
#include "Button.h"

Button::Button(DWORD Style, LONG x, LONG y, LONG w, LONG h, HWND hParent, UINT ID) : _Style(Style), _X(x), _Y(y), _Width(w), _Height(h), _hParent(hParent), _ID(ID), _bCapture(FALSE), _bTimer(FALSE), _State(NORMAL)
{

}

Button::~Button(){

}

void Button::OnPaint(HDC hDC){
	DrawBitmap(hDC);
}

void Button::OnTimer(){
	
}

void Button::OnMove(LPARAM lParam){

}

void Button::OnPressed(LPARAM lParam){
	if(_State == HIDDEN || _State == DISABLE){return;}

	if(IsPtOnMe(LOWORD(lParam), HIWORD(lParam))){
		if(!(_Style & CHECKBUTTON)){

		}
	}else{
		
	}
}

void Button::OnReleased(LPARAM lParam){

}

void Button::SetState(STATE NewState){
	if(_State == NewState) {return;}

	if(_Style & CHECKBUTTON){
		_State = NewState;
		DrawBitmap(NULL);
	}
}

BOOL Button::IsPtOnMe(POINT pt){
	RECT crt;

	SetRect(&crt, _X, _Y, _X + _Width, _Y + _Height);
	return PtInRect(&crt, pt);
}

BOOL Button::IsPtOnMe(LONG x, LONG y){
	POINT pt = {x, y};
	return IsPtOnMe(pt);
}

void Button::DrawBitmap(HDC hDC){
	if(_State == HIDDEN){return;}

	BOOL ParentDC = FALSE;

	if(hDC == NULL){
		ParentDC = TRUE;
		hDC = GetDC(_hParent);
	}

	HBITMAP hBitmap = _hBitmap[_State];
	HDC hMemDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	BitBlt(hDC, _X, _Y, _Width, _Height, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);

	if(ParentDC){ ReleaseDC(_hParent, hDC); }
}
