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
#define IDC_EDDATE			3002
#define IDC_EDTODO			3003

#define WM_CHANGEFOCUS		WM_USER + 1

#define TEMPLATE_DATE			L"****-**-**"
#define TEMPLATE_DATE_UNTIL		L"****-**-** ~ ****-**-**"

void CenterWindow(HWND hWnd);
BOOL CheckLeapYear(int Year);
// BOOL MoveToIndex(HWND hWnd, int nItems, int From, int To);

int CALLBACK Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK CompareEx(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
LRESULT CALLBACK EditSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

struct tag_Arguments{
	int Priority;
	WCHAR buf[256];
};

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
        // {WM_DISPLAYCHANGE, &PopupWindow::OnPaint},						// 해상도 고려해서 출력 : 추가예정
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

		case VK_ESCAPE:
			DestroyWindow(_hWnd);
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

	// TODO: 범위 유효성 검사 - 캘린더 확장 기능 추가시 적용
	/*
	WCHAR	lpszDate[256];

	HWND hDateWnd = FindWindowEx(GetParent(_hWnd), NULL, L"MySubClassingEdit", NULL);
	while(IDC_EDDATE != (INT_PTR)GetDlgCtrlID(hDateWnd)){
		hDateWnd = FindWindowEx(GetParent(_hWnd), NULL, L"MySubClassingEdit", NULL);
	}

	GetWindowText(hDateWnd, lpszDate, 256);
	if(lpszDate == NULL){ MessageBeep(0); return 0;}

	int TemplateDateLength		= wcslen(TEMPLATE_DATE),
		TemplateDateUntilLength	= wcslen(TEMPLATE_DATE_UNTIL),
		TextLength				= wcslen(lpszDate);

	if(TextLength != TemplateDateLength && TextLength != TemplateDateUntilLength){
		MessageBeep(0);
		return 0;
	}

	// 포맷 검사
	for(int i=0; lpszDate[i]; i++){
		if(lpszDate[i] >= '0' && lpszDate[i] <'9'){
			lpszDate[i] = '*';
		}
	}

	if(wcscmp(lpszDate, TEMPLATE_DATE) != 0 && wcscmp(lpszDate, TEMPLATE_DATE_UNTIL) != 0){
		MessageBeep(0);
		return 0;
	}
	
	const int nSeps = 3;
	int i		= 0,
		j		= 0,
		x		= 0,
		DateTokenInt[nSeps] = {0,};

	WCHAR DateToken[nSeps][256];
	WCHAR Separators[] = L"-";					// 추후 '.'도 추가될 수 있음
	WCHAR* ptr = wcstok(lpszDate, lpszDate);
	while(ptr != NULL){
		StringCbCopy(DateToken[i], sizeof(DateToken[i]), ptr);
		ptr = wcstok(NULL, Separators);
	}

	for(i=0; nSeps; i++){
		x = 0;
		while(DateToken[i][j]){
			x*= 10;
			x+= DateToken[i][j] - '0';

			j++;
		}

		DateTokenInt[i] = x;
	}

	int Year	= 0,
		Month	= 1,
		Day		= 2;
	if(DateToken[Year] ... // 범위 검사)
	*/
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
	static const int nHeaders		= 4;
	static const int nEditItems		= 4;
	static const int nStaticItems	= 4;
	static const int nButtonItems	= 2;
	static const int nControls		= nStaticItems + nEditItems + nButtonItems;
	static const WCHAR* HeaderName[nHeaders];
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
		{WM_CTLCOLORSTATIC, &MainWindow::OnCtlColorStatic},
		{WM_SETFOCUS, &MainWindow::OnSetFocus},
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
    LRESULT OnCtlColorStatic(WPARAM wParam, LPARAM lParam);
    LRESULT OnSetFocus(WPARAM wParam, LPARAM lParam);

	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);

    LRESULT OnChangeFocus(WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
    // LRESULT OnExitResizeMove(WPARAM wParam, LPARAM lParam);

private:
	BOOL RegisterHeader(HWND hListView, UINT Mask, int Format, int Width, LPCWSTR HeaderName, int Index);
	BOOL RegisterItem(HWND hListView, UINT Mask, int RowIndex, int ColumnIndex, WCHAR (*InputItems)[256]); 

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

const WCHAR* MainWindow::HeaderName[] = {
	L"우선순위",
	L"구분",
	L"날짜",
	L"할 일"
};

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

BOOL MainWindow::RegisterItem(HWND hListView, UINT Mask, int RowIndex, int ColumnIndex, WCHAR (*InputItems)[256]){
	int i		= 0,
		idx		= 0;

	LVITEM	LI;
	WCHAR	TextItems[nHeaders][256];

	for(i=0; i<nHeaders; i++){
		StringCbCopy(TextItems[i], 256, InputItems[i]);
	}

	DWORD dwStyle		= GetWindowLongPtr(hListView, GWL_STYLE);
	DWORD dwExLVStle	= ListView_GetExtendedListViewStyle(hListView);

	// 항목 작성시 iSubItem을 지정하지 않으면 등록할 수 없다.
	if(dwStyle & LVS_SORTASCENDING){
		LI.iItem	= 0;
		LI.iSubItem	= ColumnIndex;
	}else{
		LI.iItem	= RowIndex;
		LI.iSubItem	= ColumnIndex;
	}

	LI.mask		= Mask;
	LI.pszText	= (LPWSTR)TextItems[0];
	if((Mask & LVIF_PARAM) == LVIF_PARAM){
		// LI.lParam = (LPARAM)LI.pszText;
	}

	idx = ListView_InsertItem(hListView, &LI);

	i = 1;
	while(i <= (nHeaders - 1)){
		ListView_SetItemText(hListView, idx, ColumnIndex + i, TextItems[i]);
		i++;
	}

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
	// WPARAM: TAB -> PREV(0), NEXT(1), Arrow -> PREV(2), NEXT(3)
	switch(OnLShift){
		case 0:
			for(int i=nStaticItems + 0; i<nControls; i++){
				if(hControls[i] == hPrevFocus){
					if(i == nStaticItems + 0){
						SetFocus(hControls[nControls-1]);
					}else{
						SetFocus(hControls[i - 1]);
					}
				}
			}
			break;

		case 1:
			for(int i=nStaticItems + 0; i<nControls; i++){
				if(hControls[i] == hPrevFocus){
					if(i == (nControls-1)){
						SetFocus(hControls[nStaticItems + 0]);
					}else{
						SetFocus(hControls[i + 1]);
					}
				}
			}
			break;

		case 2:
			for(int i=nStaticItems + 0; i<nControls; i++){
				if(hControls[i] == hPrevFocus){
					if(i != nStaticItems + (nEditItems - 1)){
						SetFocus(hControls[i+1]);
					}
				}
			}
			break;

		case 3:
			for(int i=nStaticItems + 0; i<nControls; i++){
				if(hControls[i] == hPrevFocus){
					if(i != nStaticItems){
						SetFocus(hControls[i-1]);
					}
				}
			}
			break;
	}

	return 0;
}

LRESULT MainWindow::OnCtlColorStatic(WPARAM wParam, LPARAM lParam){
	// HBRUSH hBrush = CreateSolidBrush(RGB(230,230,230));
	// DWORD CtrlID = GetDlgCtrlID((HWND)lParam);

	for(int i=0; i<nStaticItems; i++){
		if(hControls[i] == (HWND)lParam){
			HDC hdc		= (HDC)wParam;
			SetTextColor(hdc, RGB(0,0,0));
			SetBkColor(hdc, RGB(255,255,255));
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		}
	}

	return DefWindowProc(_hWnd, WM_CTLCOLORSTATIC, wParam, lParam);
}

LRESULT MainWindow::OnSetFocus(WPARAM wParam, LPARAM lParam){
	SetFocus(hControls[nStaticItems]);
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
				INT_PTR	CtrlID;
				switch(VKCode){
					case VK_TAB:
						if(!bKeyReleased){
							if(GetKeyState(VK_LSHIFT) & 0x8000){
								SendMessage(GetParent(hWnd), WM_CHANGEFOCUS, (WPARAM)0, (LPARAM)hWnd);
							}else{
								SendMessage(GetParent(hWnd), WM_CHANGEFOCUS, (WPARAM)1, (LPARAM)hWnd);
							}
							return 0;
						}
						break;
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
	static HWND DateWnd;

	if(OldEditProc == NULL){
		OldEditProc = (WNDPROC)GetProp(GetParent(hWnd), L"CallBackEditWndProc");
	}

	switch(iMessage){
		case WM_LBUTTONDOWN:
			SetFocus(hWnd);
			return 0;

		case WM_SETFOCUS:
			SendMessage(hWnd, EM_SETSEL, 0, -1);
			break;

		// case WM_KILLFOCUS:
			// {
				// 이렇게 하면 버튼이 포커스를 받는 시점에 곧장 포커스를 빼앗아 오기 때문에 기본 동작을 수행하지 않는다.
				// 재진입 가능한 특성을 갖기 때문에 VK_RETURN의 동작을 처리하는 도중에 포커스를 바로 빼앗아버린다.
				// HWND hAppend = FindWindowEx(GetParent(hWnd), NULL, L"MySubClassingButton", L"추가");
				// if(hAppend == (HWND)wParam){ SetFocus(hWnd); }
			// }
		//	break;

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
					case VK_UP:
					case VK_DOWN:
					case VK_TAB:
					case VK_RETURN:
						if(VKCode == VK_RETURN){
							if(bKeyReleased){
								if(DateWnd != NULL){
									WCHAR	lpszDate[256];

									GetWindowText(DateWnd, lpszDate, 256);
									if(lpszDate == NULL){ MessageBeep(0); return 0;}

									int TemplateDateLength		= wcslen(TEMPLATE_DATE),
										TemplateDateUntilLength	= wcslen(TEMPLATE_DATE_UNTIL),
										TextLength				= wcslen(lpszDate);

									if(TextLength != TemplateDateLength && TextLength != TemplateDateUntilLength){
										MessageBeep(0);
										return 0;
									}

									// 포맷 검사
									for(int i=0; lpszDate[i]; i++){
										if(lpszDate[i] >= '0' && lpszDate[i] <'9'){
											lpszDate[i] = '*';
										}
									}

									if(wcscmp(lpszDate, TEMPLATE_DATE) != 0 && wcscmp(lpszDate, TEMPLATE_DATE_UNTIL) != 0){
										MessageBeep(0);
										return 0;
									}
								}
								// 키 입력을 놓았을 때 메시지를 보낸다.
								HWND hAppend = FindWindowEx(GetParent(hWnd), NULL, L"MySubClassingButton", L"추가");
								// SendMessage는 대상이 모든 처리를 끝낼 때까지 리턴하지 않는다.
								SendMessage(hAppend, BM_CLICK, 0, 0);
								// 이 점을 이용하면 버튼이 기본 동작을 끝낸 후 포커스를 옮기는 것이 가능하다.
								SetFocus(hWnd);
							}
						}else{
							if(!bKeyReleased){
								if(IDC_EDDATE == (INT_PTR)GetDlgCtrlID(hWnd)){
									WCHAR lpszDate[256];
									GetWindowText(hWnd, lpszDate, 256);
									int TemplateDateLength		= wcslen(TEMPLATE_DATE),
										TemplateDateUntilLength	= wcslen(TEMPLATE_DATE_UNTIL),
										TextLength				= wcslen(lpszDate);

									if(TextLength != TemplateDateLength && TextLength != TemplateDateUntilLength){
										SYSTEMTIME st;
										GetLocalTime(&st);

										if(TextLength > TemplateDateLength){
											StringCbPrintf(lpszDate, sizeof(lpszDate), L"%04d-%02d-%02d ~ %04d-%02d-%02d", st.wYear, st.wMonth, st.wDay, st.wYear, st.wMonth, st.wDay);
										}else{
											StringCbPrintf(lpszDate, sizeof(lpszDate), L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
										}

										SetWindowText(hWnd, lpszDate);
									}
								}

								if(VKCode == VK_TAB){
									if(GetKeyState(VK_LSHIFT) & 0x8000){
										SendMessage(GetParent(hWnd), WM_CHANGEFOCUS, (WPARAM)0, (LPARAM)hWnd);
									}else{
										SendMessage(GetParent(hWnd), WM_CHANGEFOCUS, (WPARAM)1, (LPARAM)hWnd);
									}
								}else if(VKCode == VK_DOWN){
									SendMessage(GetParent(hWnd), WM_CHANGEFOCUS, (WPARAM)2, (LPARAM)hWnd);
								}else if(VKCode == VK_UP){
									SendMessage(GetParent(hWnd), WM_CHANGEFOCUS, (WPARAM)3, (LPARAM)hWnd);
								}
							}
						}
						return 0;
				}
			}
			break;

		case WM_CREATE:
			cs = (CREATESTRUCT*)lParam;
			if(IDC_EDDATE == (INT_PTR)GetDlgCtrlID(hWnd)){ DateWnd = hWnd; }
	}

	return CallWindowProc(OldEditProc, hWnd, iMessage, wParam, lParam);
}

LRESULT MainWindow::OnCommand(WPARAM wParam, LPARAM lParam){
	// 리스트 뷰의 보기 스타일 자체는 윈도우 스타일이나,
	// 리스트 뷰의 확장 스타일은 정해진 함수나 메시지를 이용하여야 한다.
	// 비트 수가 모자랄 정도로 많은 확장 스타일을 지원하므로 마스크 값을 알고 있는게 아니라면 함수를 활용해야 한다.
	DWORD	dwExListViewStyle = ListView_GetExtendedListViewStyle(hListView); 
	WCHAR	lpszDlgItems[nHeaders][256];

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
			if(HIWORD(wParam) == BN_CLICKED){
				for(int i=0; i<nHeaders; i++){
					GetDlgItemText(_hWnd, (INT_PTR)(IDC_EDPRIORITY + i), lpszDlgItems[i], 256);
				}
				RegisterItem(hListView, LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM /*| LVIF_STATE*/, 0, 0, lpszDlgItems);
				SetWindowText(hControls[nStaticItems + nEditItems - 1], L"");
			}
			break;

		case IDC_BTNDELETE:
			{
				int Index = ListView_GetNextItem(hListView, -1, LVNI_ALL | LVNI_SELECTED);
				while(Index != -1){
					ListView_DeleteItem(hListView, Index);
					Index = ListView_GetNextItem(hListView, -1, LVNI_ALL | LVNI_SELECTED);
				}
			}
			break;
	}

	ListView_SetExtendedListViewStyle(hListView, dwExListViewStyle);
	return 0;
}

