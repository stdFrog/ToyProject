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

	- 2024.09.25(1)
	화면을 그릴 주체가 WM_PAINT이므로 DrawCalendar 함수를 수정했으며, WM_PAINT에서 더블 버퍼링을 적용한다.
	마우스 트래커 비스무리한 흉내를 내기 위해 동그라미 모양이 마우스 포인터를 따라다니도록 만들었다.
	
	- 2024.09.25(2)
	디자인 변경 후 함수 로직을 수정했다.
	각 일자에 데이터 덩어리를 저장하고자 한다.
	일자나 요일은 신경쓸 필요없으며 단순히 셀을 기준으로 데이터를 저장한다.
	단, 유효 일자가 아닌 경우는 무시한다.

	리스트 컨트롤 또는 리스트 확장 컨트롤을 이용하면 더 쉽게 달력 구조를 만들고 데이터를 추가 하는 것도 가능하다.
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

RECT g_crt;
HBITMAP hBitmap;
SYSTEMTIME Today;
int Year, Month;

TCHAR *Days[] ={
	TEXT("공백"),
	TEXT("일요일"),
	TEXT("월요일"),
	TEXT("화요일"),
	TEXT("수요일"),
	TEXT("목요일"),
	TEXT("금요일"),
	TEXT("토요일")
};

int DayOfTheWeek(int Y, int M, int D);
int LastDateOfTheMonth(int Y, int M);
int GetLineCount(int Day, int LastDate);

void GetValidRect(LPRECT rt);
void GetInvalidRect(LPRECT rt);

void DrawCalendar(HWND hWnd, HDC hdc);
void DrawBitmap(HDC hdc, int x, int y);

void SetTrackEvent(HWND hWnd);

LRESULT CALLBACK CalendarProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static BOOL bHover;
	PAINTSTRUCT ps;
	HDC hDC, hMemDC;
	HGDIOBJ hOld;
	BITMAP bmp;

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
				GetClientRect(hWnd, &g_crt);
				hBitmap = CreateCompatibleBitmap(hDC, g_crt.right, g_crt.bottom);
			}

			hOld = SelectObject(hMemDC, hBitmap);
			FillRect(hMemDC, &g_crt, GetSysColorBrush(COLOR_WINDOW));

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

		case WM_LBUTTONDOWN:
			{
				POINT cpt;
				cpt.x = lParam & 0xFFFF;
				cpt.y = lParam >> 16;

				int RowGap = crt.bottom / Line;
				int CellGap = crt.right / (sizeof(Days) / sizeof(Days[0]));

				RECT vrt, ivrt, prt;
				GetValidRect(&trt);
				GetInvalidRect(&ivrt);

				SubtractRect(&prt, &vrt, &ivrt);
				if(PtInRect(&prt, cpt)){
					/* 데이터 불러오기, 읽기, 에디트 및 리스트에 출력 */
				}
				
			}
			return 0;

		case WM_LBUTTONUP:
			/* 
			   마우스 버튼을 누른 상태로 영역 밖으로 이동한 경우를 고려한다면 LBUTTONUP에 로직 작성
			*/
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

int GetLineCount(int Day, int LastDate){
	if(Day == 0 && LastDate == 28){ return 4; }
	if(Day >= 5 && LastDate == 31){ return 6; }
	return 5;
}

void GetValidRect(LPRECT rt){
	int DofW = DayOfTheWeek(Year, Month, 1);
	int Last = LastDateOfTheMonth(Year, Month);
	int Line = GetLineCount(DofW, Last) + 2;

	int RowGap = g_crt.bottom / Line;

	*rt = {
		g_crt.left + 1,
		g_crt.top + RowGap * 2 + 1,
		g_crt.right - 1,
		g_crt.bottom - 1
	};
}

void GetInvalidRect(LPRECT rt){
	RECT rt, cprt;
	GetValidRect(&rt);

	CopyRect(&cprt, &rt);
	int CellGap = g_crt.right / (sizeof(Days) / sizeof(Days[0]));

	cprt.right -= CellGap;
	SubtractRect(rt, &rt, &cprt);
}

void DrawCalendar(HWND hWnd, HDC hdc){
	if(hBitmap == NULL){return;}

	RECT yrt, drt;
	HBRUSH hBrush, hOldBrush;
	HFONT hFont, hOldFont;
	HPEN hPen, hOldPen;

	SIZE sz;
	TEXTMETRIC tm;

	int DofW = DayOfTheWeek(Year, Month, 1);
	int Last = LastDateOfTheMonth(Year, Month);
	int Line = GetLineCount(DofW, Last) + 2;			// 년,월, 요일 표기

	int RowGap = g_crt.bottom / Line;
	int CellGap = g_crt.right / (sizeof(Days) / sizeof(Days[0]));

	SetRect(&yrt, 0, 0, g_crt.right, RowGap);
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
	SetTextAlign(hdc, TA_CENTER);
	SetTextColor(hdc, RGB(255, 255, 255));

	TCHAR buf[0x80];
	wsprintf(buf, TEXT("%d년 %d월"), Year, Month);
	TextOut(hdc, g_crt.right * 0.5f, 5, buf, lstrlen(buf));

	SetTextAlign(hdc, TA_RIGHT);
	SetTextColor(hdc, RGB(0,0,0));
	for(int i=1; i<sizeof(Days)/sizeof(Days[0]); i++){
		TextOut(hdc, drt.left + i * CellGap, drt.top, Days[i], lstrlen(Days[i]));
	}

	RECT trt;
	GetValidRect(&trt);

	hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
	Rectangle(hdc, trt.left, trt.top, trt.right, trt.bottom);
	SelectObject(hdc, hOldBrush);

	GetTextMetrics(hdc, &tm);

	int x = trt.left;
	int y = trt.top;
	int CharWidth = tm.tmAveCharWidth;
	int CharHeight = tm.tmHeight;

	/* i == day */
	for(int i=1; i<=Last; i++){
		int D = DofW + 1;
		int LineSpace = RowGap * 0.5f - CharHeight;

		if(i == Today.wDay) {

			hPen = CreatePen(PS_SOLID,2,RGB(0,0,0));
			hOldPen = (HPEN)SelectObject(hdc, hPen);
			hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

			Rectangle(
					hdc,
					x + (D * CellGap) - (CharWidth * 3),
					y - 2,
					x + (D * CellGap) + CharWidth,
					y + CharHeight + 2
			);

			DeleteObject(SelectObject(hdc, hOldPen));
			SelectObject(hdc, hOldBrush);
		}

		wsprintf(buf, TEXT("%d"), i);

		if(DofW == 0){ SetTextColor(hdc, RGB(255, 0,0)); }
		else{ SetTextColor(hdc, RGB(128, 128, 128)); }

		TextOut(hdc, x + (D * CellGap), y + LineSpace, buf, lstrlen(buf));

		/* 1픽셀만큼 늘려서 더 진하게 표시 */
		if(DofW == 0){ TextOut(hdc, x + (D * CellGap) +1, y + LineSpace, buf, lstrlen(buf)); }

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
	EventTrack.dwFlags = TME_LEAVE;
	EventTrack.hwndTrack = hWnd;
	EventTrack.dwHoverTime = 10;

	TrackMouseEvent(&EventTrack);
}

