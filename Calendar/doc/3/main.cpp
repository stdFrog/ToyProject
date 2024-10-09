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

int Table[] = {
	0,
	31, 28, 31, 30,
	31, 30, 31, 31,
	30, 31, 30, 31
};

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

RECT g_crt;
HBITMAP hBitmap;
SYSTEMTIME Today;
int Year, Month;

int DayOfTheWeek(int Y, int M, int D);
int LastDateOfTheMonth(int Y, int M);
int GetLineCount(int Day, int LastDate);

void GetValidRect(LPRECT rt);
void GetInvalidRect(LPRECT rt);

void DrawCalendar(HWND hWnd, HDC hdc);
void DrawBitmap(HDC hdc, int x, int y);

void SetTrackEvent(HWND hWnd);

HWND hList,hEdit, hCalendar, hTitleBtn, hDayBtn;

int LW, CH, CR, GAP = 3;
enum SPLIT { NONE, VERT, HORZ };
SPLIT SP;

SPLIT GetSplit(HWND hWnd, POINT pt);

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

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

	- 2024.09.26
	캘린더 윈도우를 분할했다.
	버튼 클래스와 리스트 박스 클래스로부터 윈도우를 만들어 각각 기능을 추가하고자 한다.
	리스트 박스는 서브클래싱을, 버튼은 커스텀 버튼을 사용할 수 있는데 기능을 확장할 필요가 있는지 생각해봐야 한다.

	1. 버튼은 단순히 텍스트만으로 이루어져도 상관없다.
		- 년, 월만 표현하면 되는데 추가 기능을 넣는다고 하면 지정한 달(또는 년도)의 캘린더를 보여주는 정도가 될 것이다.
	2. 리스트 박스는 캘린더를 표현하며, 일자별로 사용자가 입력한 데이터를 담고자 한다.
		- 리스트 아이템을 이용하면 비교적 쉽다. 단, 일정 크기 이상의 데이터를 담을 수 없다.
	3. 캘린더 좌측의 또 다른 리스트 박스는 해당 일정의 간략화된 제목이나 내용을 표기하는 용도로 쓸 것이다.
	4. 디자인은 깔끔하게 만들거나 기본 형태만 갖추도록 한다.

	당장의 설계 목표는 위와 같으므로 서브클래싱은 필요하지 않다.
	단, 이 이상의 기능을 추가할 수 없다.

	예로, 아이콘과 같이 작은 이미지를 추가한다거나, 드래그 앤 드롭으로 아이템을 옮기는 등의 동작은 불가하다.

	현재 코드를 첫 번째 분기로 둔다.
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	RECT crt, wrt;
	POINT pt;

	switch(iMessage){
		case WM_CREATE:
			GetLocalTime(&Today);
			Year = Today.wYear;
			Month = Today.wMonth;

			hList = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,0,0,0, hWnd, (HMENU)0, GetModuleHandle(NULL), NULL);
			hEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE, 0,0,0,0, hWnd, (HMENU)1, GetModuleHandle(NULL), NULL);
			hCalendar = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | WS_BORDER | LBS_MULTICOLUMN | LBS_NOINTEGRALHEIGHT, 0,0,0,0, hWnd, (HMENU)2, GetModuleHandle(NULL), NULL);

			hTitleBtn = CreateWindow(TEXT("button"), NULL, WS_CHILD | WS_VISIBLE, 0,0,0,0, hWnd, (HMENU)3, GetModuleHandle(NULL), NULL);
			hDayBtn = CreateWindow(TEXT("button"), NULL, WS_CHILD | WS_VISIBLE, 0,0,0,0, hWnd, (HMENU)4, GetModuleHandle(NULL), NULL);

			LW = 200;
			CR = 55;
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case 2:
					switch(HIWORD(wParam)){
						default:
							break;
					}
			}
			return 0;

		case WM_SIZE:
			if(wParam != SIZE_MINIMIZED){
				GetClientRect(hWnd, &crt);
				GetClientRect(hWnd, &g_crt);
				CH = crt.bottom * CR * 0.01f;

				int DofW = DayOfTheWeek(Year, Month, 1);
				int Last = LastDateOfTheMonth(Year, Month);
				int Line = GetLineCount(DofW, Last) + 2;

				int th, dh, tw, dw;
				th = dh = CH /Line;
				tw = dw = crt.right - LW;

				MoveWindow(hList, 0, 0, LW - GAP, crt.bottom, TRUE);
				MoveWindow(hEdit, LW, CH, crt.right - LW, crt.bottom, TRUE);
				MoveWindow(hTitleBtn, LW, 0, tw, th, TRUE);
				MoveWindow(hDayBtn, LW, th, dw, dh, TRUE);
				MoveWindow(hCalendar, LW, th + dh, crt.right - LW, CH - (th + dh + GAP), TRUE);

				GetClientRect(hCalendar, &crt);
				int Width = (crt.right - crt.left) / (sizeof(Days)/sizeof(Days[0]));
				SendMessage(hCalendar, LB_SETCOLUMNWIDTH, (WPARAM)Width, (LPARAM)0);
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

void SetTrackEvent(HWND hWnd){
	TRACKMOUSEEVENT EventTrack;

	EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
	EventTrack.dwFlags = TME_LEAVE;
	EventTrack.hwndTrack = hWnd;
	EventTrack.dwHoverTime = 10;

	TrackMouseEvent(&EventTrack);
}
