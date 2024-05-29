#define UNICODE
#define DEBUG
#include "resource.h"
#include "Button.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32")

HWND hStatusWnd;
RECT g_crt, g_wrt, g_trt;
HMENU hPopupSettings, hPopupSize;
HBITMAP hClientBitmap = NULL, hBkBitmap = NULL;

int StretchMode;
OSVERSIONINFO osv;

Button** Buttons = NULL;

/*
	굉장히 오래된 API를 이용하여 게임을 제작하다 보니 굉장히 많은 시행착오를 겪고 있다.
	디테일한 부분까지 모두 직접 신경써야 하므로 메시지의 종류뿐 아니라 함수와 리소스,
	화면 관리 방법까지 하나하나 찾아볼 수 밖에 없다.

	남은 시간이 길지 않으나 여러 기법을 적용해보고 학습에 목적을 두기로 한다.
*/























LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam){
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osv);

	if(osv.dwPlatformId >= VER_PLATFORM_WIN32_NT)
	{
		StretchMode = HALFTONE;
	}else{
		StretchMode = COLORONCOLOR;
	}

	HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
	AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hSysMenu, MF_STRING, ID_SYS_ABOUT, TEXT("프로그램 소개"));

	hPopupSettings = CreatePopupMenu();
	AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hPopupSettings, TEXT("설정"));
	AppendMenu(hPopupSettings, MF_STRING, ID_SYS_NEWGAME, TEXT("새 게임"));

	hPopupSize = CreatePopupMenu();
	AppendMenu(hPopupSettings, MF_STRING | MF_POPUP, (UINT_PTR)hPopupSize, TEXT("크기 변경"));
	AppendMenu(hPopupSize, MF_STRING, ID_SYS_RESIZE1, TEXT("25 X 25"));
	AppendMenu(hPopupSize, MF_STRING, ID_SYS_RESIZE2, TEXT("30 X 20"));
	AppendMenu(hPopupSize, MF_STRING, ID_SYS_RESIZE3, TEXT("40 X 30"));

	InitCommonControls();
	hStatusWnd = CreateStatusWindow(WS_VISIBLE | WS_CHILD, TEXT(""), hWnd, IDW_STATUS);
	SendMessage(hStatusWnd, SB_GETRECT, 0, (LPARAM)&g_trt);
	
	SetClientRect(hWnd, 16 * 25, 16 * 25 + (g_trt.bottom - g_trt.top));
	Buttons = CreateButton(25, 25);
	InitializeButton(hWnd, Buttons);

	GetClientRect(hWnd, &g_crt);

	SetTimer(hWnd, 1, 50, NULL);
	SendMessage(hWnd, WM_TIMER, 1, 0);
	return 0;
}















LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam){
	if(hBkBitmap){DeleteObject(hBkBitmap);}
	if(hClientBitmap){DeleteObject(hClientBitmap);}
	if(Buttons){DestroyButton(Buttons);}

	KillTimer(hWnd, 1);
	PostQuitMessage(0);
	return 0;
}
















LRESULT OnInitMenu(HWND hWnd, WPARAM wParam, LPARAM lParam){
	
	
	return 0;
}











LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){
	return 0;
}

LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam){
	return 0;
}

LRESULT OnRButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){
	return 0;
}

LRESULT OnRButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam){
	return 0;
}

LRESULT OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam){
	return 0;
}













LRESULT OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam){
	switch(LOWORD(wParam)){
		case ID_SYS_ABOUT:
			MessageBox(hWnd, TEXT("이 프로그램은 고전 게임 '지뢰찾기'를 모작한 것이며 마우스 조작을 통해 게임을 진행하실 수 있습니다.\r\n\r\n시스템적 변화는 일절 없으며 시스템 메뉴에서 게임 옵션을 조정할 수 있습니다.\r\n\r\n"), TEXT("프로그램 소개"), MB_OK);
			return 0;

		case ID_SYS_NEWGAME:
			return 0;

		case ID_SYS_RESIZE1:
			return 0;

		case ID_SYS_RESIZE2:
			return 0;

		case ID_SYS_RESIZE3:
			return 0;

		default:
			return DefWindowProc(hWnd, WM_SYSCOMMAND, wParam, lParam);
	}
}









LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam){
	INT Part[3];

	if(wParam != SIZE_MINIMIZED) {
		KillTimer(hWnd, 1);
		/*
		if(hClientBitmap != NULL){
			DeleteObject(hClientBitmap);
			hClientBitmap = NULL;
		}
		*/

		SendMessage(hStatusWnd, WM_SIZE, wParam, lParam);
		for(int i=0; i<3; i++){
			Part[i] = LOWORD(lParam) / 3 * (i + 1);
		}
		SendMessage(hStatusWnd, SB_SETPARTS, 3, (LPARAM)Part);
	}
	return 0;
}

LRESULT OnExitSizeMove(HWND hWnd, WPARAM wParam, LPARAM lParam){
	if(hClientBitmap != NULL){
		DeleteObject(hClientBitmap);
		hClientBitmap = NULL;
	}

	SetTimer(hWnd, 1, 50, NULL);
	SendMessage(hWnd, WM_TIMER, 1, 0);
	return 0;
}







LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	if(hClientBitmap){
		DrawBitmap(hdc, 0,0, hClientBitmap);
	}else if(hBkBitmap){
		DrawBitmap(hdc, 0,0, hBkBitmap);
	}
	EndPaint(hWnd, &ps);
	return 0;
}











/* 
	굳이 타이머를 쓰겠다 하면, 아래와 같은 코드를 작성할 수 있다.
	단, 화면을 계속해서 그려야 하는 게임이 아니므로 이 코드는 보류하고
	모든 그리기 관련 코드와 게임 관련 함수를 WM_PAINT에서 처리하기로 한다.
 */
LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam){
	switch(wParam){
		case 1:
			{
				HDC hdc = GetDC(hWnd);
				HDC hMemDC = CreateCompatibleDC(hdc);
				HDC hBkDC = CreateCompatibleDC(hdc);

				if(hClientBitmap == NULL){
					GetClientRect(hWnd, &g_crt);
					hClientBitmap = CreateCompatibleBitmap(hdc, g_crt.right, g_crt.bottom);

					if(hBkBitmap != NULL){DeleteObject(hBkBitmap);}
					hBkBitmap = CreateCompatibleBitmap(hdc, g_crt.right, g_crt.bottom);
				}

				HGDIOBJ hOld = SelectObject(hMemDC, hClientBitmap);
				HGDIOBJ hOld2 = SelectObject(hBkDC, hBkBitmap);

				FillRect(hMemDC, &g_crt, GetSysColorBrush(COLOR_WINDOW));
				FillRect(hBkDC, &g_crt, GetSysColorBrush(COLOR_WINDOW));

				DisplayButtons(hMemDC, Buttons);

				TCHAR Info[0x400];
				wsprintf(Info,
					TEXT(" Type = %s\r\n")
					TEXT(" Position(%d, %d, %d, %d)\r\n")
					TEXT(" ID = %d\r\n")
					TEXT(" Parent = %#x\r\n")
					TEXT(" State = %s\r\n")
					TEXT(" Shape = %s\r\n")
					TEXT(" Color(R: %d, G: %d, B: %d)\r\n ")
					TEXT(" Array Size = H(%d), W(%d)\r\n ")
					,
					((Buttons[0][2].GetType() == PUSH) ? TEXT("PUSH BUTTON") : TEXT("")),
					 Buttons[0][2].GetX(), Buttons[0][0].GetY(), Buttons[0][0].GetWidth(), Buttons[0][0].GetHeight(),
					Buttons[0][2].GetID(),
					Buttons[0][2].GetParent(),
					((Buttons[0][2].GetState() == NORMAL) ? TEXT("NORMAL") : TEXT("")),
					((Buttons[0][2].GetShape() == RECTANGLE) ? TEXT("RECTANGLE") : TEXT("")),
					GetRValue(Buttons[0][2].GetColorRef()), GetGValue(Buttons[0][2].GetColorRef()), GetBValue(Buttons[0][2].GetColorRef()),
					sizeof(Buttons) / sizeof(Buttons[0]),
					sizeof(Buttons[0]) / sizeof(Button)
				);

				TextOut(hMemDC, 10, 16, Info, lstrlen(Info));

				if(hClientBitmap){
					DrawBitmap(hBkDC, 0,0, hClientBitmap);
				}

				SelectObject(hMemDC, hOld);
				DeleteDC(hBkDC);
				DeleteDC(hMemDC);
				ReleaseDC(hWnd, hdc);
			}
			break;
	}

	InvalidateRect(hWnd, NULL, FALSE);
	return 0;
}


























