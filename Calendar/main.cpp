#define UNICODE
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#define CLASS_NAME TEXT("Calendar")

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wconversion-null"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) < (b)) ? (b) : (a))
#define CLAMP(a,b,c) MIN(MAX(a,c), b)
#define ABS(a) (((a) < 0) ? (-a) : (a))
#define FLOOR(a) (((a) < 0) ? ((a) - ((a)-((int)a))) - 1.f : ((a)-((a)-((int)a))))

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CalendarProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

/*
	- 2024.09.22,
	베이스 윈도우 제작, 창 분할, 캘린더 그리기 추가

	- 2024.09.25
	화면을 그릴 주체가 WM_PAINT이므로 DrawCalendar 함수를 수정했으며, WM_PAINT에서 더블 버퍼링을 적용한다.
	마우스 트래커 비스무리한 흉내를 내기 위해 동그라미 모양이 마우스 포인터를 따라다니도록 만들었다.
*/

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	WNDCLASSEX wcex = {
		sizeof(wcex),
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_BTNFACE+1),
		NULL,
		CLASS_NAME,
		NULL
	};
	RegisterClassEx(&wcex);

	wcex.lpfnWndProc = CalendarProc;
	wcex.lpszClassName = TEXT("CalendarPopup");
	wcex.hbrBackground = NULL;
	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				CLASS_NAME,
			CLASS_NAME,
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				NULL,
				(HMENU)NULL,
				hInst,
				NULL
			);

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	while(GetMessage(&msg, nullptr, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}


HWND hList,hEdit, hCalendar;
int LW, CH, CR, GAP = 3;
enum SPLIT { NONE, VERT, HORZ };
SPLIT SP;
SPLIT GetSplit(HWND hWnd, POINT pt);

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	RECT crt, wrt;
	POINT pt;

	switch(iMessage){
		case WM_CREATE:
			hList = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,0,0,0, hWnd, (HMENU)0, GetModuleHandle(NULL), NULL);
			hEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE, 0,0,0,0, hWnd, (HMENU)1, GetModuleHandle(NULL), NULL);
			hCalendar = CreateWindow(TEXT("CalendarPopup"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,0,0,0, hWnd, (HMENU)2, GetModuleHandle(NULL), NULL);
			LW = 200;
			CR = 55;
			return 0;

	case WM_SIZE:
		if(wParam != SIZE_MINIMIZED){
			GetClientRect(hWnd, &crt);
			CH = crt.bottom * CR * 0.01f;
			MoveWindow(hList, 0,0, LW - GAP, crt.bottom, TRUE);
			MoveWindow(hCalendar, LW, 0, crt.right - LW, CH - GAP, TRUE);
			MoveWindow(hEdit, LW, CH, crt.right - LW, crt.bottom, TRUE);
		}
		return 0;

		case WM_SETCURSOR:
			if(LOWORD(lParam) == HTCLIENT){
				GetCursorPos(&pt);
				ScreenToClient(hWnd, &pt);

				{
					SPLIT temp = GetSplit(hWnd, pt);
					if(temp == VERT){
						SetCursor(LoadCursor(NULL, IDC_SIZEWE));
						return TRUE;
					}

					if(temp == HORZ){
						SetCursor(LoadCursor(NULL, IDC_SIZENS));
						return TRUE;
					}
				}
			}
			break;

		case WM_LBUTTONDOWN:
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			SP = GetSplit(hWnd, pt);
			if(SP != NONE){
				SetCapture(hWnd);
			}
			return 0;

		case WM_MOUSEMOVE:
			switch(SP){
				case VERT:
					LW = CLAMP(50, 500, (int)(short)LOWORD(lParam));
					SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 0);
					break;

				case HORZ:
					GetClientRect(hWnd, &crt);
					CH = (int)(short)HIWORD(lParam);
					CR = CLAMP(20, 80, CH * 100.f / crt.bottom);
					SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 0);
					break;
			}
			return 0;

		case WM_LBUTTONUP:
			SP = NONE;
			ReleaseCapture();
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

