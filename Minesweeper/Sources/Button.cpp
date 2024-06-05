#include <windows.h>
#include "..\\Headers\\Button.h"

UINT Button::ButtonCount = 0;

VOID Button::OnPaint(HDC hDC, HBITMAP hBitmap){
	DrawBitmap(hDC, hBitmap);
}

VOID Button::OnPressed(LPARAM lParam, HBITMAP hBitmap, BOOL bLeft){
	if(_State == PRESSED){ return; }

	if(IsPtOnMe(LOWORD(lParam), HIWORD(lParam))){
		if(bLeft){
			ChangeState(PRESSED, hBitmap);
		}else{
			/*
			ChangeState(BLOCK, hBitmap);
			*/
		}
	}
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

VOID Button::DrawBitmap(HDC hDC, HBITMAP hBitmap){
	BOOL ParentDC = FALSE;

	if(hDC == NULL){
		ParentDC = TRUE;
		hDC = GetDC(_hParent);
	}

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	HDC hMemDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);
	BitBlt(hDC, 0,0, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);
	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);

	if(ParentDC){ ReleaseDC(_hParent, hDC); }
}


