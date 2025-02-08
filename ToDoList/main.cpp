///////////////////////////////// COMMENT /////////////////////////////////////
// C언어로 네트워크 통신 예시 프로그램을 만들었다.
// gnu 환경에서는 구조적 예외 처리 구문도 사용할 수 없기 때문에 상당히 불편하다.
// 예외 처리 구문을 사용할 수 있을 때와 아닐 때의 차이가 크며
// 불필요한 분기가 생겨날 수 있기 때문에 C++ 포맷과 C++ 컴파일러를 사용하기로 한다.
// 또한 캡슐화를 이용해 프로시저를 숨기고 전역 변수를 마음껏 사용하도록 하자.
// 요약하면 편하게 코딩하자는 뜻이다.
///////////////////////////////////////////////////////////////////////////////
// 이번 프로젝트에서는 리스트 뷰를 이용하여 초보 개발자들의 단골 소재인
// ToDoList 프로그램을 만든다.
// 실상 HTML과 CSS, JavaScript를 이용한 이쁜 TodoList가 많이 있어서
// C++과 API로 만드는 경우는 거의 없다.
// 간단한 예시 프로그램이라고는 하지만 직접 사용할 예정이므로 꽤 신경쓸 예정이다.
///////////////////////////////// COMMENT /////////////////////////////////////
// 리스트 뷰 컨트롤은 여러 가지 세부 항목을 가질 수 있으며
// 하나의 대상을 연속된 정보로 표현할 때 흔히 사용된다.
// 비슷한 예로 DB의 레코드를 떠올릴 수 있는데 실제로 유사하며
// DB프로그램을 만들 때 많이 사용되었던 컨트롤이다.
///////////////////////////////////////////////////////////////////////////////
// 리스트 뷰 컨트롤은 대상에 대한 2차원적인 정보를 보여주는데에 적합하며
// 98과 NT시절부터 사용되었기 때문에 굉장히 많은 기능을 갖고 있다.
// 사실상 프로그래밍 하기는 가장 어려운 컨트롤이므로 꼭 필요한 기능을 제외하곤
// 추가하지 않기로 한다.
///////////////////////////////// COMMENT /////////////////////////////////////

// Debug : 문자열의 좌상단 좌표를 0,0으로 맞췄을 때 비트맵의 좌상단(0,0)에 출력된다.
//	{
//		WCHAR buf[256];
//		StringCbPrintf(buf, sizeof(buf), L"ComboBox Client Rect = (%d,%d,%d,%d)", ComboBoxRect.left, ComboBoxRect.top, ComboBoxRect.right, ComboBoxRect.bottom);
//		// TextOut(hMemDC, 0,0, buf, wcslen(buf));
//	}

// ListView_GetItemCount(hListView);
// ListView_InsertItem(hList, &LI);

#define _WIN32_WINNT 0x0A00
//#define UNICODE				// 컴파일 옵션으로 유니코드 문자셋 지정
#include <windows.h>
#include <commctrl.h>
#include <strsafe.h>

#define ID_VIEW_CHECKBOX	13100
#define ID_VIEW_GRIDLINE	13101
#define ID_VIEW_ROWSELECT	13102
#define ID_VIEW_DRAGDROP	13103

#define ID_MENU_CALENDAR	13001

#define IDC_COMBOBOX		1000

#define IDC_BTNAPPEND		2000
#define IDC_BTNDELETE		2001

#define IDC_EDPRIORITY		3000
#define IDC_EDCATEGORY		3001
#define IDC_EDDATEYEAR		3002
#define IDC_EDDATEMONTH		3003
#define IDC_EDDATEDAY		3004
#define IDC_EDTODO			3005

#define WM_CHANGEFOCUS		WM_USER+1

void CenterWindow(HWND hWnd);
BOOL CheckLeapYear(int Year);
int CALLBACK Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParam);
LRESULT CALLBACK EditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

// Encapsulation
template <class DERIVED_TYPE>
class BaseWindow {
	protected:
		HWND _hWnd = NULL;

		virtual LPCWSTR ClassName() const = 0;
		virtual LRESULT Handler(UINT iMessage, WPARAM wParam, LPARAM lParam) = 0;

	public:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
			DERIVED_TYPE *ptr = NULL;

			if(iMessage == WM_NCCREATE){
				CREATESTRUCT* pCS = (CREATESTRUCT*)lParam;
				ptr = (DERIVED_TYPE*)pCS->lpCreateParams;
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ptr);

				ptr->_hWnd = hWnd;
			}else{
				ptr = (DERIVED_TYPE*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			}

			if(ptr){
				return ptr->Handler(iMessage, wParam, lParam);
			}else{
				return DefWindowProc(hWnd, iMessage, wParam, lParam);
			}
		}

		HWND Window() const { return _hWnd; }

		BOOL Create(
				PCWSTR lpszWindowName,
				DWORD dwStyle = WS_OVERLAPPEDWINDOW,
				DWORD dwExStyle = 0,
				LONG x = CW_USEDEFAULT,
				LONG y = CW_USEDEFAULT,
				LONG Width = CW_USEDEFAULT,
				LONG Height = CW_USEDEFAULT,
				HWND hWndParent = HWND_DESKTOP,
				HMENU hMenu = NULL
		){
			WNDCLASSEX wcex = {
					sizeof(wcex),
					CS_HREDRAW | CS_VREDRAW,
					DERIVED_TYPE::WndProc,
					0, 0,
					GetModuleHandle(NULL),
					NULL, LoadCursor(NULL, IDC_ARROW),
					(HBRUSH)(COLOR_WINDOW + 1),
					NULL,
					ClassName(),
					NULL
			};

			RegisterClassEx(&wcex);

			_hWnd = CreateWindowEx(
				dwExStyle,
				ClassName(),
				lpszWindowName,
				dwStyle,
				x, y, Width, Height,
				hWndParent,
				hMenu,
				GetModuleHandle(NULL),
				this
			);

			return ((_hWnd) ? TRUE : FALSE);
		}
};

// PopupWindow
class PopupWindow : public BaseWindow<PopupWindow> {
	// Window Reserved Message
    static const int _nMsg		= 0x400;

    typedef struct tag_MSGMAP {
        UINT iMessage;
        LRESULT(PopupWindow::* lpfnWndProc)(WPARAM, LPARAM);
    }MSGMAP;

    // QUERYENDSESSION
    MSGMAP MainMsg[_nMsg] = {
        {WM_PAINT, &PopupWindow::OnPaint},
        // {WM_DISPLAYCHANGE, &MainWindow::OnPaint},						// 해상도 고려해서 출력 : 추가예정
		{WM_KEYDOWN, &PopupWindow::OnKeyDown},								// 이 메시지를 처리하거나 창 활성시 포커스를 옮기는 방법으로 변경
		//{WM_ACTIVATE, &PopupWindow::OnActivate},
		{WM_MOUSEMOVE, &PopupWindow::OnMouseMove},
		{WM_COMMAND, &PopupWindow::OnCommand},
        {WM_SIZE, &PopupWindow::OnSize},
        {WM_MEASUREITEM, &PopupWindow::OnMeasureItem},
        {WM_DRAWITEM, &PopupWindow::OnDrawItem},
        {WM_CREATE, &PopupWindow::OnCreate},
        {WM_DESTROY, &PopupWindow::OnDestroy},
    };

