#define UNICODE
#define DEBUG
#include "resource.h"
#include "Button.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32")

typedef enum { GAME_STOP = 0x1, GAME_BEGIN = 0x2, GAME_PAUSE = 0x4, GAME_OVER = 0x8 } GAME_STATE;
typedef enum { MAP_SIZE25X25 = 0x1, MAP_SIZE30X20 = 0x2, MAP_SIZE40X30 = 0x4 } MAP_TABLE;

RECT g_crt, g_wrt, g_trt;
HBITMAP hClientBitmap;
HMENU hPopupSettings, hPopupSize;

BYTE* pData = NULL;
BITMAPINFOHEADER sih;
BITMAPINFOHEADER ih;

int StretchMode;
OSVERSIONINFO osv;

HWND hStatusWnd;
int BOMB_COUNT = 10;

const int TILE_WIDTH = 16;
const int TILE_HEIGHT = 16;
int MAP_WIDTH;
int MAP_HEIGHT;

GAME_STATE g_GameState = GAME_STOP;
MAP_TABLE g_MapTable = MAP_SIZE25X25;

int g_Time;

Button NewButton(PUSH, 0,0, 16,16, 0, NULL);

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

	MAP_WIDTH = MAP_HEIGHT = 25;
	SetClientRect(hWnd, TILE_WIDTH * MAP_WIDTH, TILE_HEIGHT * MAP_HEIGHT + (g_trt.bottom - g_trt.top));

	g_Time = 0;
	g_GameState = GAME_STOP;

	SetTimer(hWnd, 1, 1000, NULL);
	SendMessage(hWnd, WM_TIMER, 1, 0);

	SetTimer(hWnd, 2, 50, NULL);

	/* TEST */
	NewButton.ChangeParent(hWnd);
	return 0;
}

LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam){
	KillTimer(hWnd, 1);
	if(hClientBitmap){DeleteObject(hClientBitmap);}
	if(pData){free(pData);}
	PostQuitMessage(0);
	return 0;
}

LRESULT OnInitMenu(HWND hWnd, WPARAM wParam, LPARAM lParam){
	switch(g_MapTable){
	case MAP_SIZE25X25:
		CheckMenuRadioItem(hPopupSize, 0, 2, 0, MF_BYPOSITION);
		break;

	case MAP_SIZE30X20:
		CheckMenuRadioItem(hPopupSize, 0, 2, 1, MF_BYPOSITION);
		break;

	case MAP_SIZE40X30:
		CheckMenuRadioItem(hPopupSize, 0, 2, 2, MF_BYPOSITION);
		break;

	default:
		break;
	}
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
			Initialize();
			return 0;

		case ID_SYS_RESIZE1:
			g_MapTable = MAP_SIZE25X25;
			return 0;

		case ID_SYS_RESIZE2:
			g_MapTable = MAP_SIZE30X20;
			return 0;

		case ID_SYS_RESIZE3:
			g_MapTable = MAP_SIZE40X30;
			return 0;

		default:
			return DefWindowProc(hWnd, WM_SYSCOMMAND, wParam, lParam);
	}
}

LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam){
	INT Part[3];

	if(wParam != SIZE_MINIMIZED) {
		if(hClientBitmap) {
			DeleteObject(hClientBitmap);
			hClientBitmap = NULL;
		}

		SendMessage(hStatusWnd, WM_SIZE, wParam, lParam);
		for(int i=0; i<3; i++){
			Part[i] = LOWORD(lParam) / 3 * (i + 1);
		}
		SendMessage(hStatusWnd, SB_SETPARTS, 3, (LPARAM)Part);

	}
	return 0;
}

LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	/* TODO : 게임 상태에 따라 그리기 코드가 변화해야 하므로 게임 화면을 그릴 개별 함수 작성 필요 */

	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam){
	switch(wParam){
		case 1:
			{
				if(g_GameState == GAME_BEGIN){
					g_Time++;
				}

			}
			break;

		case 2:
			{
				SetStatusText(hWnd);
			}

			SendMessage(hStatusWnd, SB_GETRECT, 2, (LPARAM)&g_trt);
			InvalidateRect(hWnd, &g_trt, FALSE);
			break;
	}

	return 0;
}

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

void MapResize(MAP_TABLE CurrentTable){
	/* 대화상자 또는 팝업 메뉴 추가해서 맵 사이즈 변경 */
}

void Initialize(){
	/* TODO : 게임 상태 및 설정에 관련된 정보를 메뉴로부터 받아온 후 변수나 함수를 초기화 또는 호출 */
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

	wsprintf(Info, TEXT("Hidden Bombs : %d"), ((g_GameState != GAME_BEGIN) ? 0 : BOMB_COUNT));
	SendMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)Info);
	wsprintf(Info, TEXT("Map Size : %dx%d"), MAP_WIDTH, MAP_HEIGHT);
	SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)Info);
	wsprintf(Info, TEXT("경과 시간 : %d(s)"), g_Time);
	SendMessage(hStatusWnd, SB_SETTEXT, 2, (LPARAM)Info);
}
