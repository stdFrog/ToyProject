#define UNICODE
#define DEBUG
#include "..\\Headers\\resource.h"
#include "..\\Headers\\Color.h"
#include "..\\Headers\\Button.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32")

#define TILE_SIZE 16

BOOL bLeak = FALSE;
int StretchMode;
OSVERSIONINFO osv;

HWND hStatusWnd;
RECT g_crt, g_wrt, g_trt;
HBITMAP hClientBitmap = NULL;
HMENU hPopupSettings, hPopupSize;

typedef struct tag_MapSize{
	INT x,y;
}MapSize;

MapSize Table[3] = {
	{25, 25},
	{30, 20},
	{40, 30}
};

typedef enum { GAME_BEGIN, GAME_PAUSE, GAME_END } GAME_STATE;
UINT_PTR Index;
UINT_PTR g_Time;

int Bomb;
GAME_STATE g_GameState;
HBITMAP g_hBitmap[5];
Button** Btns = NULL;

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
	CheckMenuRadioItem(hPopupSize, ID_SYS_RESIZE1, ID_SYS_RESIZE2, ID_SYS_RESIZE1, MF_BYCOMMAND);

	g_hBitmap[0] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_NORMAL));
	g_hBitmap[1] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ONE));
	g_hBitmap[2] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TWO));
	g_hBitmap[3] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_THREE));
	g_hBitmap[4] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PRESSED));

	InitCommonControls();
	// hStatusWnd = CreateStatusWindow(WS_VISIBLE | WS_CHILD, TEXT(""), hWnd, IDW_STATUS);
	hStatusWnd = CreateWindowEx(0, STATUSCLASSNAME, NULL, /*SBARS_SIZEGRIP | SBARS_TOOLTIPS */ WS_CHILD | WS_VISIBLE, 0,0,0,0, hWnd, (HMENU)IDW_STATUS, GetModuleHandle(NULL), NULL);

	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	SetWindowLong(hWnd, GWL_STYLE, dwStyle & ~WS_THICKFRAME);
	SendMessage(hWnd, WM_NCPAINT, 1, 0);

	Bomb = 0;
	Index = 0;
	SetClientRect(hWnd, TILE_SIZE * Table[Index].x, TILE_SIZE * Table[Index].y + (g_trt.bottom - g_trt.top));
	GetClientRect(hWnd, &g_crt);
	g_GameState = GAME_PAUSE;

	Btns = CreateButtons(Table[Index].x, Table[Index].y);
	InitButtons(hWnd, Btns, Table[Index].x, Table[Index].y);

	return 0;
}

LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam){
	if(hClientBitmap){DeleteObject(hClientBitmap);}
	if(g_hBitmap){
		for(int i=0; i<5; i++){
			if(g_hBitmap[i]){
				DeleteObject(g_hBitmap[i]);
			}
		}
	}
	if(Btns){DestroyButtons(Btns, Table[Index].x, Table[Index].y);}

	KillTimer(hWnd, 1);
	PostQuitMessage(0);
	return 0;
}



LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){
	OnPressedButtons(lParam, TRUE, Btns, Table[Index].x, Table[Index].y);
	return 0;
}

LRESULT OnRButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){
	OnPressedButtons(lParam, FALSE, Btns, Table[Index].x, Table[Index].y);
	return 0;
}

LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam){
	OnReleasedButtons(TRUE, Btns, Table[Index].x, Table[Index].y);
	return 0;
}

LRESULT OnRButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam){
	OnReleasedButtons(FALSE, Btns, Table[Index].x, Table[Index].y);
	return 0;
}

LRESULT OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam){
	OnMoveButtons(lParam, Btns, Table[Index].x, Table[Index].y);
	return 0;
}

LRESULT OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam){
	UINT_PTR ID;
	static WPARAM PrevCommand = 0;

	switch(LOWORD(wParam)){
		case ID_SYS_ABOUT:
			MessageBox(hWnd, TEXT("이 프로그램은 고전 게임 '지뢰찾기'를 모작한 것이며 마우스 조작을 통해 게임을 진행하실 수 있습니다.\r\n\r\n시스템적 변화는 일절 없으며 메뉴에서 게임 옵션을 조정할 수 있습니다.\r\n\r\n"), TEXT("프로그램 소개"), MB_OK);
			return 0;

		case ID_SYS_NEWGAME:
			return 0;

		case ID_SYS_RESIZE1:
		case ID_SYS_RESIZE2:
		case ID_SYS_RESIZE3:
			if(LOWORD(PrevCommand) != LOWORD(wParam)){
				PrevCommand = wParam;
				CheckMenuRadioItem(hPopupSize, ID_SYS_RESIZE1, ID_SYS_RESIZE3, LOWORD(wParam), MF_BYCOMMAND);
				// TODO : Replace Map
			}
			return 0;

		default:
			return DefWindowProc(hWnd, WM_SYSCOMMAND, wParam, lParam);
	}
}

LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam){
	MSG msg = {0};
	INT Part[3];

	if(wParam != SIZE_MINIMIZED) {
		if(hClientBitmap != NULL){
			DeleteObject(hClientBitmap);
			hClientBitmap = NULL;
		}

		SendMessage(hStatusWnd, WM_SIZE, wParam, lParam);
		for(int i=0; i<3; i++){
			Part[i] = LOWORD(lParam) / 3 * (i + 1);
		}
		SendMessage(hStatusWnd, SB_SETPARTS, 3, (LPARAM)Part);
		SendMessage(hStatusWnd, SB_GETRECT, 0, (LPARAM)&g_trt);
	}
	return 0;
}

LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	OnDrawButtons(hdc, Btns, Table[Index].x, Table[Index].y);
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam){
	RECT srt;

	switch(wParam){
		case 1:
			{
				if(g_GameState == GAME_BEGIN){
					g_Time++;
				}else{
					g_Time = 0;
					KillTimer(hWnd, 1);
				}
			}
			break;
	}

	SendMessage(hStatusWnd, SB_GETRECT, 2, (LPARAM)&g_trt);
	InvalidateRect(hWnd, &g_trt, TRUE);
	return 0;
}


























/* Utility */
void DrawBitmap(HDC hDC, LONG X, LONG Y, HBITMAP hBitmap){
	if(hBitmap == NULL){return;}

	HDC hMemDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	BitBlt(hDC, X,Y, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

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

void SetStatusText(HWND hWnd){
	TCHAR Info[MAX_PATH];

	wsprintf(Info, TEXT("Hidden Bombs : %d"), ((g_GameState != GAME_BEGIN) ? 0 : Bomb));
	SendMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)Info);
	wsprintf(Info, TEXT("Map Size : %dx%d"), Table[Index].x, Table[Index].y);
	SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)Info);
	wsprintf(Info, TEXT("경과 시간 : %d(s)"), g_Time);
	SendMessage(hStatusWnd, SB_SETTEXT, 2, (LPARAM)Info);
}

Button** CreateButtons(int W, int H){
	Button **Btns = new Button*[H];
	for(int i=0; i<H; i++){
		Btns[i] = new Button[W];
	}

	return Btns;
}

void InitButtons(HWND hWnd, Button** Btns, int W, int H){
	for(int i=0; i<H; i++){
		for(int j=0; j<W; j++){
			Btns[i][j].SetParent(hWnd);
			Btns[i][j].SetX(j * TILE_SIZE);
			Btns[i][j].SetY(i * TILE_SIZE);
			Btns[i][j].SetBitmap(g_hBitmap);
		}
	}
}

void DestroyButtons(Button** Target, int W, int H){
	for(int i=0; i<H; i++){
		delete [] Target[i];
	}
	delete [] Target;
}

/*
	버튼을 그리는 도중 발생하는 일련의 메시지에 대하여
	교착 상태가 발생할 수 있으므로 메시지 펌프를 활용한다.
*/
void OnDrawButtons(HDC hdc, Button** Btns, int W, int H){
	MSG msg = {0};

	for(int i=0; i<H; i++){
		for(int j=0; j<W; j++){
			while(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE)){
				if(msg.message == WM_QUIT){
					PostQuitMessage(0);
				}

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			Btns[i][j].OnPaint(hdc);
		}
	}
}

/* 
	현재 구조로 테스트해 본 결과,
	반응이 느리고 재진입 허가 상태이므로 단순 이중 for문 구조가 아닌
	화면 좌표값으로부터 배열 첨자를 계산하는 구조로 변경되어야 한다.
*/
void OnPressedButtons(LPARAM lParam, BOOL bLeft, Button** Btns, int W, int H){
	for(int i=0; i<H; i++){
		for(int j=0; j<W; j++){
			Btns[i][j].OnReleased(bLeft);
		}
	}

}

void OnReleasedButtons(BOOL bLeft, Button** Btns, int W, int H){
	for(int i=0; i<H; i++){
		for(int j=0; j<W; j++){
			Btns[i][j].OnReleased(bLeft);
		}
	}
}

void OnMoveButtons(LPARAM lParam, Button** Btns, int W, int H){
	MSG msg = {0};

	for(int i=0; i<H; i++){
		for(int j=0; j<W; j++){
			while(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE)){
				if(msg.message == WM_QUIT){
					PostQuitMessage(0);
				}

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			Btns[i][j].OnMove(lParam);
		}
	}
}