	HWND hComboBox;
	HBITMAP hBitmap;

private:
	void DrawCalendar(HDC hdc, int cx, int cy);
	void DrawMouseTracker(HDC hdc);

private:
    LPCWSTR ClassName() const { return L"Example ToDoList Windows Program SubWindow Calendar"; }
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	//LRESULT OnActivate(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnSize(WPARAM wParam, LPARAM lParam);
    LRESULT OnMeasureItem(WPARAM wParam, LPARAM lParam);
    LRESULT OnDrawItem(WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);

public:
	PopupWindow();
    ~PopupWindow();

    LRESULT Handler(UINT iMessage, WPARAM wParam, LPARAM lParam);
};

// PopupWindow Init
PopupWindow::PopupWindow(){
	// 생성 및 초기화
}

PopupWindow::~PopupWindow(){
	// 삭제 및 해제
}

// PopupWindow Message
LRESULT PopupWindow::Handler(UINT iMessage, WPARAM wParam, LPARAM lParam){
	DWORD i;

    for(i=0; i<sizeof(MainMsg) / sizeof(MainMsg[0]); i++) {
        if (MainMsg[i].iMessage == iMessage) {
            return (this->*MainMsg[i].lpfnWndProc)(wParam, lParam);
        }
    }

    return DefWindowProc(_hWnd, iMessage, wParam, lParam);
}

LRESULT PopupWindow::OnMeasureItem(WPARAM wParam, LPARAM lParam){
	LPMEASUREITEMSTRUCT lpmis	= (LPMEASUREITEMSTRUCT)lParam;
	lpmis->itemHeight			= GetSystemMetrics(SM_CYVSCROLL);
	return TRUE;
}

LRESULT PopupWindow::OnDrawItem(WPARAM wParam, LPARAM lParam){
	LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
	WCHAR DatePickerItem[256];

	int x,y;
	SIZE TextSize;

	switch(lpdis->CtlID){
		case IDC_COMBOBOX:
			// 콤보 박스에 아이템이 없으면 -1이며, 보통 컨트롤이 포커스를 가졌을 때이다.
			if(lpdis->itemID == -1){
				SYSTEMTIME st;
				GetLocalTime(&st);
				int Year	= st.wYear,
					Month	= st.wMonth;

				StringCbPrintf(DatePickerItem, sizeof(DatePickerItem), L"%d년 - %d월", Year, Month);
				GetTextExtentPoint32(lpdis->hDC, DatePickerItem, wcslen(DatePickerItem), &TextSize);
				x = (lpdis->rcItem.right - lpdis->rcItem.left - TextSize.cx) / 2;
				y = (lpdis->rcItem.bottom - lpdis->rcItem.top - TextSize.cy) / 2;
				TextOut(lpdis->hDC, lpdis->rcItem.left + x, lpdis->rcItem.top + y, DatePickerItem, wcslen(DatePickerItem));
			}else if(lpdis->itemID >= 0){
				// 콤보 박스에 아이템이 있으면 인덱스 값을 가진다
				SetBkColor(lpdis->hDC, GetSysColor(COLOR_WINDOW));
				SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
				FillRect(lpdis->hDC, &lpdis->rcItem, (HBRUSH)(COLOR_WINDOW+1));

				if(lpdis->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)){
					SendMessage(hComboBox, CB_GETLBTEXT, (WPARAM)lpdis->itemID, (LPARAM)DatePickerItem);
					GetTextExtentPoint32(lpdis->hDC, DatePickerItem, wcslen(DatePickerItem), &TextSize);
					x = (lpdis->rcItem.right - lpdis->rcItem.left - TextSize.cx) / 2;
					y = (lpdis->rcItem.bottom - lpdis->rcItem.top - TextSize.cy) / 2;
					TextOut(lpdis->hDC, lpdis->rcItem.left + x, lpdis->rcItem.top + y, DatePickerItem, wcslen(DatePickerItem));
				}else if(lpdis->itemAction & (ODA_DRAWENTIRE | ODA_FOCUS)){
					HPEN	hOldPen,
							hPen		= CreatePen(PS_DOT, 1, RGB(0,255,0));
					HBRUSH	hOldBrush	= (HBRUSH)SelectObject(lpdis->hDC, GetStockObject(NULL_BRUSH));
							hOldPen		= (HPEN)SelectObject(lpdis->hDC, hPen);

					Rectangle(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right, lpdis->rcItem.bottom);

					SendMessage(hComboBox, CB_GETLBTEXT, (WPARAM)lpdis->itemID, (LPARAM)DatePickerItem);
					GetTextExtentPoint32(lpdis->hDC, DatePickerItem, wcslen(DatePickerItem), &TextSize);
					x = (lpdis->rcItem.right - lpdis->rcItem.left - TextSize.cx) / 2;
					y = (lpdis->rcItem.bottom - lpdis->rcItem.top - TextSize.cy) / 2;
					TextOut(lpdis->hDC, lpdis->rcItem.left + x, lpdis->rcItem.top + y, DatePickerItem, wcslen(DatePickerItem));

					SelectObject(lpdis->hDC, hOldPen);
					SelectObject(lpdis->hDC, hOldBrush);

					DeleteObject(hPen);
				}
			}
			break;

	}

	return TRUE;
}

LRESULT PopupWindow::OnSize(WPARAM wParam, LPARAM lParam){
	RECT crt, srt;
	int ComboBoxButtonWidth		= GetSystemMetrics(SM_CXVSCROLL),
		ComboBoxButtonHeight	= GetSystemMetrics(SM_CYVSCROLL);

	if(SIZE_MINIMIZED != wParam){
		GetClientRect(_hWnd, &crt);
		SetRect(&srt, ComboBoxButtonWidth, ComboBoxButtonHeight, (crt.right - crt.left) - ComboBoxButtonWidth * 2, (crt.bottom - crt.top) - ComboBoxButtonHeight);
		SetWindowPos(hComboBox, NULL, srt.left, srt.top, srt.right, srt.bottom, SWP_NOZORDER);

		if(hBitmap){
			DeleteObject(hBitmap);
			hBitmap = NULL;
		}

	}
	return 0;
}