/* Utility */
void DrawBitmap(HDC hDC, LONG X, LONG Y, HBITMAP hBitmap){
	if(hBitmap == NULL){return;}

	HDC hMemDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	BitBlt(hDC, X,Y, bmp.bmWidth, bmp.bmHeight, hMemDC, X,Y, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
}

void SetClientRect(HWND hWnd, int Width, int Height){
	RECT crt;
	DWORD Style, ExStyle;

	SetRect(&crt, 0,0, Width, Height);
	Style = GetWindowLong(hWnd, GWL_STYLE);
	ExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);

	AdjustWindowRectEx(&crt, Style, GetMenu(hWnd) != NULL, ExStyle);
	if(Style & WS_VSCROLL){crt.right += GetSystemMetrics(SM_CXVSCROLL);}
	if(Style & WS_HSCROLL){crt.bottom += GetSystemMetrics(SM_CYHSCROLL);}

	SetWindowPos(hWnd, NULL, 0,0, crt.right - crt.left, crt.bottom - crt.top, SWP_NOZORDER);
}

/*
void SetStatusText(HWND hWnd){
	TCHAR Info[MAX_PATH];

	wsprintf(Info, TEXT("Hidden Bombs : %d"), ((g_GameState != GAME_BEGIN) ? 0 : BOMB_COUNT));
	SendMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)Info);
	wsprintf(Info, TEXT("Map Size : %dx%d"), MAP_WIDTH, MAP_HEIGHT);
	SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)Info);
	wsprintf(Info, TEXT("경과 시간 : %d(s)"), g_Time);
	SendMessage(hStatusWnd, SB_SETTEXT, 2, (LPARAM)Info);
}
*/

/* Game Settings */
Button** CreateButton(int w, int h){
	Button** Btns = new Button*[h];
	for(int i=0; i<h; i++){
		Btns[i] = new Button[w];
	}

	return Btns;
}

void DestroyButton(Button** Btns){
	for(int i=0; i<25; i++){
		delete [] Btns[i];
	}

	delete [] Btns;
}

BOOL InitializeButton(HWND hWnd, Button** Btns){
	/*
	for(int i=0; i<sizeof(Btns) / sizeof(Btns[0]); i++){
		for(int j=0; j<sizeof(Btns[0]) / sizeof(Button); j++){
		*/
	for(int i=0; i<25; i++){
		for(int j=0; j<25; j++){
			Btns[i][j].ChangeParent(hWnd);
			Btns[i][j].SetX(j * 16);
			Btns[i][j].SetY(i * 16);
			Btns[i][j].SetWidth(16);
			Btns[i][j].SetHeight(16);
			Btns[i][j].SetColor(Color(Color::LightGray));
		}
	}

	return TRUE;
}

/* OnFunction */
void DisplayButtons(HDC hDC, Button** Btns){
	/*
	for(int i=0; i<sizeof(Btns) / sizeof(Btns[0]); i++){
		for(int j=0; j<sizeof(Btns[0]) / sizeof(Button); j++){
		*/
	for(int i=0; i<25; i++){
		for(int j=0; j<25; j++){
			Btns[i][j].OnPaint(hDC);
		}
	}
}
