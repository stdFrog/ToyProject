#define DEBUG
#include "resource.h"

const int WIDTH = 600;
const int HEIGHT = GetSystemMetrics(SM_CYSCREEN);
const int cMonitors = GetSystemMetrics(SM_CMONITORS);
int nMonitor;

enum { MOUSE, IMAGE, CAPTION, CLASSNAME, LAST_COUNT };

RECT crt;
HFONT hMainFont;
HBITMAP hMainBmp;

INT CharHeight;
TEXTMETRIC Metric;
HWND hList, hEdit;

LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam){
	HDC hDC = GetDC(hWnd);
	GetTextMetrics(hDC, &Metric);
	ReleaseDC(hWnd, hDC);

	CharHeight = Metric.tmHeight;
	hMainFont = CreateFont(CharHeight, 0,0,0,0,0,0,0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_ROMAN, TEXT("Times New Roman"));

	SetWindowPos(hWnd, NULL, 0,0, WIDTH, HEIGHT, SWP_NOZORDER | SWP_NOMOVE);
	GetClientRect(hWnd, &crt);

	hList = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_CLIPSIBLINGS, 0, LAST_COUNT * CharHeight, WIDTH, HEIGHT, hWnd, (HMENU)IDW_LISTBOX, GetModuleHandle(NULL), NULL);
	hEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL | WS_CLIPSIBLINGS, crt.right - crt.left >> 1, LAST_COUNT * CharHeight, WIDTH, HEIGHT, hWnd, (HMENU)IDW_EDIT, GetModuleHandle(NULL), NULL);

	SetTimer(hWnd, 1, 10, NULL);
	return 0;
}

LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam){
	KillTimer(hWnd, 1);
	if(hMainFont){DeleteObject(hMainFont);}
	if(hMainBmp){DeleteObject(hMainBmp);}
	PostQuitMessage(0);
	return 0;
}

LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam){
	if(wParam != SIZE_MINIMIZED){
		if(hMainBmp){
			DeleteObject(hMainBmp);
			hMainBmp = NULL;
		}
	}

	MoveWindow(hList, 0, LAST_COUNT * CharHeight, LOWORD(lParam), HIWORD(lParam) - 50, TRUE);

	return 0;
}

LRESULT OnMove(HWND hWnd, WPARAM wParam, LPARAM lParam){
	/*
		TODO : 수정 필요
		RECT rtMultipleMonitor, wrt;

		EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&rtMultipleMonitor);
		GetWindowRect(hWnd, &wrt);
	*/
	return 0;
}

LRESULT OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam){
	INT i;
	TCHAR str[MAX_PATH];

	switch(LOWORD(wParam)){
		case IDW_LISTBOX:
			switch(HIWORD(wParam)){
				case LBN_SELCHANGE:
					/*
					i = SendMessage(hList, LB_GETCURSEL, 0, 0);
					SendMessage(hList, LB_GETTEXT, i, (LPARAM)str);
					SetWindowText(hWnd, str);
					*/
					break;
			}
			break;
	}

	return 0;
}

LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){
	EnumWindows(EnumProc, (LPARAM)NULL);
	return 0;
}

LRESULT OnRButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){
	SendMessage(hList, LB_RESETCONTENT, 0,0);
	return 0;
}

LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	DrawBitmap(hdc, 0,0, hMainBmp);
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam){
	TCHAR DisplayInfo[LAST_COUNT][MAX_PATH];
	COLORREF Color;

	switch(wParam){
		case 1:
			HDC hdc = GetDC(hWnd);
			HDC hMemDC = CreateCompatibleDC(hdc);

			GetClientRect(hWnd, &crt);
			if(hMainBmp == NULL){
				hMainBmp = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
			}
			HGDIOBJ hOld = SelectObject(hMemDC, hMainBmp);
			FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));

			/*
				TODO : 그리기, 조사
			 */
			{
				POINT pt;
				GetCursorPos(&pt);

				HWND hWndPoint = WindowFromPoint(pt);
				GetWindowText(hWndPoint, DisplayInfo[CAPTION], MAX_PATH);
				GetClassName(hWndPoint, DisplayInfo[CLASSNAME], MAX_PATH);

				if(hWndPoint == hWnd){
					ScreenToClient(hWnd, &pt);
				}

				Color = GetPixel(GetDC(hWndPoint), pt.x, pt.y);

				INT R = (BYTE)(Color);
				INT G = ((BYTE)(((WORD)(Color)) >> 8));
				INT B = ((BYTE)((Color)>>16));

				SetBkMode(hMemDC, TRANSPARENT);
				HFONT hOldFont = (HFONT)SelectObject(hMemDC, hMainFont);

				wsprintf(DisplayInfo[MOUSE], TEXT("Mouse(%d, %d)"), pt.x, pt.y);
				wsprintf(DisplayInfo[IMAGE], TEXT("Color(R: %d, G: %d, B: %d)"), R, G, B);

				for(int i=0; i<LAST_COUNT; i++){
					TextOut(hMemDC, 0, crt.top + CharHeight * i, DisplayInfo[i], lstrlen(DisplayInfo[i]));
				}
				
				MoveToEx(hMemDC, 0, LAST_COUNT * CharHeight, NULL);
				LineTo(hMemDC, crt.right, LAST_COUNT * CharHeight);

				AllInfoFromPoint(hMemDC, pt);

				SelectObject(hMemDC, hOldFont);
			}

			{
				
			}

			SelectObject(hMemDC, hOld);
			DeleteDC(hMemDC);
			ReleaseDC(hWnd, hdc);
			break;
	}

	InvalidateRect(hWnd, NULL, FALSE);
	return 0;
}

void DrawBitmap(HDC hDC, LONG X, LONG Y, HBITMAP hBitmap){
	if(hBitmap == NULL){return;}

	HDC hMemDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	BitBlt(hDC, 0,0, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
}

void AllInfoFromPoint(HDC hDC, POINT pt){
	// EnumWindows
}

BOOL CALLBACK EnumProc(HWND hSearch, LPARAM lParam){
	RECT wrt;
	TCHAR Caption[MAX_PATH];
	TCHAR WndInfo[MAX_PATH];

	GetWindowText(hSearch, Caption, MAX_PATH);
	GetWindowRect(hSearch, &wrt);

	if(lstrcmp(Caption, TEXT("")) == 0 || lstrcmp(Caption, NULL) == 0){
		// wsprintf(WndInfo, TEXT("ProcessInfo = %s"), PEInfo);
		wsprintf(WndInfo, TEXT("Handle = %x"), hSearch);
	}else{
		wsprintf(WndInfo, TEXT("Caption = %s"), Caption);
	}

	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)WndInfo);
	return TRUE;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT lprcMonitor, LPARAM dwData){
	/*
		TODO : 수정 필요
	 */
	MONITORINFOEX mi;
	
	// bFind = TRUE;
	mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &mi);

	if((mi.dwFlags & MONITORINFOF_PRIMARY) == 0){
		(*(LPRECT)dwData) = *lprcMonitor;
		nMonitor = (nMonitor + 1) % cMonitors;
		return FALSE;
	}else{
		if(((nMonitor + 1) % cMonitors) == 0){
			(*(LPRECT)dwData) = *lprcMonitor;
			nMonitor = (nMonitor + 1) % cMonitors;
			return FALSE;
		}
		return TRUE;
	}
}