LRESULT PopupWindow::OnKeyDown(WPARAM wParam, LPARAM lParam){
	WORD VKCode,
		  KeyFlags,
		  ScanCode,
		  RepeatCount;

	BOOL bExtended,
		 bWasKeyDown;

	// 추후 확장시 사용(모드 변경)
	VKCode		= LOWORD(wParam);
	KeyFlags	= HIWORD(lParam);
	ScanCode	= LOBYTE(KeyFlags);
	bExtended	= ((KeyFlags&& KF_EXTENDED) == KF_EXTENDED);

	// 확장 키 플래그 있을 시 0xE0이 접두(HIWORD)로 붙는다
	if(bExtended){ ScanCode = MAKEWORD(ScanCode, 0xE0); }

	bWasKeyDown = ((KeyFlags & KF_REPEAT) == KF_REPEAT);
	RepeatCount = LOWORD(lParam);

	// 특정 키입력은 리스트 박스로 전달
	switch(VKCode){
		case VK_UP:
		case VK_LEFT:
		case VK_DOWN:
		case VK_RIGHT:
			SendMessage(hComboBox, WM_KEYDOWN, wParam, lParam);
			break;
	}

	return 0;
}

LRESULT PopupWindow::OnCommand(WPARAM wParam, LPARAM lParam){
	int Index,
		Length;

	switch(LOWORD(wParam)){
		case IDC_COMBOBOX:
			switch(HIWORD(wParam)){
				case CBN_SELCHANGE:
					InvalidateRect(_hWnd, NULL, FALSE);
					break;
			}
			break;
	}

	return 0;
}

LRESULT PopupWindow::OnPaint(WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(_hWnd, &ps);
	
	// PopupWindow의 작업영역 내에서 콤보박스 위치
	RECT PopupWindowRect, ComboBoxRect;
	GetWindowRect(hComboBox, &ComboBoxRect);
	ScreenToClient(_hWnd, (LPPOINT)&ComboBoxRect);
	ScreenToClient(_hWnd, (LPPOINT)&ComboBoxRect+1);

	// PopupWindow의 작업영역 크기
	GetClientRect(_hWnd, &PopupWindowRect);

	// 비트맵 생성
	static RECT BitmapRect;

	HDC hMemDC		= CreateCompatibleDC(hdc);
	if(hBitmap == NULL){
		int BitmapWidth		= PopupWindowRect.right;
		int BitmapHeight	= PopupWindowRect.bottom - ComboBoxRect.bottom;
		hBitmap				= CreateCompatibleBitmap(hdc, BitmapWidth, BitmapHeight);

		// hMemDC가 내부 설정 고려
		SetRect(&BitmapRect, 0, 0, BitmapWidth, BitmapHeight);
	}
	HGDIOBJ hOld	= SelectObject(hMemDC, hBitmap);
	// COMMENT:	hMemDC는 선택된 리소스에 따라 적절한 정보를 유지/관리하는데
	//			지금처럼 DC에 비트맵을 선택하면(=교체, 끼워넣음) 내부적으로 DC가 관리하는 화면 영역을 비트맵 좌상단에 맞추는 것으로 보인다.
	FillRect(hMemDC, &BitmapRect, GetSysColorBrush(COLOR_BTNFACE));

	// 달력
	DrawCalendar(hMemDC, BitmapRect.right - BitmapRect.left, BitmapRect.bottom - BitmapRect.top);
	// 마우스 트래커
	DrawMouseTracker(hMemDC);
	
	// 출력 위치 설정
	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);
	BitBlt(hdc, 0, ComboBoxRect.bottom, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

	// 리소스 정리
	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);

	EndPaint(_hWnd, &ps);
	return 0;
}

LRESULT PopupWindow::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT crt, wrt, ort, srt;
	HWND hParent = GetParent(_hWnd);

	GetWindowRect(hParent, &ort);
	int OwnerWidth	= ort.right - ort.left;
	int OwnerHeight	= ort.bottom - ort.top;
	
	DWORD dwStyle	= GetWindowLongPtr(_hWnd, GWL_STYLE);
	DWORD dwExStyle	= GetWindowLongPtr(_hWnd, GWL_EXSTYLE);

	SetRect(&srt, 0, 0, OwnerWidth / 2, OwnerHeight / 2);
	AdjustWindowRectEx(&srt, dwStyle, GetMenu(_hWnd) != NULL, dwExStyle);
	if(dwStyle & WS_VSCROLL){ srt.right += GetSystemMetrics(SM_CXVSCROLL); }
	if(dwStyle & WS_HSCROLL){ srt.bottom += GetSystemMetrics(SM_CYHSCROLL); }

	int CalendarWidth	= srt.right - srt.left;
	int CalendarHeight	= srt.bottom - srt.top;
	SetRect(&crt, ort.left + (OwnerWidth - CalendarWidth) / 2, ort.top + (OwnerHeight - CalendarHeight) / 2, CalendarWidth, CalendarHeight);
	SetWindowPos(_hWnd, NULL, crt.left, crt.top, crt.right, crt.bottom, SWP_NOZORDER);

	int ComboBoxButtonWidth		= GetSystemMetrics(SM_CXVSCROLL);
	int ComboBoxButtonHeight	= GetSystemMetrics(SM_CYVSCROLL);

	GetClientRect(_hWnd, &crt);
	SetRect(&srt, crt.left + ComboBoxButtonWidth, crt.top + ComboBoxButtonHeight, (crt.right - crt.left) - ComboBoxButtonWidth * 2, (crt.bottom - crt.top) - ComboBoxButtonHeight);
	hComboBox = CreateWindow(L"combobox", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_OWNERDRAWFIXED | CBS_DROPDOWNLIST | CBS_HASSTRINGS, 0,0,0,0, _hWnd, (HMENU)IDC_COMBOBOX, GetModuleHandle(NULL), NULL);
	SetWindowPos(hComboBox, NULL, srt.left, srt.top, srt.right, srt.bottom, SWP_NOZORDER);

	SYSTEMTIME st;
	GetLocalTime(&st);
	int Year	= st.wYear,
		Month	= st.wMonth,
		Day		= st.wDay;

	int YearRange	= 5,
		MonthRange	= 12;

	TCHAR DatePickerItem[256];
	for(int i=0; i<YearRange * 2; i++){
		for(int j=1; j<=MonthRange; j++){
			StringCbPrintf(DatePickerItem, sizeof(DatePickerItem), L"%d년 - %d월", (Year - YearRange)+ i, j);
			SendDlgItemMessage(_hWnd, IDC_COMBOBOX, CB_ADDSTRING, 0, (LPARAM)DatePickerItem);
		}
	}
	return 0;
}

LRESULT PopupWindow::OnDestroy(WPARAM wParam, LPARAM lParam){
	if(hBitmap){DeleteObject(hBitmap);}
	return 0;
}

LRESULT PopupWindow::OnMouseMove(WPARAM wParam, LPARAM lParam){
	InvalidateRect(_hWnd, NULL, FALSE);
	return 0;
}

