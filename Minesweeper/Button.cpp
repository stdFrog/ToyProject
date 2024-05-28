#include <windows.h>
#include "Button.h"

/*
	커스텀 컨트롤의 그리기 주체를 바꿀 필요가 있으므로 전체 수정 필요
	2024.05.27 11:24
*/

UINT Button::ButtonCount = 0;

VOID Button::OnPaint(HDC hDC){
	DrawBitmap(hDC);
}

VOID Button::OnPressed(LPARAM lParam){

}

VOID Button::OnReleased(LPARAM lParam){

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

	if(_hBitmap == NULL){
		_hBitmap = CreateCompatibleBitmap(hDC, _x + _Width, _y + _Height);
	}

	HDC hMemDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOld = SelectObject(hMemDC, _hBitmap);

	switch(_State){
		case NORMAL:
			{
				_Color = Color(Color::Gray);
				HBRUSH hBrush = CreateSolidBrush(_Color);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
				Rectangle(hMemDC, _x, _y, _Width, _Height);
				DeleteObject(SelectObject(hMemDC, hOldBrush));
			}
			break;

		case PRESSED:
			{
				_Color = Color(Color::White);
				HBRUSH hBrush = CreateSolidBrush(_Color);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
				Rectangle(hMemDC, _x, _y, _Width, _Height);
				DeleteObject(SelectObject(hMemDC, hOldBrush));
			}
			break;
	}

	BitBlt(hDC, _x, _y, _Width, _Height, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);

	if(ParentDC){ ReleaseDC(_hParent, hDC); }
}

