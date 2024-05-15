#include "Control.h"

Button::Button(DWORD Style, int x, int y, int Width, int Height, UINT ID, HWND hParent)
: _x(x), _y(y), _Width(Width), _Height(Height), _Style(Style), _ID(ID), _hParent(hParent),
_State(GB_NORMAL), _bCapture(FALSE), _bTimer(FALSE)
{

}

Button::~Button() {

}

void Button::DrawBitmap(HDC hdc) {
	HDC hMemDC;
	HBITMAP hBitmap, hOldBitmap;
	BOOL NullDC = FALSE;

	if(hdc == NULL) {
		NullDC = TRUE;
		hdc = GetDC(hParent);
	}

	if(State == GB_HIDDEN){return;}

	hBit = Bitmap[State];
	hMemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(hMemDC, hBit);

	BitBlt(hdc, x,y,w,h, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, OldBitmap);
	DeleteDC(hMemDC);

	if(NullDC) {
		ReleaseDC(hParent, hdc);
	}
}

BOOL Button::IsPtOnMe(POINT pt){
	RECT crt;
	SetRect(&crt, x, y, x + w, y + h);

	return (PtInRect(&crt, pt));
}

BOOL Button::IsPtOnMe(int x, int y){
	POINT pt = {x,y};

	return IsPtOnMe(pt);
}

void Button::SetState(eState State){
	if((Style & GBS_CHECK) != 0) {
		ChangeState(State);
	}
}

void Button::Enable(BOOL bEnable) {
	if(bEnable){
		State = GB_NORMAL;
	}else{
		State = GB_DISABLE;
		KillTimer(hParent, 1234);
		TimerByMe = FALSE;
	}

	DrawBitmap(NULL);
}

void Button::Show(BOOL bShow) {
	if(bShow) {
		State = GB_NORMAL;
		DrawBitmap(NULL);
	}else{
		State = GB_HIDDEN;
		InvalidateRect(hParent, NULL, TRUE);
	}
}

void Button::OnPaint(HDC hdc){
	DrawBitmap(hdc);
}

void Button::OnTimer(){
	POINT pt;

	if(bTimer == FALSE){return;}

	GetCursorPos(&pt);
	ScreenToClient(hParent, &pt);

	if(IsPtOnMe(pt) == FALSE) {
		KillTimer(hParent, 1234);
		bTimer = FALSE;
		ChangeState(GB_NORMAL);
	}
}

void Button::OnDown(LPARAM lParam)
{
	if(State == GB_HIDDEN || State == GB_DISABLE){return;}

	if(IsPtOnMe(LOWORD(lParam), HIWORD(lParam)))
	{
		if((Style & GBS_CHECK) == 0)
		{
			ChangeState(GB_DOWN);
		}else{
			CheckState = State;
			ChangeState(CheckState == GB_NORMAL ? GB_DOWN : GB_NORMAL);
		}

		SetCapture(hParent);
		bCapture = TRUE;
	}
}

void Button::OnMove(LPARAM lParam)
{	
	HWND hParent;
	int x = (int)(short)LOWORD(lParam);
	int y = (int)(short)HIWORD(lParam);

	if(State == GB_HIDDEN || State == GB_DISABLE){return;}

	if(bCapture) {
		if((Style & GBS_CHECK) == 0) {
			if(IsPtOnMe(x,y)) {
				ChangeState(GB_DOWN);
			}else{
				ChangeState(GB_NORMAL);
			}
		}else{
			if(IsPtOnMe(x,y)) {
				ChangeState(CheckState == GB_NORMAL ? GB_DOWN : GB_NORMAL);
			}else{
				ChangeState(CheckState);
			}
		}
	}else{
		for(hParent = this->hParent; GetParent(hParent); hParent = GetParent(hParent)){;}

		// 현재 프로그램의 주체(오너 = 최상위 윈도우)가 활성되어 있니?
		if(GetForegroundWindow() != hParent){return;}

		// 다른 누군가가 캡처하고 있는 상황에서는 핫 상태로 변하지 않게끔 한다.
		if(GetCapture() == NULL && (Style & GBS_CHECK) == 0 && IsPtOnMe(x,y)) {
			SetTimer(hParent, 1234, 50, NULL);
			TimerByMe = TRUE;
			ChangeState(GB_HOT);
		}
	}
}

void Button::OnUp(LPARAM lParam)
{
	POINT pt;

	if(bCapture){
		ReleaseCapture();
		bCapture = FALSE;

		if((Style & GBS_CHECK) == 0) {
			ChangeState(GB_NORMAL);
		}

		GetCursorPos(&pt);
		ScreenToClient(hParent, &pt);
		if(IsPtOnMe(pt)) {
			SendMessage(hParent, WM_COMMAND, MAKEWPARAM(ID, GBN_CLICKED), 0);
		}
	}
}