void PopupWindow::DrawCalendar(HDC hdc, int cx, int cy){
	static int Days[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	static CONST WCHAR* DayOfTheWeek[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };

	int x = 0,
		y = 0,
		Index,
		Length,
		Year,
		Month,
		Day,
		LastDay,
		DayOfWeek,
		Div,
		DivGap,
		RowGap,
		ColumnGap;

	BOOL bTrue;
	RECT CellRect;
	WCHAR buf[64];
	SIZE TextSize;

	FILETIME ft;
	SYSTEMTIME st, today;

	GetLocalTime(&today);

	Index = SendMessage(hComboBox, CB_GETCURSEL, 0,0);
	if(Index == CB_ERR){
		Year = today.wYear;
		Month = today.wMonth;
		StringCbPrintf(buf, sizeof(buf), L"%d년 - %d월", Year, Month);
		Index = SendMessage(hComboBox, CB_FINDSTRING, (WPARAM)-1, (LPARAM)buf);
		SendMessage(hComboBox, CB_SETCURSEL, (WPARAM)Index, (LPARAM)0);
	}else{
		SendMessage(hComboBox, CB_GETLBTEXT, (WPARAM)Index, (LPARAM)buf);

		for(int i=0; buf[i]; i++){
			if(buf[i] >= '0' && buf[i] <= '9'){
				x*= 10;
				x+= buf[i] - '0';
			}else if(buf[i] == '-'){
				Year	= x;
				x		= 0;
			}
		}

		Month = x;
	}

	if(Month == 2 && CheckLeapYear(Year)){
		LastDay = 29;
	}else{
		LastDay = Days[Month];
	}

	memset(&st, 0, sizeof(st));
	st.wYear	= Year;
	st.wMonth	= Month;
	st.wDay		= 1;
	SystemTimeToFileTime(&st, &ft);
	FileTimeToSystemTime(&ft, &st);
	
	DayOfWeek	= st.wDayOfWeek;

	HPEN hPen = CreatePen(PS_DOT, 1, RGB(255,255,255)),
		 hOldPen = (HPEN)SelectObject(hdc, hPen);

	Div		= 7;
	DivGap	= cx / Div;
	for(int i=0; i<Div+1; i++){
		MoveToEx(hdc, i * DivGap, 0, NULL);
		LineTo(hdc, i * DivGap, cy);
	}
	CellRect.right = DivGap;

	DivGap	= cy / Div;
	for(int i=0; i<Div+1; i++){
		MoveToEx(hdc, 0, i * DivGap, NULL);
		LineTo(hdc, cx, i * DivGap);
	}
	CellRect.bottom = DivGap;
	SelectObject(hdc, hOldPen);

	CellRect.left = CellRect.top = 0;

	RowGap		= CellRect.bottom - CellRect.top;
	ColumnGap	= CellRect.right - CellRect.left;

	#define GET_RED(V)			((BYTE)(((DWORD_PTR)(V)) & 0xff))
	#define GET_GREEN(V)		((BYTE)(((DWORD_PTR)(((WORD)(V))) >> 8) & 0xff))
	#define GET_BLUE(V)			((BYTE)(((DWORD_PTR)(V >> 16)) & 0xff))
	#define SET_RGB(R,G,B)		((COLORREF)(((BYTE)(R) | ((WORD)((BYTE)(G)) << 8)) | (((DWORD)(BYTE)(B)) << 16)))

	// RED
	#define RUBYRED				0xE0115F
	#define ROSERED				0xFF033E
	#define CRIMSONRED			0xDC143C
	#define VERMILIONRED		0xE34234

	// BLUE
	#define DEEPSKYBLUE			0x00BFFF
	#define LIGHTSKYBLUE		0x87CEFA
	#define LIGHTBLUEMIST		0x9AC0CD
	#define LIGHTCYANBLUE		0xE0FFFF

	// TODO: 매크로 활용
	HBRUSH hTodayBrush				= CreateSolidBrush(RGB(224,255,255)),
		   hOldBrush;

	COLORREF BeginningWeekendColor	= RGB(0,191,255),
			 HolidayColor			= RGB(255,3,62);

	SetBkMode(hdc, TRANSPARENT);

	WCHAR DayOfTheWeekBuf[32];
	for(int i=0; i<Div; i++){
		if(i == 0){ SetTextColor(hdc, HolidayColor); }
		else if(i == 6){ SetTextColor(hdc, BeginningWeekendColor); }
		else { SetTextColor(hdc, RGB(0,0,0)); }
		StringCbPrintf(DayOfTheWeekBuf, sizeof(DayOfTheWeekBuf), L"%s", DayOfTheWeek[i]);
		GetTextExtentPoint32(hdc, DayOfTheWeekBuf, wcslen(DayOfTheWeekBuf), &TextSize);
		x = (CellRect.right - CellRect.left - TextSize.cx) / 2;
		y = (CellRect.bottom - CellRect.top - TextSize.cy) / 2;	

		TextOut(hdc, i * ColumnGap + x, CellRect.left + y, DayOfTheWeekBuf, wcslen(DayOfTheWeekBuf));
	}

	POINT Origin;
	WCHAR DayBuf[256];
	int yy				= CellRect.top + RowGap,
		iRowRadius		= RowGap / 2,
		iColumnRadius	= ColumnGap / 2;
	for(Day = 1; Day <= LastDay; Day++){
		StringCbPrintf(DayBuf, sizeof(DayBuf), L"%d", Day);
		GetTextExtentPoint32(hdc, DayBuf, wcslen(DayBuf), &TextSize);
		x = (CellRect.right - CellRect.left - TextSize.cx) / 2;
		y = (CellRect.bottom - CellRect.top - TextSize.cy) / 2;	

		if(Year == today.wYear && Month == today.wMonth && Day == today.wDay){
			hOldPen		= (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
			hOldBrush	= (HBRUSH)SelectObject(hdc, hTodayBrush);

			Origin.x	= DayOfWeek * ColumnGap + iColumnRadius;
			Origin.y	= yy + iRowRadius;

			Ellipse(hdc, Origin.x - iColumnRadius, Origin.y - iRowRadius, Origin.x + iColumnRadius, Origin.y + iRowRadius);
			SelectObject(hdc, hOldBrush);
			SelectObject(hdc, hOldPen);
		}

		if(DayOfWeek == 0){
			SetTextColor(hdc, HolidayColor);
		}else if(DayOfWeek == 6){
			SetTextColor(hdc, BeginningWeekendColor);
		}else{
			SetTextColor(hdc, SET_RGB(0,0,0));
		}

		TextOut(hdc, DayOfWeek * ColumnGap + x, yy + y, DayBuf, wcslen(DayBuf));
	
		DayOfWeek++;
		if(DayOfWeek == 7){
			DayOfWeek = 0;
			yy += RowGap;
		}
	}

	DeleteObject(hTodayBrush);
	DeleteObject(hPen);
}

void PopupWindow::DrawMouseTracker(HDC hdc){
	POINT Mouse;
	GetCursorPos(&Mouse);
	ScreenToClient(_hWnd, &Mouse);

	RECT crt;
	GetClientRect(_hWnd, &crt);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	HBRUSH hOldBrush;
	Mouse.y -= (crt.bottom - crt.top) - bmp.bmHeight;
	hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
	Ellipse(hdc, Mouse.x - 5, Mouse.y - 5, Mouse. x + 5, Mouse.y + 5);
	SelectObject(hdc, hOldBrush);
}

// MainWindow
class MainWindow : public BaseWindow<MainWindow> {
	// Window Reserved Message
    static const int _nMsg			= 0x400;
	static const int nItems			= 4;
	static const int nEditItems		= 6;
	static const int nButtonItems	= 2;
	static const int nControls		= nEditItems + nButtonItems;
	static BOOL CheckBox, GridLine, RowSelect, DragDrop;

    typedef struct tag_MSGMAP {
        UINT iMessage;
        LRESULT(MainWindow::* lpfnWndProc)(WPARAM, LPARAM);
    }MSGMAP;

    // QUERYENDSESSION
    MSGMAP MainMsg[_nMsg] = {
        {WM_TIMER, &MainWindow::OnTimer},
        {WM_PAINT, &MainWindow::OnPaint},
        {WM_DISPLAYCHANGE, &MainWindow::OnPaint},						// 해상도 고려해서 출력 : 추가예정
        {WM_SIZE, &MainWindow::OnSize},
        {WM_INITMENU, &MainWindow::OnInitMenu},
        {WM_INITMENUPOPUP, &MainWindow::OnInitMenuPopup},				// 각 팝업마다 활성시 매번 개별적으로 메시지 전달
        {WM_COMMAND, &MainWindow::OnCommand},
        {WM_NOTIFY, &MainWindow::OnNotify},
		{WM_LBUTTONDOWN, &MainWindow::OnLButtonDown},
		{WM_MOUSEMOVE, &MainWindow::OnMouseMove},
		{WM_LBUTTONUP, &MainWindow::OnLButtonUp},
        // {WM_EXITSIZEMOVE, &MainWindow::OnExitResizeMove},
		{WM_CHANGEFOCUS, &MainWindow::OnChangeFocus},
        {WM_CREATE, &MainWindow::OnCreate},
        {WM_DESTROY, &MainWindow::OnDestroy},
    };

	int ListViewWidth;
	int ListViewHeight;
	const int ThickFrame = 3;

	PopupWindow popwin;
	WNDPROC OldEditProc, OldButtonProc;
	HWND hListView, hCalendar, hControls[nControls];
	INITCOMMONCONTROLSEX icex;
	HMENU hMenu, hPopupMenu, hPopupView;

private:
    LPCWSTR ClassName() const { return L"Example ToDoList Windows Program"; }
    LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
    LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnInitMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);

	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);

    LRESULT OnChangeFocus(WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
    // LRESULT OnExitResizeMove(WPARAM wParam, LPARAM lParam);

private:
	BOOL RegisterHeader(HWND hListView, UINT Mask, int Format, int Width, LPCWSTR HeaderName, int Index);
	BOOL RegisterItems(HWND hListView, UINT Mask, int RowIndex, int ColumnIndex, int nItems, ...);

public:
    MainWindow();
    ~MainWindow();

    LRESULT Handler(UINT iMessage, WPARAM wParam, LPARAM lParam);
};

// Init static var
BOOL MainWindow::CheckBox		= FALSE,
	 MainWindow::GridLine		= FALSE,
	 MainWindow::RowSelect		= FALSE,
	 MainWindow::DragDrop		= FALSE;

// Utility
void CenterWindow(HWND hWnd) {
	RECT wrt, srt;
	LONG lWidth, lHeight;
	POINT NewPosition;

	GetWindowRect(hWnd, &wrt);
	GetWindowRect(GetDesktopWindow(), &srt);

	lWidth	= wrt.right - wrt.left;
	lHeight = wrt.bottom - wrt.top;
	NewPosition.x = (srt.right - lWidth) / 2;
	NewPosition.y = (srt.bottom - lHeight) / 2;

	SetWindowPos(hWnd, NULL, NewPosition.x, NewPosition.y, lWidth, lHeight, SWP_NOZORDER);
}

BOOL CheckLeapYear(int Year){
	if(Year % 4 == 0 && Year % 100 != 0){return TRUE;}
	if(Year % 100 == 0 && Year % 400 == 0){return TRUE;}

	return FALSE;
}

// MainWindow Init
MainWindow::MainWindow() {
	// DLL 초기화
	icex.dwSize		= sizeof(icex);
	icex.dwICC		= ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);
}