SPLIT GetSplit(HWND hWnd, POINT pt){
	RECT crt, wrt, vrt, hrt;
	int CH;

	GetClientRect(hWnd, &crt);
	CH = crt.bottom * CR * 0.01f;
	SetRect(&vrt, LW - GAP, 0, LW, crt.bottom);
	
	if(PtInRect(&vrt, pt)){
		return VERT;
	}

	SetRect(&hrt, LW, CH - GAP, crt.right, CH);
	if(PtInRect(&hrt, pt)){
		return HORZ;
	}

	return NONE;
}

HBITMAP hBitmap;
SYSTEMTIME Today;
int Year, Month;

int GetDigit(int number);
int DayOfTheWeek(int Y, int M, int D);
int LastDateOfTheMonth(int Y, int M);

void DrawCalendar(HWND hWnd, HDC hdc);
void DrawBitmap(HDC hdc, int x, int y);

void SetTrackEvent(HWND hWnd);

LRESULT CALLBACK CalendarProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static BOOL bHover;
	static RECT rt;
	PAINTSTRUCT ps;
	HDC hDC, hMemDC;
	HGDIOBJ hOld;
	BITMAP bmp;

	static int sx, sy;

	switch(iMessage){
		case WM_CREATE:
			GetLocalTime(&Today);
			Year = Today.wYear;
			Month = Today.wMonth;
			return 0;

		case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);
			hMemDC = CreateCompatibleDC(hDC);

			if(hBitmap == NULL){
				GetClientRect(hWnd, &rt);
				hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			}

			hOld = SelectObject(hMemDC, hBitmap);
			FillRect(hMemDC, &rt, GetSysColorBrush(COLOR_WINDOW));

			{
				POINT cpt;
				GetCursorPos(&cpt);
				ScreenToClient(hWnd, &cpt);

				HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, GetStockObject(NULL_BRUSH));
				Ellipse(hMemDC, cpt.x - 5, cpt.y - 5, cpt.x + 5, cpt.y + 5);
				SelectObject(hMemDC, hOldBrush);
			}

			DrawCalendar(hWnd, hMemDC);

			GetObject(hBitmap, sizeof(BITMAP), &bmp);
			BitBlt(hDC, 0,0, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

			SelectObject(hMemDC, hOld);
			DeleteDC(hMemDC);
			EndPaint(hWnd, &ps);
			return 0;

		case WM_MOUSEMOVE:
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;

		case WM_SIZE:
			if(wParam != SIZE_MINIMIZED){
				DeleteObject(hBitmap);
				hBitmap = NULL;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return 0;

		case WM_DESTROY:
			if(hBitmap){DeleteObject(hBitmap);}
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

int DayOfTheWeek(int Y, int M, int D){
	SYSTEMTIME st;
	FILETIME ft;

	memset(&st, 0, sizeof(st));
	st.wYear = Y;
	st.wMonth = M;
	st.wDay = D;

	SystemTimeToFileTime(&st, &ft);
	FileTimeToSystemTime(&ft, &st);
	return st.wDayOfWeek;
}

int LastDateOfTheMonth(int Y, int M){
	static int Table[] = {
		0,
		31, 28, 31, 30,
		31, 30, 31, 31,
		30, 31, 30, 31
	};

	int last;
	last = Table[M];
	if(M == 2 && ((Y % 4 == 0 && Y % 100 != 0) || Y % 400 == 0)){
		last = 29;
	}

	return last;
}

void DrawCalendar(HWND hWnd, HDC hdc){
	if(hBitmap == NULL){return;}

	static TCHAR *Days[] ={
		TEXT("공백"),
		TEXT("일요일"),
		TEXT("월요일"),
		TEXT("화요일"),
		TEXT("수요일"),
		TEXT("목요일"),
		TEXT("금요일"),
		TEXT("토요일")
	};

	RECT crt, yrt, drt;
	HBRUSH hBrush, hOldBrush;
	HFONT hFont, hOldFont;
	HPEN hPen, hOldPen;

	GetClientRect(hWnd, &crt);

	/* Halving */
	int Halv = 9;
	int RowGap = crt.bottom / Halv;
	int CellGap = crt.right / (sizeof(Days) / sizeof(Days[0]));

	SetRect(&yrt, 0, 0, crt.right, RowGap);
	hBrush = CreateSolidBrush(RGB(0,0,255));
	FillRect(hdc, &yrt, hBrush);
	DeleteObject(hBrush);

	SetRect(&drt, 0, yrt.bottom, yrt.right, yrt.bottom + RowGap);
	hBrush = CreateSolidBrush(RGB(240, 240, 240));
	FillRect(hdc, &drt, hBrush);
	DeleteObject(hBrush);

	hFont = CreateFont(
		20, 0, GM_COMPATIBLE, 0,
		FW_BOLD, 0,0,0,
		DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY,
		VARIABLE_PITCH | FF_MODERN,
		TEXT("consolas")
	);

	hOldFont = (HFONT)SelectObject(hdc, hFont);

	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT);
	SetTextColor(hdc, RGB(255, 255, 255));

	TCHAR buf[0x80];
	wsprintf(buf, TEXT("%d년 %d월"), Year, Month);

	SIZE sz;
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	GetTextExtentPoint32(hdc, buf, lstrlen(buf), &sz);
	TextOut(hdc, crt.right * 0.5f - sz.cx, 5, buf, lstrlen(buf));

	SetTextAlign(hdc, TA_RIGHT);
	SetTextColor(hdc, RGB(0,0,0));
	for(int i=1; i<=7; i++){
		TextOut(hdc, drt.left + i * CellGap, drt.top, Days[i], lstrlen(Days[i]));
	}

	/*
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	TextOut(hdc, crt.right * 0.5f - tm.tmAveCharWidth * lstrlen(buf), tm.tmHeight * 0.5f, buf, lstrlen(buf));
	*/

	int DofW = DayOfTheWeek(Year, Month, 1);
	int Last = LastDateOfTheMonth(Year, Month);
	
	int x = crt.left, y = RowGap * 2;
	int CellInternalLeading;

	for(int i=1; i<=Last; i++){
		 if(i == Today.wDay) {
			hPen = CreatePen(PS_SOLID,2,RGB(0,0,0));
			hOldPen = (HPEN)SelectObject(hdc, hPen);
			hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
			Rectangle(hdc, (x + (DofW+1) * CellGap) - (tm.tmAveCharWidth * 3), y-2, x + (DofW+1) * CellGap + tm.tmAveCharWidth, y + tm.tmHeight + 2);
			DeleteObject(SelectObject(hdc, hOldPen));
			SelectObject(hdc, hOldBrush);
		}

		/* i == day */
		wsprintf(buf, TEXT("%d"), i);

		if(DofW == 0){ SetTextColor(hdc, RGB(255, 0,0)); }
		else{ SetTextColor(hdc, RGB(128, 128, 128)); }

		TextOut(hdc, x + (DofW+1) * CellGap, y, buf, lstrlen(buf));
		if(DofW == 0){
			/* 1픽셀만큼 늘려서 더 진하게 표시 */
			TextOut(hdc, x + (DofW+1) * CellGap+1, y, buf, lstrlen(buf));
		}

		DofW++;
		if(DofW == 7){
			DofW = 0;
			y += RowGap;
		}
	}

	DeleteObject(SelectObject(hdc, hOldFont));
}

void SetTrackEvent(HWND hWnd){
	TRACKMOUSEEVENT EventTrack;

	EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
	EventTrack.dwFlags = TME_HOVER | TME_LEAVE;
	EventTrack.hwndTrack = hWnd;
	EventTrack.dwHoverTime = 10;

	TrackMouseEvent(&EventTrack);
}

#include <math.h>
int	GetDigit(int number){
	return FLOOR(log10(number)) + 1;
}
