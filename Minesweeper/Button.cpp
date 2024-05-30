#include <windows.h>
#include "Button.h"

UINT Button::ButtonCount = 0;

VOID Button::OnPaint(HDC hDC){
	DrawBitmap(hDC);
}

VOID Button::OnPressed(LPARAM lParam, BOOL bLeft){
	if(_State == PRESSED){ return; }

	if(IsPtOnMe(LOWORD(lParam), HIWORD(lParam))){
		if(bLeft){
			ChangeState(PRESSED);
		}else{
			ChangeState(BLOCK);
		}
	}
}

VOID Button::OnReleased(LPARAM lParam, BOOL bLeft){
	/* 사용 안함 */
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

	switch(_State){
		case NORMAL:
			{
				HBRUSH hBrush = CreateSolidBrush(_Color);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Rectangle(hDC, _x, _y, _Width, _Height);
				DeleteObject(SelectObject(hDC, hOldBrush));
			}
			break;

		case PRESSED:
			{
				HBRUSH hBrush = CreateSolidBrush(_Color);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Rectangle(hDC, _x, _y, _Width, _Height);
				DeleteObject(SelectObject(hDC, hOldBrush));
			}
			break;

		case BLOCK:
			{

			}
			break;

		default:
			break;
	}

	if(ParentDC){ ReleaseDC(_hParent, hDC); }
}

