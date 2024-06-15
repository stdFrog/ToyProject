#include <windows.h>
#include "..\\Headers\\Button.h"

UINT Button::ButtonCount = 0;

VOID Button::OnPaint(HDC hDC){
	DrawBitmap(hDC);
}

VOID Button::OnPressed(LPARAM lParam, BOOL bLeft){
	if(_State != NORMAL && _State != BLOCK){return;}

	_bLeft = bLeft;
	if(IsPtOnMe(LOWORD(lParam), HIWORD(lParam))){
		if(bLeft){
			if(_State == NORMAL){
				ChangeState(PRESS);
				MessageBox(NULL, TEXT("Test"), TEXT(""), MB_OK);
			}else{
				ChangeState(NORMAL);
			}
		}

		SetCapture(_hParent);
		_bCapture = TRUE;
	}
}

VOID Button::OnReleased(BOOL bLeft){
	if(!_bCapture){return;}

	ReleaseCapture();
	_bCapture = FALSE;
 
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(_hParent, &pt);

	if(IsPtOnMe(pt)){
		if(bLeft){
			ChangeState(PRESS);
			/* TODO : 주변 8칸 탐색, 비어있는 칸 활성 상태로 변환 */

		}else{
			ChangeState(BLOCK);
		}
	}
}

VOID Button::OnMove(LPARAM lParam){
	LONG x = (LONG)(WORD)LOWORD(lParam);
	LONG y = (LONG)(WORD)HIWORD(lParam);

	if(_bCapture){
		if(IsPtOnMe(x, y)){
			if(_bLeft){
				ChangeState(PRESS);
			}
		}else{
			ChangeState(NORMAL);
		}
	}else{
		HWND hParent;
		for(hParent = _hParent; ::GetParent(hParent); hParent = ::GetParent(hParent)){;}

		if(GetForegroundWindow() != hParent){return;}

		/* 캡처한 프로그램이 없고 내 자신 위에 있을 때 */
		if(GetCapture() == NULL && IsPtOnMe(x,y)){
			SetTimer(hParent, 3, 50, NULL);
			_bTimer = TRUE;
			ChangeState(HOT);
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