int CALLBACK CompareEx(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	WCHAR	lpszLT[256],
			lpszRT[256];

	LPNMLISTVIEW lv		= (LPNMLISTVIEW)lParamSort;
	HWND hListView		= lv->hdr.hwndFrom;

	ListView_GetItemText(hListView, lParam1, (int)(IDC_EDDATE - IDC_EDPRIORITY), lpszLT, 256);
	ListView_GetItemText(hListView, lParam2, (int)(IDC_EDDATE - IDC_EDPRIORITY), lpszRT, 256);

	int	Compare,
		LeftValue,
		RightValue,
		LeftValueLength,
		RightValueLength,
		TemplateDateLength,
		TemplateDateUntilLength;

	LeftValueLength			= wcslen(lpszLT);
	RightValueLength		= wcslen(lpszRT);
	TemplateDateLength		= wcslen(TEMPLATE_DATE);
	TemplateDateUntilLength	= wcslen(TEMPLATE_DATE_UNTIL);

	// TODO: 날짜 서식 지정 완료, 포맷 비교 후 정렬 연산 수행
	if(LeftValueLength != RightValueLength){
		CONST WCHAR	*Separators = L" ~ ";
		WCHAR		*LongString = (((LeftValueLength) < (RightValueLength)) ? lpszRT : lpszLT),
					*ShortString= (((LeftValueLength) < (RightValueLength)) ? lpszLT : lpszRT),
					*ptr		= wcstok(LongString, Separators);

		Compare = 0;
		while(ptr != NULL && Compare == 0){
			Compare = wcscmp(ptr, ShortString);
			if(Compare < 0){
				LeftValue = -1;
				RightValue = 1;
			}
			else if(Compare > 0){
				LeftValue = 1;
				RightValue = -1;
			}else{
				ptr = wcstok(NULL, Separators);
			}
		}
		if(Compare == 0){ LeftValue = RightValue = 0; }

	}else{
		Compare = 0;
		WCHAR *lPtr = lpszLT, *rPtr = lpszRT;
		while(Compare == 0 && *rPtr){
			Compare = *lPtr++ - *rPtr++;
		}

		int Positive = (Compare > 0);
		int Negative = (Compare < 0);

		if((Positive - Negative) == 0)	{ LeftValue =		RightValue = 0; }
		if((Positive - Negative) > 0)	{ LeftValue = 1;	RightValue = -1; }
		if((Positive - Negative) < 0)	{ LeftValue = -1;	RightValue = 1; }
	}

	if(LeftValue == RightValue){
		ListView_GetItemText(hListView, lParam1, IDC_EDPRIORITY - IDC_EDPRIORITY, lpszLT, 256);
		ListView_GetItemText(hListView, lParam2, IDC_EDPRIORITY - IDC_EDPRIORITY, lpszRT, 256);

		LeftValue	= _wtoi(lpszLT);
		RightValue	= _wtoi(lpszRT);
	}

	return (((LeftValue) < (RightValue)) ? -1 : (((LeftValue) > (RightValue)) ? 1 : 0));
}

