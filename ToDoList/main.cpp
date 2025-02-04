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
        // {WM_EXITSIZEMOVE, &MainWindow::OnExitResizeMove},
        {WM_CREATE, &MainWindow::OnCreate},
        {WM_DESTROY, &MainWindow::OnDestroy},
    };

	HWND hListView;
	INITCOMMONCONTROLSEX icex;
	HMENU hMenu, hPopupMenu, hPopupView;

	int ListViewWidth;
	int ListViewHeight;

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

    // LRESULT OnExitResizeMove(WPARAM wParam, LPARAM lParam);

private:
	BOOL RegisterHeader(HWND hListView, UINT Mask, int Format, int Width, LPCWSTR HeaderName, int Index);

public:
    MainWindow();
    ~MainWindow();

    LRESULT Handler(UINT iMessage, WPARAM wParam, LPARAM lParam);
};

BOOL MainWindow::CheckBox = FALSE,
	 MainWindow::GridLine = FALSE,
	 MainWindow::RowSelect = FALSE,
	 MainWindow::DragDrop = FALSE;

// Utility
#include "MyUtility.h"
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

    if (!win.Create(L"ToDoList")){ return 0; }

    ShowWindow(win.Window(), nCmdShow);
	MyUtility::CenterWindow(win.Window());

    MSG msg = { 0 };
    while(GetMessage(&msg, NULL, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    return (int)msg.wParam;
}

LRESULT MainWindow::Handler(UINT iMessage, WPARAM wParam, LPARAM lParam) {
    DWORD i;

    for(i=0; i<sizeof(MainMsg) / sizeof(MainMsg[0]); i++) {
        if (MainMsg[i].iMessage == iMessage) {
            return (this->*MainMsg[i].lpfnWndProc)(wParam, lParam);
        }
    }

    return DefWindowProc(_hWnd, iMessage, wParam, lParam);
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

// Windows Message
#define ID_VIEW_CHECKBOX	13100
#define ID_VIEW_GRIDLINE	13101
#define ID_VIEW_ROWSELECT	13102
#define ID_VIEW_DRAGDROP	13103

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
	hMenu		= CreateMenu();
	hPopupMenu	= CreatePopupMenu();
	hPopupView	= CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hPopupMenu, L"메뉴(&Menu)");
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
