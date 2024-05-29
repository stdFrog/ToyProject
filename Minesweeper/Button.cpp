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

VOID Button::DisplayState(){
	TCHAR Info[0x400];
	wsprintf(Info,
			TEXT("Type = %s\r\n")
			TEXT("Position(%d, %d, %d, %d)\r\n")
			TEXT("ID = %d\r\n")
			TEXT("Parent = %#x\r\n")
			TEXT("State = %s\r\n")
			TEXT("Shape = %s\r\n")
			TEXT("Capture = %s\r\n"),
			TEXT("Bitmap = %s\r\n")
			,
			((_Type == PUSH) ? TEXT("PUSH BUTTON") : TEXT("")),
			 _x, _y, _Width, _Height,
			_ID,
			_hParent,
			((_State == NORMAL) ? TEXT("NORMAL") : TEXT("")),
			((_Shape == RECTANGLE) ? TEXT("RECTANGLE") : TEXT("")),
			((_bCapture) ? TEXT("CAPTURE") : TEXT("Not CAPTURE")),
			((_hBitmap == NULL) ? TEXT("Is NULL") : TEXT("Not NULL"))
		);

	MessageBox(NULL, Info, TEXT("Display State Info"), MB_OK);
}

VOID Button::OnPressed(LPARAM lParam, BOOL bLeft){
	if(_State == PRESSED){ return; }

	if(IsPtOnMe(LOWORD(lParam), HIWORD(lParam))){
		if(bLeft){
			ChangeState(PRESSED);
		}else{
			/* TODO : 체크 표시 또는 깃발 표시 */
		}

		/* 
		   잘못 클릭하여 취소하고 싶은 경우
		   대부분 마우스를 누른채 화면 밖으로 이동하므로
		   필요시 이에 대한 처리도 추가할 것
		 */
	}
}

VOID Button::OnReleased(LPARAM lParam, BOOL bLeft){
	/*
		위 Pressed의 예외 분기를 추가할 경우 Released로 상태를 변화시키는 코드를 옮겨와야 함
	 */
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
	/*
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
	*/

	switch(_State){
		default:
		case NORMAL:
			{
				/* TODO : 그라데이션 / 애니메이션 효과 */
				/*
				HBRUSH hBrush = CreateSolidBrush(_Color);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
				Rectangle(hMemDC, _x, _y, _Width, _Height);
				DeleteObject(SelectObject(hMemDC, hOldBrush));
				*/
				HBRUSH hBrush = CreateSolidBrush(_Color);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Rectangle(hDC, _x, _y, _Width, _Height);
				DeleteObject(SelectObject(hDC, hOldBrush));
			}
			break;

		case PRESSED:
			{
				/*
				HBRUSH hBrush = CreateSolidBrush(_Color);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
				Rectangle(hMemDC, _x, _y, _Width, _Height);
				DeleteObject(SelectObject(hMemDC, hOldBrush));
				*/

				HBRUSH hBrush = CreateSolidBrush(_Color);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Rectangle(hDC, _x, _y, _Width, _Height);
				DeleteObject(SelectObject(hDC, hOldBrush));
			}
			break;
	}

	/*
	BitBlt(hDC, _x, _y, _Width, _Height, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);

	if(ParentDC){ ReleaseDC(_hParent, hDC); }
	*/
}

