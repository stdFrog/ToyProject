#define UNICODE
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>
#define CLASS_NAME TEXT("Calendar")

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) < (b)) ? (b) : (a))
#define CLAMP(a,b,c) MIN(MAX(a,b), c)

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow){
	WNDCLASSEX wcex = {
		sizeof(wcex),
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		NULL,
		CLASS_NAME,
		NULL
	};

	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindowEx(
				WS_EX_CLIENTEDGE | WS_EX_COMPOSITED,
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

/* Split Attrb */
CONST INT GAP = 5;
typedef enum tag_SplitState {NONE, VERT, HORZ} SPLIT;

/* 수정 필요 */
void GetCurrentSplit(HWND hWnd, SPLIT* Split, int iRatio);

/* Child Window Attrb */
CONST INT_PTR BTNCOUNT = 2;
CONST INT_PTR ID_LIST = 0, ID_CALENDAR = 1, ID_EDIT = 2;
CONST INT_PTR ID_BTN[BTNCOUNT] = {3, };

HWND hList, hCalendar, hEdit;
HWND hBtn[BTNCOUNT];

void CreateChildWindow(HWND hWnd);
void SetChildPosition(HWND hWnd, int ListWidth, int CalendarHeight, int iRatio);

/* Structure */
typedef struct tag_ButtonSize{
	int Width, Height;
}ButtonSize;

/* Utility */
void GetDate(int* Year, int* Month);
int DayOfTheWeek(int Year, int Month, int Day);
int LastDateOfTheMonth(int Year, int Month);
int GetLineCount(int Day = -1, int LastDate = -1);

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static SPLIT SplitState;
	static int iRatio, ListWidth, CalendarHeight;

	RECT crt;
	SPLIT TempSplit;
	int TempCalendarHeight;

	switch(iMessage){
		case WM_CREATE:
			iRatio = 60;
			ListWidth = 200;
			SplitState = NONE;
			CreateChildWindow(hWnd);
			return 0;

		case WM_LBUTTONDOWN:
			GetCurrentSplit(hWnd, &SplitState, iRatio);
			if(SplitState != NONE){
				SetCapture(hWnd);
			}
			return 0;

		case WM_LBUTTONUP:
			SplitState = NONE;
			ReleaseCapture();
			return 0;

		case WM_MOUSEMOVE:
			switch(SplitState){
				case VERT:
					ListWidth = CLAMP((int)(short)LOWORD(lParam), 100, 400);
					SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 0);
					break;

				case HORZ:
					CalendarHeight = (int)(short)HIWORD(lParam);
					GetClientRect(hWnd, &crt);
					iRatio = CalendarHeight * 100.f / crt.bottom;
					iRatio = CLAMP(iRatio, 30, 60);
					SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, 0);
					break;
			}
			return 0;

		case WM_SETCURSOR:
			if(LOWORD(lParam) == HTCLIENT){
				GetCurrentSplit(hWnd, &TempSplit, iRatio);
				if(TempSplit == VERT){
					SetCursor(LoadCursor(NULL, IDC_SIZEWE));
					return TRUE;
				}
				if(TempSplit == HORZ){
					SetCursor(LoadCursor(NULL, IDC_SIZENS));
					return TRUE;
				}
			}
			break;

		case WM_SIZE:
			if(wParam != SIZE_MINIMIZED){
				SetChildPosition(hWnd, ListWidth, CalendarHeight, iRatio);
			}
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void CreateChildWindow(HWND hWnd){
	HINSTANCE hInst = GetModuleHandle(NULL);
	
	hList = CreateWindow(
				TEXT("ListBox"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER,
				0,0,0,0,
				hWnd,
				(HMENU)ID_LIST,
				hInst,
				NULL
			);

	hCalendar = CreateWindow(
				TEXT("ListBox"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY | LBS_MULTICOLUMN | LBS_NOINTEGRALHEIGHT,
				0,0,0,0,
				hWnd,
				(HMENU)ID_CALENDAR,
				hInst,
				NULL
			);

	hEdit = CreateWindow(
				TEXT("Edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE,
				0,0,0,0,
				hWnd,
				(HMENU)ID_EDIT,
				hInst,
				NULL
			);

	for(int i=0; i<sizeof(hBtn)/sizeof(hBtn[0]); i++){
		hBtn[i] = CreateWindow(
				TEXT("Button"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | BS_PUSHBUTTON,
				0,0,0,0,
				hWnd,
				(HMENU)(INT_PTR)(ID_BTN[0] + i),
				hInst,
				NULL
			);
	}
}

void GetDate(int* Year, int* Month){
	SYSTEMTIME Today; 
	GetLocalTime(&Today);

	*Year = Today.wYear;
	*Month = Today.wMonth;
}

int DayOfTheWeek(int Year, int Month, int Day){
	SYSTEMTIME st;
	FILETIME ft;

	memset(&st, 0, sizeof(st));
	st.wYear = Year;
	st.wMonth = Month;
	st.wDay = Day;

	SystemTimeToFileTime(&st, &ft);
	FileTimeToSystemTime(&ft, &st);
	return st.wDayOfWeek;
}

int LastDateOfTheMonth(int Year, int Month){
	static int Table[] = {
		0,
		31, 28, 31, 30,
		31, 30, 31, 31,
		30, 31, 30, 31
	};

	int last;
	last = Table[Month];
	if(Month == 2 && ((Year % 4 == 0 && Year % 100 != 0) || Year % 400 == 0)){
		last = 29;
	}

	return last;
}

int GetLineCount(int Day, int LastDate){
	int Y, M;

	if(Day == -1 && LastDate == -1){
		GetDate(&Y, &M);
		Day = DayOfTheWeek(Y, M, 1);
		LastDate = LastDateOfTheMonth(Y, M);
	}

	if(Day == 0 && LastDate == 28){ return 4; }
	if(Day >= 5 && LastDate == 31){ return 6; }
	return 5;
}

void SetChildPosition(HWND hWnd, int ListWidth, int CalendarHeight, int iRatio){
	int ListHeight,
		EditWidth, EditHeight,
		CalendarWidth, 
		LineCount;

	ButtonSize BtnSize[BTNCOUNT];

	RECT crt;
	GetClientRect(hWnd, &crt);

	LineCount = GetLineCount();
	ListHeight = crt.bottom;

	CalendarWidth = crt.right - ListWidth;
	CalendarHeight = crt.bottom * iRatio * 0.01f;
	BtnSize[0] = {CalendarWidth, CalendarHeight / LineCount};
	
	EditWidth = crt.right - ListWidth;
	EditHeight = crt.bottom - CalendarHeight;
	// BtnSize[1] = {0,0};

	SetWindowPos(hList, NULL, crt.left, crt.top, ListWidth - GAP, ListHeight, SWP_NOZORDER);
	SetWindowPos(hBtn[0], NULL, ListWidth, crt.top, BtnSize[0].Width, BtnSize[0].Height, SWP_NOZORDER);
	SetWindowPos(hCalendar, NULL, ListWidth, BtnSize[0].Height, CalendarWidth, CalendarHeight - BtnSize[0].Height - GAP, SWP_NOZORDER);
	SetWindowPos(hEdit, NULL, ListWidth, CalendarHeight, EditWidth, EditHeight, SWP_NOZORDER);
	// SetWindowPos(hBtn[1], NULL, ListWidth + EditWidth * 0.5f, CalendarHeight + EditHeight, BtnSize[1].Width, BtnSize[1].Height, SWP_NOZORDER);
}

void GetCurrentSplit(HWND hWnd, SPLIT* Split, int iRatio){
	RECT crt, wrt, hrt, vrt;
	POINT pt;
	int CalendarHeight;

	GetCursorPos(&pt);
	ScreenToClient(hWnd, &pt);

	GetClientRect(hWnd, &crt);
	GetWindowRect(hList, &wrt);
	ScreenToClient(hWnd, (LPPOINT)&wrt);
	ScreenToClient(hWnd, (LPPOINT)&wrt+1);

	CalendarHeight = crt.bottom * iRatio * 0.01f;
	SetRect(&vrt, wrt.right, wrt.top, wrt.right + GAP, wrt.bottom);
	if(PtInRect(&vrt, pt)){
		*Split = VERT;
		return;
	}

	SetRect(&hrt, wrt.right, CalendarHeight - GAP, crt.right, CalendarHeight);
	if(PtInRect(&hrt, pt)){
		*Split = HORZ;
		return;
	}

	*Split = NONE;
}
