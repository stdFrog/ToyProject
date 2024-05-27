#include <windows.h>
#include "Button.h"

/*
	커스텀 컨트롤의 그리기 주체를 바꿀 필요가 있으므로 전체 수정 필요
	2024.05.27 11:24
*/

Button::Button(DWORD Style, LONG x, LONG y, LONG w, LONG h, HWND hParent, UINT ID) : _Style(Style), _X(x), _Y(y), _Width(w), _Height(h), _hParent(hParent), _ID(ID), _bCapture(FALSE), _bTimer(FALSE), _State(NORMAL)
{

}

Button::~Button(){

}

void Button::OnPaint(HDC hDC){
	DrawBitmap(hDC);
}

void Button::OnTimer(){
	if(_bTimer == FALSE){return;}

	POINT pt;
	GetCursorPos(&pt);

	ScreenToClient(_hParent, &pt);
	if(IsPtOnMe(pt) == FALSE){
		KillTimer(_hParent, 0x400);
		_bTimer = FALSE;
		ChangeState(NORMAL);
	}
}

void Button::OnMove(LPARAM lParam){
	if(_State == HIDDEN || _State == DISABLE){return;}

	int x = (int)(short)LOWORD(lParam);
	int y = (int)(short)HIWORD(lParam);
	
	if(_bCapture){
		if((_Style & CHECKBUTTON) == 0){
			if(IsPtOnMe(x,y)){
				ChangeState(PRESSED);
			}else{
				ChangeState(NORMAL);
			}
		}else{
			if(IsPtOnMe(x,y)){
				ChangeState((_State == NORMAL) ? PRESSED : NORMAL);
			}else{
				ChangeState(_State);
			}
		}
	}else{
		for(HWND hParent = _hParent; GetParent(hParent); hParent = GetParent(hParent)){;}
		if(GetForegroundWindow() != _hParent){return;}

		if(GetCapture() == NULL && (_Style & CHECKBUTTON) == 0 && IsPtOnMe(x,y)){
			SetTimer(_hParent, 0x400, 50, NULL);
			_bTimer = TRUE;
			ChangeState(HOT);
		}
	}
}

void Button::OnPressed(LPARAM lParam){
	if(_State == HIDDEN || _State == DISABLE){return;}

	if(IsPtOnMe(LOWORD(lParam), HIWORD(lParam))){
		if((_Style & CHECKBUTTON) == 0){
			ChangeState(PRESSED);
		}
	}else{
		ChangeState((_State == NORMAL) ? PRESSED : NORMAL);
	}

	SetCapture(_hParent);
	_bCapture = TRUE;
}

void Button::OnReleased(LPARAM lParam){
	if(_bCapture){
		ReleaseCapture();
		_bCapture = FALSE;

		if((_Style & CHECKBUTTON) == 0){
			ChangeState(NORMAL);
		}

		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(_hParent, &pt);

		if(IsPtOnMe(pt)){
			/* TODO : State에 따른 이미지 변경 */
		}
	}
}

void Button::ChangeState(STATE NewState){
	if(_State == NewState) {return;}

	_State = NewState;
	DrawBitmap(NULL);
}

void Button::SetState(STATE NewState){
	if((_Style & CHECKBUTTON) != 0){
		ChangeState(NewState);
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

	/* TODO : 비트맵? 그리기? */
	Rectangle(hMemDC, 10, 10, 20, 20);

	BitBlt(hDC, _X, _Y, _Width, _Height, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);

	if(ParentDC){ ReleaseDC(_hParent, hDC); }
}

