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
#define IDC_COMBOBOX		100

void CenterWindow(HWND hWnd);
BOOL CheckLeapYear(int Year);

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

private:
    LPCWSTR ClassName() const { return L"Example ToDoList Windows Program SubWindow Calendar"; }
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
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
	LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
	lpmis->itemHeight = GetSystemMetrics(SM_CYVSCROLL);
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

	// TODO: 달력 그리기
	DrawCalendar(hMemDC, BitmapRect.right - BitmapRect.left, BitmapRect.bottom - BitmapRect.top);

	// Debug : 문자열의 좌상단 좌표를 0,0으로 맞췄을 때 비트맵의 좌상단(0,0)에 출력된다.
	{
		WCHAR buf[256];
		StringCbPrintf(buf, sizeof(buf), L"ComboBox Client Rect = (%d,%d,%d,%d)", ComboBoxRect.left, ComboBoxRect.top, ComboBoxRect.right, ComboBoxRect.bottom);
		// TextOut(hMemDC, 0,0, buf, wcslen(buf));
	}

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

void PopupWindow::DrawCalendar(HDC hdc, int cx, int cy){
	static int Days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	int x = 0,
		Index,
		Length,
		Year,
		Month,
		Day,
		LastDay,
		DayOfWeek,
		Div,
		DivGap;

	BOOL bTrue;
	RECT CellRect;

	FILETIME ft;
	SYSTEMTIME st, today;
	WCHAR buf[256];

	GetLocalTime(&today);

	Index = SendMessage(hComboBox, CB_GETCURSEL, 0,0);
	if(Index == CB_ERR){
		Year = today.wYear;
		Month = today.wMonth;
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

	StringCbPrintf(buf, sizeof(buf), L"Month = %d, Year = %d", Month, Year);
	TextOut(hdc, 0,0, buf, wcslen(buf));

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

	// TODO: 요일
	for(Day = 1; Day <= LastDay; Day++){

	}

	// TODO: 일자

	// TODO: 리소스 정리
	DeleteObject(hPen);
}

// MainWindow
class MainWindow : public BaseWindow<MainWindow> {
	// Window Reserved Message
    static const int _nMsg		= 0x400;
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
        {WM_CREATE, &MainWindow::OnCreate},
        {WM_DESTROY, &MainWindow::OnDestroy},
    };

	PopupWindow popwin;
	HWND hListView, hCalendar;
	INITCOMMONCONTROLSEX icex;
	HMENU hMenu, hPopupMenu, hPopupView;

	int ListViewWidth;
	int ListViewHeight;
	const int ThickFrame = 3;

private:
    LPCWSTR ClassName() const { return L"Example ToDoList Windows Program"; }
    LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
    LRESULT OnSize(WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
	LRESULT OnInitMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);

	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);

    // LRESULT OnExitResizeMove(WPARAM wParam, LPARAM lParam);

private:
	BOOL RegisterHeader(HWND hListView, UINT Mask, int Format, int Width, LPCWSTR HeaderName, int Index);

public:
    MainWindow();
    ~MainWindow();

    LRESULT Handler(UINT iMessage, WPARAM wParam, LPARAM lParam);
};

// Init static var
BOOL MainWindow::CheckBox = FALSE,
	 MainWindow::GridLine = FALSE,
	 MainWindow::RowSelect = FALSE,
	 MainWindow::DragDrop = FALSE;

// Utility
void CenterWindow(HWND hWnd) {
	RECT wrt, srt;
	LONG lWidth, lHeight;
	POINT NewPosition;

	GetWindowRect(hWnd, &wrt);
	GetWindowRect(GetDesktopWindow(), &srt);

	lWidth = wrt.right - wrt.left;
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
			break;
		case ID_VIEW_ROWSELECT:
			RowSelect	= !RowSelect;
			dwExListViewStyle = ((RowSelect) ? (DWORD)(DWORD_PTR)(dwExListViewStyle | LVS_EX_FULLROWSELECT) : (DWORD)(DWORD_PTR)(dwExListViewStyle & ~LVS_EX_FULLROWSELECT));
			break;
		case ID_VIEW_DRAGDROP:
			DragDrop	= !DragDrop;
			dwExListViewStyle = ((DragDrop) ? (DWORD)(DWORD_PTR)(dwExListViewStyle | LVS_EX_HEADERDRAGDROP) : (DWORD)(DWORD_PTR)(dwExListViewStyle & ~LVS_EX_HEADERDRAGDROP));
			break;
	}

	ListView_SetExtendedListViewStyle(hListView, dwExListViewStyle);
	return 0;
}

LRESULT MainWindow::OnNotify(WPARAM wParam, LPARAM lParam){
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
			WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT,
			0,0,0,0,
			_hWnd,
			NULL,
			GetModuleHandle(NULL),
			NULL
	);

	ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES | LVS_EX_HEADERDRAGDROP);
	CheckBox = GridLine = RowSelect = DragDrop = TRUE;

	int nItems		= 4;
	int CellWidth	= ListViewWidth / nItems;

	RegisterHeader(hListView, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, CellWidth, L"우선순위", 0);
	RegisterHeader(hListView, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, CellWidth, L"구분", 1);
	RegisterHeader(hListView, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, CellWidth, L"날짜", 2);
	RegisterHeader(hListView, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, CellWidth, L"할 일", 3);

    return 0;
}

LRESULT MainWindow::OnDestroy(WPARAM wParam, LPARAM lParam) {
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
    BeginPaint(_hWnd, &ps);
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


