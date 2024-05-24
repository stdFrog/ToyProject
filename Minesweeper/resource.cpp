#define UNICODE
#define DEBUG
#include "resource.h"
#include "Button.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32")

RECT g_crt, g_wrt;
HBITMAP hClientBitmap;

BYTE* pData = NULL;
BITMAPINFOHEADER sih;
BITMAPINFOHEADER ih;

int StretchMode;
OSVERSIONINFO osv;

HWND hStatusWnd;

const int TILE_WIDTH = 16;
const int TILE_HEIGHT = 16;
int MAPSIZE = 25;

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

	HMENU hPopup = CreatePopupMenu();
	AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hPopup, TEXT("설정"));
	AppendMenu(hPopup, MF_STRING, 40001, TEXT("새 게임"));
	AppendMenu(hPopup, MF_STRING, 40002, TEXT("크기 변경"));

	InitCommonControls();
	hStatusWnd = CreateStatusWindow(WS_VISIBLE | WS_CHILD, TEXT(""), hWnd, IDW_STATUS);

	SetClientRect(hWnd, TILE_WIDTH * MAPSIZE, TILE_HEIGHT * MAPSIZE);

	SetTimer(hWnd, 1, 10, NULL);
	SendMessage(hWnd, WM_TIMER, 1, 0);
	return 0;
}

LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam){
	KillTimer(hWnd, 1);
	if(hClientBitmap){DeleteObject(hClientBitmap);}
	if(pData){free(pData);}
	PostQuitMessage(0);
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

		case ID_SYS_RESIZE:
			return 0;

		case ID_SYS_EXIT:
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
	if(hClientBitmap){
		DrawBitmap(hdc, 0,0, hClientBitmap);
	}
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam){
	switch(wParam){
		case 1:
			{

			}
			break;
	}
	return 0;
}

void* LoadBmp(BITMAPINFOHEADER* ih){
	void *buf = NULL;
	TCHAR lpstrFile[MAX_PATH] = TEXT("");
	TCHAR FileName[MAX_PATH];
	TCHAR InitDir[MAX_PATH];
	TCHAR *path[MAX_PATH];
	TCHAR *pt = NULL;
	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = lpstrFile;
	ofn.lpstrFilter = TEXT("모든 파일(*.*)\0*.*\0비트맵 파일(*.bmp)\0*.bmp\0\0");
	ofn.lpstrTitle= TEXT("비트맵 파일을 선택하세요");
	ofn.lpstrDefExt = TEXT("txt");
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.hwndOwner = NULL;

	GetWindowsDirectory(InitDir, MAX_PATH);
	ofn.lpstrInitialDir = InitDir;

	if(GetOpenFileName(&ofn) != 0)
	{
		if(wcscmp(lpstrFile + ofn.nFileExtension, TEXT("bmp")) == 0)
		{
			HANDLE hFile = CreateFile(lpstrFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if(hFile != INVALID_HANDLE_VALUE)
			{
				DWORD dwRead;
				SetFilePointer(hFile, sizeof(BITMAPFILEHEADER), NULL, FILE_BEGIN);
				if(ReadFile(hFile, ih, sizeof(BITMAPINFOHEADER), &dwRead, NULL))
				{
					if(ih->biSizeImage == 0)
					{
						ih->biSizeImage = (((ih->biBitCount * ih->biWidth + 31) / 32) * 4 * ih->biHeight);
					}

					buf = malloc(ih->biSizeImage);
					if(ReadFile(hFile, buf, ih->biSizeImage, &dwRead, NULL))
					{
						CloseHandle(hFile);
						return buf;
					}else{
						MessageBox(NULL, TEXT("Failed to read bmp file data"), TEXT("Error"), MB_OK);
					}
				}else{
					MessageBox(NULL, TEXT("Failed to ReadFile"), TEXT("Error"), MB_OK);
				}

				CloseHandle(hFile);
			}else{
				MessageBox(NULL, TEXT("cannot open this file"), TEXT("Error"), MB_OK);
			}
		}else{
			MessageBox(NULL, TEXT("those not bmp file"), TEXT("Warning"), MB_OK);
		}
	}

	return NULL;
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

void SetMapSize(){
	/* 대화상자를 생성하여 화면 설정을 마칩니다. */
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


