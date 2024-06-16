#include <windows.h>
#include "..\\Headers\\Button.h"

UINT Button::ButtonCount = 0;

VOID Button::OnPaint(HDC hDC){
	DrawBitmap(hDC);
}

VOID Button::OnLPressed(){
	if(_State != NORMAL){return;}

	ChangeState(PRESSING);

	if(GetCapture() == NULL){
		SetCapture(_hParent);
		_bCapture = TRUE;
	}
}

VOID Button::OnRPressed(){
	if(_State != NORMAL && _State != BLOCK){return;}
	ChangeState((_State == NORMAL) ? (BLOCK) : (NORMAL));
}


VOID Button::OnReleased(){
	if(!_bCapture){return;}

	ReleaseCapture();
	_bCapture = FALSE;
 
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(_hParent, &pt);

	ChangeState((IsPtOnMe(pt)) ? (PRESS) : (NORMAL));
	/* TODO : 주변 8칸 탐색, 비어있는 칸 활성 상태로 변환 */
}

BOOL Button::IsPtOnMe(POINT pt){
	RECT crt;

	SetRect(&crt, _x, _y, _x + _Width, _y + _Height);
	return PtInRect(&crt, pt);
}

BOOL Button::IsPtOnMe(LONG x, LONG y){
	POINT pt = {x, y};
	return IsPtOnMe(pt);
}

VOID Button::DrawBitmap(HDC hDC){
	BOOL ParentDC = FALSE;

	if(hDC == NULL){
		ParentDC = TRUE;
		hDC = GetDC(_hParent);
	}

	HDC hMemDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOld = SelectObject(hMemDC, _hBitmap[_State]);
	BitBlt(hDC, _x,_y, _Width, _Height, hMemDC, 0,0, SRCCOPY);
	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);

	if(ParentDC){ ReleaseDC(_hParent, hDC); }
}