/*
// 비교 함수
int CALLBACK Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	// 현재 로직에선 아이템을 등록하는 함수 내부적으로 버퍼를 생성하여 lParam에 전달하고 있다.
	// 때문에 함수가 종료된 후 스택 프레임이 해제되면서 인스턴스화 된 데이터가 전부 날아가기 때문에 유효하지 않은 값이 lParam1, lParm2로 전달된다.
	// 이 함수 때문에 로직 전체를 변경할 생각은 없으므로 호출될 때 전체 리스트를 정렬하는 함수를 따로 작성하기로 한다.
	// 항목을 등록할 때 즉, RegisterItem 함수를 호출하는 부분을 보면 LVIF_LPARAM 플래그를 설정한 것을 알 수 있다.
	// 이 플래그를 설정해두면 사용자 정의 데이터를 저장해둘 수 있는데 이 값이 비교 함수 즉, 콜백함수인 Compare의 첫 번째 인수와 두 번째 인수로 전달된다.
	//WCHAR	lpszLT[256],
	//		lpszRT[256];

	LPNMLISTVIEW lv		= (LPNMLISTVIEW)lParamSort;
	HWND hListView		= lv->hdr.hwndFrom;

	// 항목 4개일 때 4번 호출된다. 즉, 항목 개수만큼 반복 호출되며 lv->iItem과 lv->iSubItem은 각각 -1과 0의 값을 가진다.
	// 리스트 뷰 항목을 식별하거나 사용하지 않는 경우 lv의 iItem은 -1, 하위 항목이 없는 경우 lv의 iSubItem은 0이 된다.

	// 아래와 같이 항목 번호를 0과 1로 지정하면 단순히 반전 시키는 동작도 가능하다.
	// ListView_GetItemText(hListView, 0, lv->iSubItem, lpszLT, 256);
	// ListView_GetItemText(hListView, 1, lv->iSubItem, lpszRT, 256);

	LVFINDINFO fi;
	fi.flags = LVFI_PARAM;
	fi.lParam = lParam1;
	fi.psz = (LPWSTR)lParam1;
	fi.vkDirection = VK_DOWN;
	int idx = ListView_FindItem(hListView, -1, &fi);
	ListView_GetItemText(hListView, idx, lv->iSubItem, lpszLT, 256);

	fi.flags = LVFI_STRING;
	fi.psz = (LPWSTR)lParam2;
	fi.vkDirection = VK_DOWN;
	idx = ListView_FindItem(hListView, -1, &fi);
	ListView_GetItemText(hListView, idx, lv->iSubItem, lpszRT, 256);

	int	LeftValue	= (int)lParam1,
		RightValue	= (int)lParam2;

	// LeftValue		= _wtoi(lpszLT);
	// RightValue		= _wtoi(lpszRT);

	// Debug : 문자열의 좌상단 좌표를 0,0으로 맞췄을 때 비트맵의 좌상단(0,0)에 출력된다.
	// {
	//	WCHAR buf[256];
	//	StringCbPrintf(buf, sizeof(buf), L"lParam1 = %s, lParam2 = %s", (LPWSTR)lParam1, (LPWSTR)lParam2);
	//	MessageBox(HWND_DESKTOP, buf, L"Debug", MB_OK);
	// }

	return (((LeftValue) < (RightValue)) ? -1 : ((LeftValue) > (RightValue)) ? 1 : 0);
}
*/

