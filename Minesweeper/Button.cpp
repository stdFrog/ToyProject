#include <windows.h>
#include "Button.h"

#define CLAMP(Min, Max, N) (((N) < (Min)) ? (Min) : ((N) < (Max)) ? (N) : (Max))

float lerp(float p1, float p2, float d1){
	return (1 - d1) * p1 + d1 * p2;
}

float length(float x, float y){
	return x * x + y * y;
}

float distance(POINT p1, POINT p2){
	POINT d = {p1.x - p2.x, p1.y - p2.y};
	return length(d.x, d.y);
}

float clamp(float x, float lowerlimit = 0.0f, float upperlimit = 1.0f) {
	if (x < lowerlimit) return lowerlimit;
	if (x > upperlimit) return upperlimit;
	return x;
}

float smoothstep (float edge0, float edge1, float x) {
	// Scale, and clamp x to 0..1 range
	x = clamp((x - edge0) / (edge1 - edge0));

	return x * x * (3.0f - 2.0f * x);
}


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
				/*
					그라데이션 효과를 넣은 사각형을 만들어 출력해봤다.
					연산량이 너무 많아서 제대로 그려지지 않으나 타이머나 메시지 루프를 이용한다면 충분히 가능할 것으로 보인다.
					다만, 게임 특성에 맞지 않으므로 비트맵을 제작하기로 한다.

					그라데이션 효과는 SmoothStep에 대한 글을 찾아보면 알 수 있는데 그리 어렵지 않다.
				 */
				// HBRUSH hBrush = CreateSolidBrush(NewColor);
				// HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, GetStockObject(NULL_BRUSH));

				Rectangle(hDC, _x, _y, _Width, _Height);

				// DeleteObject(SelectObject(hMemDC, hOldBrush));
				SelectObject(hMemDC, hOldBrush);
			}
			break;

		case PRESSED:
			{
				/*
				HBRUSH hBrush = CreateSolidBrush(_Color);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Rectangle(hDC, _x, _y, _Width, _Height);
				DeleteObject(SelectObject(hDC, hOldBrush));
				*/
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