MainWindow::~MainWindow() {
	// DLL 해제
}

// MainThread
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    MainWindow win;

    if(!win.Create(L"ToDoList", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN)){ return 0; }

    ShowWindow(win.Window(), nCmdShow);
	CenterWindow(win.Window());

    MSG msg = { 0 };
    while(GetMessage(&msg, NULL, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    return (int)msg.wParam;
}

// Native Function
BOOL MainWindow::RegisterHeader(HWND hListView, UINT Mask, int Format, int Width, LPCWSTR HeaderName, int Index){
	LVCOLUMN Column;

	Column.mask		= Mask;
	Column.fmt		= Format;
	Column.cx		= Width;
	Column.pszText	= (LPWSTR)HeaderName;
	Column.iSubItem = Index;
	SendMessage(hListView, LVM_INSERTCOLUMN, Index, (LPARAM)&Column);

	return TRUE;
}

BOOL MainWindow::RegisterItems(HWND hListView, UINT Mask, int RowIndex, int ColumnIndex, int nItems, ...){
	int i=0, idx;
	va_list	AP;
	va_start(AP, nItems);

	LVITEM LI;
	WCHAR* TextItems[MainWindow::nItems];
	if(nItems > MainWindow::nItems){return FALSE;}

	while((TextItems[i++] = va_arg(AP, WCHAR*)) != NULL){;}

	DWORD dwStyle		= GetWindowLongPtr(hListView, GWL_STYLE);
	DWORD dwExLVStle	= ListView_GetExtendedListViewStyle(hListView);

	if(dwStyle & LVS_SORTASCENDING){
		LI.mask		= Mask;
		LI.iSubItem	= ColumnIndex;
		LI.pszText	= (LPWSTR)TextItems[0];
	}else{
		LI.mask		= Mask;
		LI.iItem	= RowIndex;
		LI.iSubItem	= ColumnIndex;
		LI.pszText	= (LPWSTR)TextItems[0];
	}

	if(Mask & LVIF_PARAM){
		LI.lParam	= (LPARAM)TextItems[0];
	}

	idx = ListView_InsertItem(hListView, &LI);

	i = 1;
	while(i <= (nItems - 1)){
		ListView_SetItemText(hListView, idx, ColumnIndex + i, TextItems[i]);
		i++;
	}

	va_end(AP);

	return TRUE;
}

// MainWindow Message
LRESULT MainWindow::Handler(UINT iMessage, WPARAM wParam, LPARAM lParam) {
    DWORD i;

    for(i=0; i<sizeof(MainMsg) / sizeof(MainMsg[0]); i++) {
        if (MainMsg[i].iMessage == iMessage) {
            return (this->*MainMsg[i].lpfnWndProc)(wParam, lParam);
        }
    }

    return DefWindowProc(_hWnd, iMessage, wParam, lParam);
}


LRESULT MainWindow::OnInitMenu(WPARAM wParam, LPARAM lParam){
	// 각 메뉴가 활성화 될 때 딱 한번만 전달
	// 한 군데서 관리하는 것이 편하므로 가급적 위 메시지만 사용
	((CheckBox) ? CheckMenuItem(GetSubMenu((HMENU)wParam, 0), ID_VIEW_CHECKBOX, MF_BYCOMMAND | MF_CHECKED) : CheckMenuItem(GetSubMenu((HMENU)wParam,0), ID_VIEW_CHECKBOX, MF_BYCOMMAND | MF_UNCHECKED));
	((GridLine) ? CheckMenuItem(GetSubMenu((HMENU)wParam, 0), ID_VIEW_GRIDLINE, MF_BYCOMMAND | MF_CHECKED) : CheckMenuItem(GetSubMenu((HMENU)wParam,0), ID_VIEW_GRIDLINE, MF_BYCOMMAND | MF_UNCHECKED));
	((RowSelect) ? CheckMenuItem(GetSubMenu((HMENU)wParam,0), ID_VIEW_ROWSELECT, MF_BYCOMMAND | MF_CHECKED) : CheckMenuItem(GetSubMenu((HMENU)wParam,0), ID_VIEW_ROWSELECT, MF_BYCOMMAND | MF_UNCHECKED));
	((DragDrop) ? CheckMenuItem(GetSubMenu((HMENU)wParam, 0), ID_VIEW_DRAGDROP, MF_BYCOMMAND | MF_CHECKED) : CheckMenuItem(GetSubMenu((HMENU)wParam,0), ID_VIEW_DRAGDROP, MF_BYCOMMAND | MF_UNCHECKED));

	return 0;
}

LRESULT MainWindow::OnInitMenuPopup(WPARAM wParam, LPARAM lParam){
	return (DefWindowProc(_hWnd, WM_INITMENUPOPUP, wParam, lParam));
}

LRESULT MainWindow::OnChangeFocus(WPARAM wParam, LPARAM lParam){
	HWND hPrevFocus = (HWND)lParam;
	WPARAM OnLShift = wParam;

	// LPARAM: HWND type
	// WPARAM: PREV(0), NEXT(1)
	switch(OnLShift){
		case 0:
			for(int i=0; i<nControls; i++){
				if(hControls[i] == hPrevFocus){
					if(i == 0){
						SetFocus(hControls[nControls-1]);
					}else{
						SetFocus(hControls[i - 1]);
					}
				}
			}
			break;

		case 1:
			for(int i=0; i<nControls; i++){
				if(hControls[i] == hPrevFocus){
					if(i == (nControls-1)){
						SetFocus(hControls[0]);
					}else{
						SetFocus(hControls[i + 1]);
					}
				}
			}
			break;
	}

	return 0;
}

LRESULT CALLBACK ButtonSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static CREATESTRUCT* cs;
	static WNDPROC OldButtonProc;
	static BOOL bNext;

	if(OldButtonProc == NULL){
		OldButtonProc = (WNDPROC)GetProp(GetParent(hWnd), L"CallBackButtonWndProc");
	}

	switch(iMessage){
		case WM_CHAR:
		case WM_KEYUP:
		case WM_KEYDOWN:
			{
				WORD VKCode,
					  KeyFlags,
					  ScanCode,
					  RepeatCount;

				BOOL bExtended,
					 bWasKeyDown,
					 bKeyReleased;

				// 추후 확장시 사용(모드 변경)
				VKCode		= LOWORD(wParam);
				KeyFlags	= HIWORD(lParam);
				ScanCode	= LOBYTE(KeyFlags);
				bExtended	= ((KeyFlags&& KF_EXTENDED) == KF_EXTENDED);

				// 확장 키 플래그 있을 시 0xE0이 접두(HIWORD)로 붙는다
				if(bExtended){ ScanCode = MAKEWORD(ScanCode, 0xE0); }

				bWasKeyDown = ((KeyFlags & KF_REPEAT) == KF_REPEAT);
				RepeatCount = LOWORD(lParam);

				bKeyReleased = ((KeyFlags & KF_UP) == KF_UP);

				// 컨트롤 포커스 한 번에 관리
				switch(VKCode){
					case VK_TAB:
						if(!bKeyReleased){
							if(GetKeyState(VK_LSHIFT) & 0x8000){
								SendMessage(GetParent(hWnd), WM_CHANGEFOCUS, (WPARAM)0, (LPARAM)hWnd);
							}else{
								SendMessage(GetParent(hWnd), WM_CHANGEFOCUS, (WPARAM)1, (LPARAM)hWnd);
							}
						}
						return 0;
				}
			}
			break;

		case WM_CREATE:
			cs = (CREATESTRUCT*)lParam;
	}

	return CallWindowProc(OldButtonProc, hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK EditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static CREATESTRUCT* cs;
	static WNDPROC OldEditProc;
	static BOOL bNext = TRUE;

	if(OldEditProc == NULL){
		OldEditProc = (WNDPROC)GetProp(GetParent(hWnd), L"CallBackEditWndProc");
	}

	switch(iMessage){
		case WM_CHAR:
		case WM_KEYUP:
		case WM_KEYDOWN:
			{
				WORD VKCode,
					  KeyFlags,
					  ScanCode,
					  RepeatCount;

				BOOL bExtended,
					 bWasKeyDown,
					 bKeyReleased;

				// 추후 확장시 사용(모드 변경)
				VKCode		= LOWORD(wParam);
				KeyFlags	= HIWORD(lParam);
				ScanCode	= LOBYTE(KeyFlags);
				bExtended	= ((KeyFlags&& KF_EXTENDED) == KF_EXTENDED);

				// 확장 키 플래그 있을 시 0xE0이 접두(HIWORD)로 붙는다
				if(bExtended){ ScanCode = MAKEWORD(ScanCode, 0xE0); }

				bWasKeyDown = ((KeyFlags & KF_REPEAT) == KF_REPEAT);
				RepeatCount = LOWORD(lParam);

				bKeyReleased = ((KeyFlags & KF_UP) == KF_UP);

				// 컨트롤 포커스 한 번에 관리
				switch(VKCode){
					case VK_TAB:
						if(!bKeyReleased){
							if(GetKeyState(VK_LSHIFT) & 0x8000){
								SendMessage(GetParent(hWnd), WM_CHANGEFOCUS, (WPARAM)0, (LPARAM)hWnd);
							}else{
								SendMessage(GetParent(hWnd), WM_CHANGEFOCUS, (WPARAM)1, (LPARAM)hWnd);
							}
						}
						return 0;
				}
			}
			break;

		case WM_CREATE:
			cs = (CREATESTRUCT*)lParam;
	}

	return CallWindowProc(OldEditProc, hWnd, iMessage, wParam, lParam);
}

LRESULT MainWindow::OnCommand(WPARAM wParam, LPARAM lParam){
	// 리스트 뷰의 보기 스타일 자체는 윈도우 스타일이나,
	// 리스트 뷰의 확장 스타일은 정해진 함수나 메시지를 이용하여야 한다.
	// 비트 수가 모자랄 정도로 많은 확장 스타일을 지원하므로 마스크 값을 알고 있는게 아니라면 함수를 활용해야 한다.
	DWORD dwExListViewStyle = ListView_GetExtendedListViewStyle(hListView); 

	switch(LOWORD(wParam)){
		case ID_MENU_CALENDAR:
			if(popwin.Window() != NULL){DestroyWindow(popwin.Window());}
			popwin.Create(L"Calendar", WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_VISIBLE | WS_CLIPCHILDREN, 0,0,0,0,0, _hWnd, NULL);
			break;
		case ID_VIEW_CHECKBOX:
			CheckBox	= !CheckBox;
			dwExListViewStyle = ((CheckBox) ? (DWORD)(DWORD_PTR)(dwExListViewStyle | LVS_EX_CHECKBOXES) : (DWORD)(DWORD_PTR)(dwExListViewStyle & ~LVS_EX_CHECKBOXES));
			break;
		case ID_VIEW_GRIDLINE:
			GridLine	= !GridLine;
			dwExListViewStyle = ((GridLine) ? (DWORD)(DWORD_PTR)(dwExListViewStyle | LVS_EX_GRIDLINES) : (DWORD)(DWORD_PTR)(dwExListViewStyle & ~LVS_EX_GRIDLINES));
		case ID_VIEW_ROWSELECT:
			RowSelect	= !RowSelect;
			dwExListViewStyle = ((RowSelect) ? (DWORD)(DWORD_PTR)(dwExListViewStyle | LVS_EX_FULLROWSELECT) : (DWORD)(DWORD_PTR)(dwExListViewStyle & ~LVS_EX_FULLROWSELECT));
			break;
		case ID_VIEW_DRAGDROP:
			DragDrop	= !DragDrop;
			dwExListViewStyle = ((DragDrop) ? (DWORD)(DWORD_PTR)(dwExListViewStyle | LVS_EX_HEADERDRAGDROP) : (DWORD)(DWORD_PTR)(dwExListViewStyle & ~LVS_EX_HEADERDRAGDROP));
			break;

		case IDC_BTNAPPEND:
			break;
		case IDC_BTNDELETE:
			break;

		case IDC_EDPRIORITY:
			break;
		case IDC_EDCATEGORY:
			break;
		case IDC_EDDATEYEAR:
			break;
		case IDC_EDDATEMONTH:
			break;
		case IDC_EDDATEDAY:
			break;
		case IDC_EDTODO:
			break;
	}

	ListView_SetExtendedListViewStyle(hListView, dwExListViewStyle);
	return 0;
}

// 비교 함수
int CALLBACK Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParam){
	int ret,
		LeftItem,
		RightItem,
		LeftValue	= 0,
		RightValue	= 0;

	WCHAR	lpszLT[0x10],
			lpszRT[0x10];

	LVFINDINFO fi;
	LPNMLISTVIEW lv	= (LPNMLISTVIEW)lParam;
	HWND hListView	= lv->hdr.hwndFrom;

	fi.flags		= LVFI_STRING;
	fi.psz			= (LPCWSTR)lParam1;
	fi.vkDirection	= VK_DOWN;
	LeftItem		= ListView_FindItem(hListView, -1, &fi);

	fi.psz			= (LPCWSTR)lParam2;
	RightItem		= ListView_FindItem(hListView, -1, &fi);

	ListView_GetItemText(hListView, LeftItem, lv->iSubItem, lpszLT, 0x10);
	ListView_GetItemText(hListView, RightItem, lv->iSubItem, lpszRT, 0x10);
	
	int i=0, j=0;
	while(lpszLT[i]){ LeftValue *= 10; LeftValue += lpszLT[i] - '0'; i++; }
	while(lpszRT[j]){ RightValue *= 10; RightValue += lpszRT[j] -'0'; j++; }

	return (((LeftValue) < (RightValue)) ? -1 : ((LeftValue) > (RightValue)) ? 1 : 0);
}