LRESULT MainWindow::OnNotify(WPARAM wParam, LPARAM lParam){
	LPNMHDR				hdr;
	LPNMLISTVIEW		lv;
	LPNMLVGETINFOTIP	tip;
	LPNMITEMACTIVATE	activate;

	LVCOLUMN			Column;
	int					ItemCount;

	hdr			= (LPNMHDR)lParam;
	switch(hdr->code){
		case LVN_ITEMACTIVATE:
			activate	= (LPNMITEMACTIVATE)lParam;
			if(hdr->hwndFrom == hListView){
				// 활성시 메시지 발생(Default : DblClk)
			}
			return 0;

		case LVN_GETINFOTIP:
			if(hdr->hwndFrom == hListView){
				tip		= (LPNMLVGETINFOTIP)lParam;

				if(tip->iSubItem == 0){
					WCHAR	lpszCat[256],
							lpszToolTipText[256],
							lpszToDo[nHeaders][256],
							lpszHeader[nHeaders][256];
					int		TextLength = 0;

					memset(lpszCat, 0 ,sizeof(lpszCat));
					for(int i=0; i < nHeaders; i++){
						Column.mask			= LVCF_TEXT;
						Column.pszText		= lpszHeader[i];
						Column.cchTextMax	= sizeof(lpszHeader[i]) / sizeof(lpszHeader[i][0]);

						ListView_GetColumn(hListView, i, &Column);
						StringCbCopy(lpszHeader[i], 256, Column.pszText);

						ListView_GetItemText(hListView, tip->iItem, i, lpszToDo[i], 256);
						StringCbPrintf(lpszToolTipText, sizeof(lpszToolTipText), L"%s = %s\r\n", lpszHeader[i], lpszToDo[i]);
						StringCchCat(lpszCat, sizeof(lpszCat) / sizeof(lpszCat[0]), lpszToolTipText);

						TextLength += wcslen(lpszToolTipText);
					}

					lpszCat[TextLength - 1] = lpszCat[TextLength - 2] = 0;
					StringCchPrintf(tip->pszText, TextLength + 1, L"%s",lpszCat);
				}else{

				}
			}
			break;

		case LVN_ITEMCHANGED:
			// 텍스트나 이미지의 변경, 포커스를 잃거나 선택 상태가 변경될 때 매번 보내진다.
			lv			= (LPNMLISTVIEW)lParam;
			if(lv->uChanged == LVIF_STATE && lv->uNewState == (LVIS_SELECTED | LVIS_FOCUSED)){
				// 첫 번째 항목을 선택한 상태에서 두 번째 항목으로 선택을 변경한다고 가정할 때, 총 세 번의 통지 메시지가 보내진다.
				// [기존 항목의 선택 해제 -> 기존 항목의 포커스 해제 -> 새로운 항목 선택 & 포커스] 순이며 상세히 알고 있어야 적합한 처리를 할 수 있다.
				// 해당 항목의 상태가 변경되고 변경된 상태가 선택과 포커스를 동시에 가질 때,
				// 즉 새로운 항목으로 변경이 완료된 시점에 수행할 동작을 작성한다.

				// 리스트 뷰의 각 항목이 가지는 최대 문자열 길이는 128이다(NULL포함).
				WCHAR lpszItemText[nHeaders][256];
				for(int i=0; i < nHeaders; i++){
					ListView_GetItemText(hListView, lv->iItem, i, lpszItemText[i], 256);
					SetWindowText(hControls[nStaticItems + i], lpszItemText[i]);
				}

				/*
				// 가상 리스트뷰 컨트롤이나 오너 드로우 리스트 뷰라면 최대 문자열 길이를 늘릴 수 있다.
				int ret,
					Length			= 128;

				LVITEM ToDoItem		= {0,};
				ToDoItem.iSubItem	= nEditItems - 1;

				do{
					Length *= 2;
					if(lpszToDoText != NULL){ free(lpszToDoText); }
					lpszToDoText = (WCHAR*)malloc(sizeof(WCHAR) * Length);
					ToDoItem.pszText	= lpszToDoText;
					ToDoItem.cchTextMax	= Length;
					ret = SendMessage(hListView, LVM_GETITEMTEXT, (WPARAM)lv->iItem, (LPARAM)&ToDoItem);
				}while(ret == Length - 1);

				SetWindowText(hControls[nStaticItems + nEditItems -1], lpszToDoText);
				free(lpszToDoText);
				*/
			}
			break;

		case LVN_COLUMNCLICK:
			lv			= (LPNMLISTVIEW)lParam;
			// ListView_SortItems(hListView, (PFNLVCOMPARE)Compare, lv);
			ListView_SortItemsEx(hListView, (PFNLVCOMPARE)CompareEx, lv);

			/*
			// 잘못된 로직
			ItemCount = ListView_GetItemCount(hListView);

			if(ItemCount > 1){
				int		Index,
						LeftItem,
						RightItem,
						LeftValue,
						RightValue;

				WCHAR	lpszLT[256],
						lpszRT[256];

				Index = 0;

				// 1차 순회
				while(Index < ItemCount - 1){
					// 두 번째 인수에 검색 시작 위치 지정, -1일 경우 처음부터 검색하며 검색 시작 지점은 검색에서 제외된다.
					// LeftItem		= ListView_GetNextItem(hListView, Index, LVNI_ALL | LVNI_BELOW);
					// RightItem	= ListView_GetNextItem(hListView, Index + 1, LVNI_ALL | LVNI_BELOW);

					ListView_GetItemText(hListView, Index, 0, lpszLT, 256);
					ListView_GetItemText(hListView, Index + 1, 0, lpszRT, 256);

					LeftValue		= _wtoi(lpszLT);
					RightValue		= _wtoi(lpszRT);

					if(LeftValue > RightValue){
						// 항목 번호는 내부적으로 관리하기 때문에 한 항목에 대해서만 순서를 변경하면 된다.
						MoveToIndex(hListView, nHeaders, Index+1, Index);
					}

					Index++;
				}
			}
			*/
			break;
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
			WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT, // LVS_SORTASCENDING
			0,0,0,0,
			_hWnd,
			NULL,
			GetModuleHandle(NULL),
			NULL
	);

	ListView_SetExtendedListViewStyle(
			hListView,
			LVS_EX_FULLROWSELECT |
			LVS_EX_GRIDLINES |
			LVS_EX_CHECKBOXES |
			LVS_EX_HEADERDRAGDROP |
			LVS_EX_INFOTIP |
			LVS_EX_LABELTIP |
			LVS_EX_TRACKSELECT
			/*| LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT | LVS_EX_UNDERLINECOLD*/
			);

	// unit: ms, -1(System Default)
	ListView_SetHoverTime(hListView, 10);
	CheckBox = GridLine = RowSelect = DragDrop = TRUE;

	int CellWidth	= ListViewWidth / nHeaders;

	// TODO: 디자인 끝낸 후 사용자가 항목 늘릴 수 있도록 확장
	for(int i=0; i<nHeaders; i++){
		RegisterHeader(hListView, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, CellWidth, HeaderName[i], i);
	}

	// 스태틱
	for(int i=0; i<nStaticItems; i++){
		hControls[i] = CreateWindow(L"static", NULL, WS_CHILD | WS_VISIBLE | SS_LEFT, 0,0,0,0, _hWnd, (HMENU)-1, GetModuleHandle(NULL), NULL);

		SetWindowText(hControls[i], HeaderName[i]);
	}

	// 에디트
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
		hControls[nStaticItems + i] = CreateWindow(L"MySubClassingEdit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL , 0,0,0,0, _hWnd, (HMENU)(INT_PTR)(IDC_EDPRIORITY + i), GetModuleHandle(NULL), NULL);
	}

	// Priority
	SetWindowLongPtr(hControls[nStaticItems + 0], GWL_STYLE, GetWindowLongPtr(hControls[nStaticItems + 0], GWL_STYLE) | ES_NUMBER);
	SendMessage(hControls[nStaticItems + 0], EM_LIMITTEXT, (WPARAM)9, 0);								// 정수 최대값이 10문자이므로 범위를 제한한다.

	// Category
	SendMessage(hControls[nStaticItems + 1], EM_LIMITTEXT, (WPARAM)32, 0);								// 문자 조합시 최대 길이가 256이므로 범위를 제한한다.

	// Date : 기본 날짜 포맷
	SYSTEMTIME st;
	GetLocalTime(&st);
	WCHAR DATE[256];
	StringCbPrintf(DATE, sizeof(DATE), L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
	SetWindowText(hControls[nStaticItems + 2], DATE);
	SendMessage(hControls[nStaticItems + 2], EM_LIMITTEXT, (WPARAM)23, 0);								// 날짜 서식 최대 길이로 범위를 제한한다.

	// ToDo
	// 기본적으로 리스트 뷰 컨트롤에 문자열 최대 허용 길이가 설정되어 있다.
	// 해당 프로젝트에서는 오너 드로우 리스트 뷰 컨트롤을 사용할 필요가 없으므로 변경하지 않기로 한다.
	SendMessage(hControls[nStaticItems + 3], EM_LIMITTEXT, (WPARAM)128 - (23 + 32 + 9), 0);				// 리스트 뷰 컨트롤의 최대 허용 길이로 범위를 제한한다.

	// 버튼
	GetClassInfo(NULL, L"button", &wc);
	wc.hInstance		= GetModuleHandle(NULL);
	wc.lpszClassName	= L"MySubClassingButton";
	OldButtonProc		= wc.lpfnWndProc;
	wc.lpfnWndProc		= (WNDPROC)ButtonSubProc;
	RegisterClass(&wc);

	SetProp(_hWnd, L"CallBackButtonWndProc", (HANDLE)OldButtonProc);
	for(int i=0; i<nButtonItems; i++){
		hControls[nStaticItems + nEditItems + i] = CreateWindow(L"MySubClassingButton", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,0,0,0, _hWnd, (HMENU)(INT_PTR)(IDC_BTNAPPEND + i), GetModuleHandle(NULL), NULL);
	}

	SetWindowText(hControls[nStaticItems + nEditItems], L"추가");
	SetWindowText(hControls[nStaticItems + nEditItems+1], L"삭제");

	SetFocus(hControls[nStaticItems + 0]);
    return 0;
}

LRESULT MainWindow::OnDestroy(WPARAM wParam, LPARAM lParam) {
	// 본래 프로시저로 변경
	for(int i=0; i<nEditItems; i++){
		SetClassLongPtr(hControls[nStaticItems + i], GCLP_WNDPROC, (LONG_PTR)OldEditProc);
	}
	for(int i=0; i<nButtonItems; i++){
		SetClassLongPtr(hControls[nStaticItems + nEditItems + i], GCLP_WNDPROC, (LONG_PTR)OldButtonProc);
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

	int x, y,
		Padding			= 10,
		EditWidth,
		EditHeight		= 20,
		StaticWidth,
		StaticHeight	= 20,
		ButtonWidth,
		ButtonHeight	= 20;

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

		SetWindowPos(hListView, NULL, Padding, Padding, crt.right - crt.left, crt.bottom - crt.top, SWP_NOZORDER);
		ListViewWidth	= crt.right - crt.left;
		ListViewHeight	= crt.bottom - crt.top;

		HDWP hdwpStatic = BeginDeferWindowPos(nStaticItems);
		for(int i=0; i<nEditItems; i++){
			GetWindowRect(hListView, &wrt);
			ScreenToClient(_hWnd, (LPPOINT)&wrt);
			ScreenToClient(_hWnd, (LPPOINT)&wrt+1);
			
			x			= wrt.right + Padding;
			y			= wrt.top;
			StaticWidth = 100;
			hdwpStatic	= DeferWindowPos(hdwpStatic, hControls[i], NULL, x, y + i * StaticHeight, StaticWidth, StaticHeight, SWP_NOZORDER);
		}
		EndDeferWindowPos(hdwpStatic);

		HDWP hdwpEdit	= BeginDeferWindowPos(nEditItems);
		for(int i=0; i<nEditItems; i++){
			GetWindowRect(hControls[0], &wrt);
			ScreenToClient(_hWnd, (LPPOINT)&wrt);
			ScreenToClient(_hWnd, (LPPOINT)&wrt+1);
			GetClientRect(_hWnd, &crt);

			x			= wrt.right + Padding;
			y			= wrt.top;
			EditWidth	= (crt.right - crt.left) - (ListViewWidth + StaticWidth + Padding * 4);
			hdwpEdit	= DeferWindowPos(hdwpEdit, hControls[nStaticItems + i], NULL, x, y + i * EditHeight, EditWidth, EditHeight, SWP_NOZORDER);
		}
		EndDeferWindowPos(hdwpEdit);

		HDWP hdwpButton	= BeginDeferWindowPos(nButtonItems);
		for(int i=nButtonItems, j=0; i>0 && j<nButtonItems; j++, i--){
			GetClientRect(_hWnd, &crt);

			ButtonWidth = 100;
			x			= (crt.right - crt.left) - ((ButtonWidth + Padding * 2) * i);
			y			= (crt.bottom - crt.top) - (Padding + ButtonHeight);
			hdwpButton	= DeferWindowPos(hdwpButton, hControls[nStaticItems + nEditItems + j], NULL, x, y, ButtonWidth, ButtonHeight, SWP_NOZORDER);
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

/*	// 수행시간이 길어진다
BOOL MoveToIndex(HWND hWnd, int nItems, int From, int To){
	int		i = 0,
			idx = 0;

	LVITEM	li;

	WCHAR** lpszPrevText = (WCHAR**)malloc(sizeof(WCHAR*) * nItems);
	for(i=0; i<nItems; i++){
		lpszPrevText[i] = (WCHAR*)malloc(sizeof(WCHAR) * 256);
	}

	for(i=0; i<nItems; i++){
		ListView_GetItemText(hWnd, From, i, lpszPrevText[i], 256);
	}

	if(!ListView_DeleteItem(hWnd, From)) { return FALSE; }

	// 항목 작성시 iSubItem을 지정하지 않으면 등록할 수 없다.
	li.mask			= LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM; //| LVIF_STATE;
	li.iItem		= To;
	li.iSubItem		= 0;
	li.pszText		= lpszPrevText[0];
	li.lParam		= (LPARAM)lpszPrevText[0];
	if((idx = ListView_InsertItem(hWnd, &li)) == -1) { return FALSE; }

	i = 1;
	while(i <= (nItems - 1)){
		ListView_SetItemText(hWnd, idx, i, lpszPrevText[i]);
		i++;
	}

	for(i=0; i<nItems; i++){
		free(lpszPrevText[i]);
	}
	free(lpszPrevText);

	return TRUE;
}
*/

