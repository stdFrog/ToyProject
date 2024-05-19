#define DEBUG
#include "resource.h"

const int WIDTH = 300;
const int HEIGHT = 300;
int cMonitors, nMonitor, LT;

enum { MOUSE, IMAGE, CAPTION, CLASSNAME, LAST_COUNT };

HDC g_PointDC;
RECT g_crt, g_wrt;
HFONT hMainFont;
HBITMAP hMainBmp;

INT CharHeight;
TEXTMETRIC Metric;
HWND hList, hEdit;
BOOL bCapture;
RECT *rtMultipleMonitor;

/* Main Procedure(MP) */
LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam){
	cMonitors = GetSystemMetrics(SM_CMONITORS);
	rtMultipleMonitor = (RECT*)malloc(sizeof(RECT) * cMonitors);
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)rtMultipleMonitor);

	HDC hDC = GetDC(hWnd);
	GetTextMetrics(hDC, &Metric);
	ReleaseDC(hWnd, hDC);

	CharHeight = Metric.tmHeight;
	hMainFont = CreateFont(CharHeight, 0,0,0,0,0,0,0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_ROMAN, TEXT("Times New Roman"));

	SetClientRect(hWnd, WIDTH, HEIGHT);
	hList = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | WS_CLIPSIBLINGS, 0, 0, 0, 0, hWnd, (HMENU)IDW_LISTBOX, GetModuleHandle(NULL), NULL);
	hEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL | WS_CLIPSIBLINGS, 0,0,0,0, hWnd, (HMENU)IDW_EDIT, GetModuleHandle(NULL), NULL);

	SetTimer(hWnd, 1, 10, NULL);
	return 0;
}

LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam){
	KillTimer(hWnd, 1);
	if(hMainFont){DeleteObject(hMainFont);}
	if(hMainBmp){DeleteObject(hMainBmp);}
	if(rtMultipleMonitor){free(rtMultipleMonitor);}
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

	LT = LAST_COUNT * CharHeight;
	GetClientRect(hWnd, &g_crt);
	SetChildRect(hEdit, 0, LT, LOWORD(lParam), (HIWORD(lParam) - LT) / 2);
	SetChildRect(hList, 0, (HIWORD(lParam) - LT) / 2 + GetSystemMetrics(SM_CYHSCROLL) * 4, LOWORD(lParam), (HIWORD(lParam) - LT) / 2);

	return 0;
}

LRESULT OnMove(HWND hWnd, WPARAM wParam, LPARAM lParam){
	GetWindowRect(hWnd, &g_wrt);
	return 0;
}

LRESULT OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam){
	INT i;
	TCHAR Caption[MAX_PATH];
	TCHAR Info[MAX_PATH];

	switch(LOWORD(wParam)){
		case IDW_LISTBOX:
			switch(HIWORD(wParam)){
				case LBN_SELCHANGE:
					i = SendMessage(hList, LB_GETCURSEL, 0, 0);
					SendMessage(hList, LB_GETTEXT, i, (LPARAM)Caption);
					GetInfo(Caption, Info);
					SetWindowText(hEdit, Info);
					break;
			}
			break;

		case ID_ABOUT:
			MessageBox(hWnd, TEXT("이 프로그램은 윈도우 환경에서 프로그램을 개발하는 작성자가 화를 참지 못하고 제작한 윈도우 관리 도구입니다.\r\n\r\n어떠한 시스템 변화도 허용하지 않으나 추후 작성자의 마음대로 레지스트리 기능을 추가할 수 있습니다.\r\n\r\n메뉴의 캡쳐 기능을 이용하여 이미지를 복사할 수 있으며 저장하는 등의 기능은 지원하지 않습니다.\r\n\r\n하위 프로세스를 검색하거나 이미지의 픽셀 값 따위를 구하기 위해 만들어진 프로그램이며 추후 업데이트 될 수 있습니다.\r\n"), TEXT("프로그램 소개"), MB_OK);
			break;

		case ID_CAPTURE:
			CreateWindow(CHILD_CLASS_NAME, NULL, WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, hWnd, (HMENU)IDW_CAPTURE, GetModuleHandle(NULL), NULL);
			break;

		case ID_EXIT:
			DestroyWindow(hWnd);
			break;
	}

	return 0;
}

LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){
	static LONG X = (LONG)(short)(LOWORD(lParam));
	static LONG Y = (LONG)(short)(HIWORD(lParam));

	if(0 <= X && X <= g_crt.right - g_crt.left && 0 <= Y && Y <= LT){
		SendMessage(hList, LB_RESETCONTENT, 0,0);
		EnumWindows(EnumProc, (LPARAM)NULL);
	}
	
	return 0;
}

LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam){
	ReleaseCapture();
	bCapture = FALSE;
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
	switch(wParam){
		case 1:
			HDC hdc = GetDC(hWnd);
			HDC hMemDC = CreateCompatibleDC(hdc);

			if(hMainBmp == NULL){
				GetClientRect(hWnd, &g_crt);
				hMainBmp = CreateCompatibleBitmap(hdc, g_crt.right, g_crt.bottom);
			}
			HGDIOBJ hOld = SelectObject(hMemDC, hMainBmp);
			FillRect(hMemDC, &g_crt, GetSysColorBrush(COLOR_WINDOW));

			InfoFromPoint(hWnd, hMemDC);

			SelectObject(hMemDC, hOld);
			DeleteDC(hMemDC);
			ReleaseDC(hWnd, hdc);
			break;
	}

	InvalidateRect(hWnd, NULL, FALSE);
	return 0;
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

void SetChildRect(HWND hWnd, int X, int Y, int Width, int Height){
	RECT crt;
	HWND hParent = GetParent(hWnd);

	SetRect(&crt, X, Y, Width, Height);
	SetWindowPos(hWnd, NULL, crt.left, crt.top, crt.right, crt.bottom, SWP_NOZORDER);
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

void InfoFromPoint(HWND hWnd, HDC hDC){
	TCHAR DisplayInfo[LAST_COUNT][MAX_PATH];
	COLORREF Color;

	POINT pt;
	GetCursorPos(&pt);

	HWND hWndPoint = WindowFromPoint(pt);
	GetWindowText(hWndPoint, DisplayInfo[CAPTION], MAX_PATH);
	GetClassName(hWndPoint, DisplayInfo[CLASSNAME], MAX_PATH);
	g_PointDC = GetWindowDC(hWndPoint);

	if(hWndPoint == hWnd){
		ScreenToClient(hWnd, &pt);
	}

	Color = GetPixel(GetDC(hWndPoint), pt.x, pt.y);

	INT R = (BYTE)(Color);
	INT G = ((BYTE)(((WORD)(Color)) >> 8));
	INT B = ((BYTE)((Color)>>16));

	SetBkMode(hDC, TRANSPARENT);
	HFONT hOldFont = (HFONT)SelectObject(hDC, hMainFont);

	wsprintf(DisplayInfo[MOUSE], TEXT("Mouse(%d, %d)"), pt.x, pt.y);
	wsprintf(DisplayInfo[IMAGE], TEXT("Color(R: %d, G: %d, B: %d)"), R, G, B);

	for(int i=0; i<LAST_COUNT; i++){
		TextOut(hDC, 0, g_crt.top + CharHeight * i, DisplayInfo[i], lstrlen(DisplayInfo[i]));
	}
	
	MoveToEx(hDC, 0, LAST_COUNT * CharHeight, NULL);
	LineTo(hDC, g_crt.right, LAST_COUNT * CharHeight);

	SelectObject(hDC, hOldFont);
}

void GetInfo(TCHAR* Caption, TCHAR* Info){
	memset(Info, 0, sizeof(Info));

	RECT wrt;
	DWORD dwProcessID, dwCountOfBytes;
	TCHAR WndInfo[MAX_PATH];
	TCHAR ClassName[MAX_PATH];
	TCHAR ProcessInfo[MAX_PATH];
	HWND WndSel = FindWindow(NULL, Caption);

	GetWindowRect(WndSel, &wrt);
	GetClassName(WndSel, ClassName, MAX_PATH);
	GetWindowThreadProcessId(WndSel, &dwProcessID);

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessID);
	if(hProcess){
		HMODULE hModule;
		if(EnumProcessModules(hProcess, &hModule, sizeof(hModule), &dwCountOfBytes)){
			GetModuleFileNameEx(hProcess, hModule, ProcessInfo, sizeof(ProcessInfo));
		}else{
			wsprintf(ProcessInfo, TEXT("Enum Module Error : %d"), GetLastError());
		}
		CloseHandle(hProcess);
	}else{
		wsprintf(ProcessInfo, TEXT("Open Error : %d"), GetLastError());
	}

	wsprintf(WndInfo, TEXT("Caption = %s\r\n")
				TEXT("ClassName = %s\r\n")
				TEXT("Window(%d,%d,%d,%d)\r\n")
				TEXT("W(%d),H(%d)\r\n")
				TEXT("ProcessID(%5d)\r\n")
				TEXT("Path = %s\r\n"), Caption, ClassName, wrt.left, wrt.top, wrt.right, wrt.bottom, wrt.right - wrt.left, wrt.bottom - wrt.top, dwProcessID, ProcessInfo);
	lstrcpy(Info, WndInfo);
}

BOOL CALLBACK EnumProc(HWND hSearch, LPARAM lParam){
	TCHAR Caption[MAX_PATH];
	GetWindowText(hSearch, Caption, MAX_PATH);

	if(lstrcmp(Caption, TEXT("")) == 0 || lstrcmp(Caption, NULL) == 0 || lstrcmp(Caption, TEXT("Default IME")) == 0 || lstrcmp(Caption, TEXT("MSCTFIME UI")) == 0){
		return TRUE;
	}

	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)Caption);
	return TRUE;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT lprcMonitor, LPARAM dwData){
	MONITORINFOEX mi;
	
	mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &mi);

	if(nMonitor < cMonitors){
		(*((LPRECT)dwData + nMonitor)) = *lprcMonitor;
		nMonitor++;
		return TRUE;
	}

	return FALSE;
}

/* Child Procedure(CP) */
LRESULT OnChildCreate(HWND hWnd, WPARAM wParam, LPARAM lParam){
	int CurrentMonitor = GetCurrentMonitor();
	HDC hdc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

	return 0;
}

LRESULT OnChildDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam){
	PostQuitMessage(0);
	return 0;
}

LRESULT OnChildLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){

	return 0;
}

LRESULT OnChildLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam){

	return 0;
}

LRESULT OnChildPaint(HWND hWnd, WPARAM wParam, LPARAM lParam){

	return 0;
}

LRESULT OnChildTimer(HWND hWnd, WPARAM wParam, LPARAM lParam){

	return 0;
}

int GetCurrentMonitor(){
	return 0;
}