LRESULT MainWindow::OnNotify(WPARAM wParam, LPARAM lParam){
	LPNMHDR hdr;
	LPNMLISTVIEW lv;
	LPNMLVGETINFOTIP tip;

	hdr = (LPNMHDR)lParam;
	lv	= (LPNMLISTVIEW)lParam;
	tip = (LPNMLVGETINFOTIP)lParam;

	WCHAR lpszToDo[256];
	WCHAR lpszCaption[256];

	switch(hdr->code){
		case LVN_GETINFOTIP:
			// TODO: 디버깅 및 툴팁 출력
			if(hdr->hwndFrom == hListView){
				ListView_GetItemText(hListView, tip->iItem, tip->iSubItem, lpszToDo, 256);
				// SetWindowText(_hWnd, lpszToDo);
				return 0;
			}
			break;

		case LVN_ITEMCHANGED:
			if(lv->uChanged == LVIF_STATE && lv->uNewState == (LVIS_SELECTED | LVIS_FOCUSED)){
				// ListView_GetItemText(hListView, lv->iiTem, lv->iSubITem, lpszCaption, 256);
			}
			break;

		case LVN_COLUMNCLICK:
			// 정렬
			ListView_SortItems(hListView, (PFNLVCOMPARE)Compare, lv);
			return TRUE;
	}

	return DefWindowProc(_hWnd, WM_NOTIFY, wParam, lParam);
}

