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
					0,0,
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
	static const int IDM_MENU	= 13000;

    typedef struct tag_MSGMAP {
        UINT iMessage;
        LRESULT(MainWindow::* lpfnWndProc)(WPARAM, LPARAM);
    }MSGMAP;

    // QUERYENDSESSION
    MSGMAP MainMsg[_nMsg] = {
        {WM_TIMER, &MainWindow::OnTimer},
        {WM_PAINT, &MainWindow::OnPaint},
        {WM_DISPLAYCHANGE, &MainWindow::OnPaint},				// 해상도 고려해서 출력 : 추가예정
        {WM_SIZE, &MainWindow::OnSize},
        // {WM_EXITSIZEMOVE, &MainWindow::OnExitResizeMove},
        {WM_CREATE, &MainWindow::OnCreate},
        {WM_DESTROY, &MainWindow::OnDestroy},
    };

	HWND hListView;
	INITCOMMONCONTROLSEX icex;

	int ListViewWidth;
	int ListViewHeight;

private:
    LPCWSTR ClassName() const { return L"Example ToDoList Windows Program"; }
    LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
    LRESULT OnSize(WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
    // LRESULT OnExitResizeMove(WPARAM wParam, LPARAM lParam);

private:
	BOOL RegisterHeader(HWND hListView, UINT Mask, int Format, int Width, LPCWSTR HeaderName, int Index);

public:
    MainWindow();
    ~MainWindow();

    LRESULT Handler(UINT iMessage, WPARAM wParam, LPARAM lParam);
};

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
LRESULT MainWindow::OnCreate(WPARAM wParam, LPARAM lParam) {
	HMENU hMenu, hPopupMenu;

	hMenu		= CreateMenu();
	hPopupMenu	= CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hPopupMenu, L"메뉴(&Menu)");
	AppendMenu(hPopupMenu, MF_STRING, (UINT_PTR)IDM_MENU, L"보기(&View)");
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

// 업데이트 예정
// TODO: 보기(View) 항목 팝업으로 변경
// TODO: 메뉴 항목에 캘린더 추가
// TODO: 리스트뷰 스타일 옵션 메뉴 항목에 추가
// TODO: 리스트뷰 사이즈 조정 기능(드래그)
// TODO: 간단히 보기(툴팁) 기능
// TODO: 자세히 보기(할 일 항목만, 읽기 전용 에디트) 기능
// TODO: 캘린더(공통 컨트롤 or 커스텀, 팝업 스타일)
// TODO: 파일 저장 형식 설계
// TODO: 리스트뷰 목록 파일로 저장
// TODO: 연결 프로그램 및 전용 파일 형식 제작
// TODO: 프로그램 종료시 마지막 위치 저장
// TODO: 미디어 플레이어 추가(=MP3, 공개 라이브러리 활용, 네트워크 연결없이 파일로 저장된 것만)

// 아래 항목은 시각적 디자인 마친 후 추가
// TODO: 항목별 입력란 설명자 정적 컨트롤 추가
// TODO: 항목별 입력란 에디트 컨트롤 추가