LRESULT MainWindow::OnCreate(WPARAM wParam, LPARAM lParam) {
	hMenu			= CreateMenu();
	hPopupMenu		= CreatePopupMenu();
	hPopupView		= CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hPopupMenu, L"메뉴(&Menu)");
	AppendMenu(hMenu, MF_STRING, ID_MENU_CALENDAR, L"달력(C&alendar)");
	AppendMenu(hPopupMenu, MF_STRING | MF_POPUP, (UINT_PTR)hPopupView, L"보기(&View)");
	AppendMenu(hPopupView, MF_STRING, ID_VIEW_CHECKBOX, L"체크 박스(&C)");
	AppendMenu(hPopupView, MF_STRING, ID_VIEW_GRIDLINE, L"격자 줄선(&G)");
	AppendMenu(hPopupView, MF_STRING, ID_VIEW_ROWSELECT, L"행 전체 선택(&R)");
	AppendMenu(hPopupView, MF_STRING, ID_VIEW_DRAGDROP, L"세부항목 이동(&D)");
	SetMenu(_hWnd, hMenu);

	hListView	= CreateWindow(
			WC_LISTVIEW,
			NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT /*| LVS_SORTASCENDING*/,
			0,0,0,0,
			_hWnd,
			NULL,
			GetModuleHandle(NULL),
			NULL
	);

	ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP /*| LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT*/);
	CheckBox = GridLine = RowSelect = DragDrop = TRUE;

	int CellWidth	= ListViewWidth / nItems;

	// TODO: 디자인 끝낸 후 사용자가 항목 늘릴 수 있도록 확장
	RegisterHeader(hListView, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, CellWidth, L"우선순위", 0);
	RegisterHeader(hListView, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, CellWidth, L"구분", 1);
	RegisterHeader(hListView, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, CellWidth, L"날짜", 2);
	RegisterHeader(hListView, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, CellWidth, L"할 일", 3);

	RegisterItems(hListView, LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, 0, 0, nItems, L"1", L"해야될 일", L"2025-02-08", L"공부");
	RegisterItems(hListView, LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, 0, 0, nItems, L"2", L"해야될 일", L"2025-02-08", L"식사");
	RegisterItems(hListView, LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, 0, 0, nItems, L"3", L"해야될 일", L"2025-02-08", L"운동");
	RegisterItems(hListView, LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, 0, 0, nItems, L"4", L"해야될 일", L"2025-02-08", L"수면");
	
	// TODO: 입력칸 만들기
	WNDCLASS wc;
	GetClassInfo(NULL, L"edit", &wc);
	wc.hInstance		= GetModuleHandle(NULL);
	wc.lpszClassName	= L"MySubClassingEdit";
	OldEditProc			= wc.lpfnWndProc;
	wc.lpfnWndProc		= (WNDPROC)EditSubProc;
	RegisterClass(&wc);

	// 생성시 이전 프로시저에 대한 정보를 마지막 인수로 전달하려 했으나 논리적으로 맞지 않음
	// SetProp 이용해서 여분 메모리 할당하고 데이터 전달
	SetProp(_hWnd, L"CallBackEditWndProc", (HANDLE)OldEditProc);
	for(int i=0; i<nEditItems; i++){
		hControls[i] = CreateWindow(L"MySubClassingEdit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 0,0,0,0, _hWnd, (HMENU)IDC_EDPRIORITY + i, GetModuleHandle(NULL), NULL);
	}

	GetClassInfo(NULL, L"button", &wc);
	wc.hInstance		= GetModuleHandle(NULL);
	wc.lpszClassName	= L"MySubClassingButton";
	OldButtonProc		= wc.lpfnWndProc;
	wc.lpfnWndProc		= (WNDPROC)ButtonSubProc;
	RegisterClass(&wc);

	SetProp(_hWnd, L"CallBackButtonWndProc", (HANDLE)OldButtonProc);
	for(int i=0; i<nButtonItems; i++){
		hControls[nEditItems + i] = CreateWindow(L"MySubClassingButton", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,0,0,0, _hWnd, (HMENU)IDC_BTNAPPEND + i, GetModuleHandle(NULL), NULL);
	}

	SetWindowText(hControls[nEditItems], L"추가");
	SetWindowText(hControls[nEditItems+1], L"삭제");

	SetFocus(hControls[0]);
    return 0;
}

LRESULT MainWindow::OnDestroy(WPARAM wParam, LPARAM lParam) {
	// 본래 프로시저로 변경
	for(int i=0; i<nEditItems; i++){
		SetClassLongPtr(hControls[i], GCLP_WNDPROC, (LONG_PTR)OldEditProc);
	}
	for(int i=0; i<nButtonItems; i++){
		SetClassLongPtr(hControls[nEditItems + i], GCLP_WNDPROC, (LONG_PTR)OldButtonProc);
	}

	// 여분 메모리 삭제
	RemoveProp(_hWnd, L"CallBackEditWndProc");
	RemoveProp(_hWnd, L"CallBackButtonWndProc");

    PostQuitMessage(0);
    return 0;
}

LRESULT MainWindow::OnSize(WPARAM wParam, LPARAM lParam) {
	DWORD dwStyle, dwExStyle;
	RECT srt, crt, wrt;

    if (wParam != SIZE_MINIMIZED) {
		GetClientRect(_hWnd, &crt);
		InflateRect(
				&crt, 
				(-((crt.right - crt.left) * 0.2f)),
				(-((crt.bottom - crt.top) * 0.15f))
		);

		dwStyle		= GetWindowLongPtr(_hWnd, GWL_STYLE);
		dwExStyle	= GetWindowLongPtr(_hWnd, GWL_EXSTYLE);

		AdjustWindowRectEx(&crt, dwStyle, GetMenu(_hWnd) != NULL, dwExStyle);
		if(dwStyle & WS_VSCROLL){ crt.right += GetSystemMetrics(SM_CXVSCROLL); }
		if(dwStyle & WS_HSCROLL){ crt.bottom += GetSystemMetrics(SM_CYHSCROLL); }

		SetWindowPos(hListView, NULL, 10, 10, crt.right - crt.left, crt.bottom - crt.top, SWP_NOZORDER);
		ListViewWidth	= crt.right - crt.left;
		ListViewHeight	= crt.bottom - crt.top;

		// TODO: 위치 및 크기 설정
		HDWP hdwpEdit	= BeginDeferWindowPos(nEditItems);
		for(int i=0, j=0; i<nEditItems; j+=70, i++){
			hdwpEdit	= DeferWindowPos(hdwpEdit, hControls[i], NULL, 10 + j, ListViewHeight + 10 + 5, 50, 20, SWP_NOZORDER);
		}
		EndDeferWindowPos(hdwpEdit);

		HDWP hdwpButton	= BeginDeferWindowPos(nButtonItems);
		for(int i=0, j=0; i<nButtonItems; j+=70, i++){
			hdwpButton	= DeferWindowPos(hdwpButton, hControls[nEditItems + i], NULL, 10 + j, ListViewHeight + 10 + 35, 50, 20, SWP_NOZORDER);
		}
		EndDeferWindowPos(hdwpButton);
    }

    return 0;
}

/*
LRESULT MainWindow::OnExitResizeMove(WPARAM wParam, LPARAM lParam) {
    if (wParam != SIZE_MINIMIZED) {
        if (_Class && ResizeNotify) {
            ((GameRenderer*)_Class->*ResizeNotify)(wParam, lParam);
        }
    }

    return 0;
}
*/

LRESULT MainWindow::OnPaint(WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
	HDC hdc = BeginPaint(_hWnd, &ps);
    EndPaint(_hWnd, &ps);
    return 0;
}

LRESULT MainWindow::OnTimer(WPARAM wParam, LPARAM lParam) {
    /*
    switch (wParam) {
		case 1: {

		}
        break;
    }
    */
    return 0;
}

// TODO: 리스트뷰 사이즈 조정 기능 - 컨트롤 및 디자인 끝낸 후 추가 예정
LRESULT MainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	return 0;
}

LRESULT MainWindow::OnMouseMove(WPARAM wParam, LPARAM lParam){
	return 0;
}

LRESULT MainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam){
	return 0;
}
