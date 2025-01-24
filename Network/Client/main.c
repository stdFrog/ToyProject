#define UNICODE
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#include <commctrl.h>
#include <strsafe.h>

/* Main Window Control ID */
#define IDC_BTNSEND			101
#define IDC_EDMSG			201

/* Dialog Control ID */
#define IDC_DLGEDIPADDRESS	1101
#define IDC_DLGEDPORT		1201

/* Menu ID */
#define ID_CONFIGCONNECT	40001

#define THICKNESS			3
//#define THICKNESS			GetSystemMetrics(SM_CYSIZEFRAME)
//#define SERVERIP			"127.0.0.1"
//#define SERVERPORT		9000

/* ID of the Sync Object. */
#define RECVEVENTID			TEXT("Local\\ExampleRecvEvent_25_01_20")
#define SENDEVENTID			TEXT("Local\\ExampleSendEvent_25_01_20")
#define CONNECTEVENTID		TEXT("Local\\ExampleConnectEvent_25_01_20")

#define MAPPINGID			TEXT("Local\\ExampleMappingObject_25_01_20")
#define TIMERID				TEXT("Local\\ExampleTimerObject_25_01_20")

#define CLASSNAME			TEXT("ClientProgramLayout")
#define TCLASSNAME			TEXT("DisplayPannelClass")
#define LCLASSNAME			TEXT("EditPannelClass")
#define RCLASSNAME			TEXT("ButtonPannelClass")

/* Declare a Procedure */
INT_PTR CALLBACK DialogProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DisplayPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditSubProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK BtnPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

/* Declare a SubThread */
DWORD WINAPI ClientMain(LPVOID lpArg);
DWORD WINAPI WorkerThread(LPVOID lpArg);

/* Declare a Util */
void Exit(); 
void WSAExit();
DWORD ErrorMessage();
DWORD WSAErrorMessage();
void ShowMessage(TCHAR* buf);

int ConvertIP(const TCHAR *IP);
BOOL CheckSplitBar(HWND hWnd, POINT pt, int DisplayPannelRatio);

/* Indirect Dialog */
#define DLGTITLE			TEXT("InputBox")
#define DLGFONT				TEXT("MS Sans Serif")
#define SIZEOF(str)			(sizeof(str)/sizeof((str)[0]))

#pragma pack(push, 4)          
static struct tag_CustomDialog{
	DWORD	Style; 
	DWORD	dwExtendedStyle; 
	WORD	nControl; 
	short	x; 
	short	y; 
	short	cx; 
	short	cy; 
	WORD	Menu;							// 메뉴 리소스 서수 지정
	WORD	WndClass;						// 윈도우 클래스 서수 지정
	WCHAR	wszTitle[SIZEOF(DLGTITLE)];		// 대화상자 제목 지정
	short	FontSize;						// DS_SETFONT 스타일 지정 시 폰트 크기 설정(픽셀)
	WCHAR	wszFont[SIZEOF(DLGFONT)];		// DS_SETFONT 스타일 지정 시 폰트 이름 설정
};
#pragma pack(pop)

static struct tag_DlgInOut{
	ATOM IPAtom;
	ATOM PortAtom;
};

LRESULT CreateCustomDialog(struct tag_CustomDialog Template, HWND hOwner, LPVOID lpArg);

/*	
	// 전역 변수로 아래와 같이 읽기 전용 문자열을 만들었으나 런타임 시간에 배열 첨자 연산이 추가되고 보안(잘못된 메모리 접근) 위험이 있기 때문에 매크로 상수와 매크로 함수를 이용하기로 한다.
	// 그리고 여러 래퍼 매크로 함수를 만들 수 있기 때문에 훨씬 편하다.
	// 다만, 매크로의 경우 컴파일 전처리 과정에 포함된다. 즉, 코드를 생성하지 않고 메모리에서 이미 작성된 소스를 재작성(변환: 패스 과정)하므로 디버깅이 어려우니 주의해야 한다.
	// 보안 문제는 상당히 저수준을 요하기 때문에 신경 쓸 필요없다.

// Debug Message Index
#define FAILED_CREATETHREAD				0
#define FAILED_LOADTHREAD				1
#define FAILED_CREATETEVENT				2
#define FAILED_LOADEVENT				3
#define FAILED_CREATESECTION			4
#define FAILED_LOADSECTION				5
#define FAILED_READSECTION				6
#define FAILED_CREATETIMER				7
#define FAILED_LOADTIMER				8
#define FAILED_CREATECOMPLETIONPORT		9
#define FAILED_LOADCOMPLETIONPORT		10
#define FAILED_LOADQUEUE				11
#define FAILED_CREATESOCKET				12
#define FAILED_CONNECTSOCKET			13
// #define FAILED_FUNCTION					14
#define FAILED_ENCODING					15
#define FAILED_LOSTCONNECTION			16

// Debug Message
typedef LPCTSTR* FMLPOINTER;
LPCTSTR FailedMessageList[] = {
	TEXT("Failed to create thread."),
	TEXT("Failed to load thread."),
	TEXT("Failed to create event object."),
	TEXT("Failed to load event object."),
	TEXT("Failed to create the shared section."),
	TEXT("Failed to load the shared section."),
	TEXT("Failed to read the Share section."),
	TEXT("Failed to create timer object."),
	TEXT("Failed to load timer object."),
	TEXT("Failed to create network communication model."),
	TEXT("Failed to load network communication model."),
	TEXT("An error occurred in removing an I/O packet from the system queue and retrieving the data in the packet."),
	TEXT("Failed to create IP socket."),
	TEXT("Failed to socket connection."),
	TEXT("An error occurred while executing the function."),
	TEXT("Failed to change the encoding of the character."),
	TEXT("The connection to the server has been lost."),
};
FMLPOINTER FML = FailedMessageList;
*/

// Debug Message Macro
#define FAILED_CREATETHREAD				Failed to create thread.
#define FAILED_LOADTHREAD				Failed to load thread.
#define FAILED_CREATETEVENT				Failed to create event object.
#define FAILED_LOADEVENT				Failed to load event object.
#define FAILED_CREATESECTION			Failed to create the shared section.
#define FAILED_LOADSECTION				Failed to load the shared section.
#define FAILED_READSECTION				Failed to read the Share section.
#define FAILED_CREATETIMER				Failed to create timer object.
#define FAILED_LOADTIMER				Failed to load timer object.
#define FAILED_CREATECOMPLETIONPORT		Failed to create network communication model.
#define FAILED_LOADCOMPLETIONPORT		Failed to load network communication model.
#define FAILED_LOADQUEUE				An error occurred in removing an I/O packet from the system queue and retrieving the data in the packet.
#define FAILED_CREATESOCKET				Failed to create IP socket.
#define FAILED_CONNECTSOCKET			Failed to socket connection.
#define FAILED_FUNCTION					An error occurred while executing the function.
#define FAILED_ENCODING					Failed to change the encoding of the character.
#define FAILED_LOSTCONNECTION			The connection to the server has been lost.

// Debug Message Concatenate Macro
#define MSG(str)						#str
#define VALUE(macro)					macro
#define ERROR_MSG_GENERIC(text)			TEXT(MSG(text))
#define ERROR_MSG_FUNCTION(func, text)	TEXT("["MSG(func)"] :"MSG(VALUE(text)))		

// Wrapper Macro
#define ERR(a)							ERROR_MSG_GENERIC(a)
#define ERRFUNC(a,b)					ERROR_MSG_FUNCTION(a,b)
/* 
	모듈 내부에서 생성한 핸들은 해당 프로세스가 생성한 스레드간에 공유될 수 있으나 다른 프로세스와는 공유할 수 없다.
	즉, 서버와 클라이언트가 동일한 객체의 핸들을 소유하고 있어야 할 필요가 없으며 서버는 서버 고유의 입출력 완료 포트 객체를 소유하고
	클라이언트는 클라이언트 고유의 입출력 완료 포트 객체를 소유하면 된다.
*/
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow){
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0){ Exit(); }

	INITCOMMONCONTROLSEX icex = {sizeof(icex), ICC_DATE_CLASSES};
	if(!InitCommonControlsEx(&icex)){ Exit(); }

	HANDLE hCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0,0);
	if(hCP == NULL){ Exit(); }

	HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, TIMERID);
	if(hTimer == NULL){ Exit(); }

	SOCKET*	sock = (SOCKET*)HeapAlloc(GetProcessHeap(), 0, sizeof(SOCKET));
	if(sock == NULL){ Exit(); }

	DWORD TlsIndex = TlsAlloc();
	if(TlsIndex == TLS_OUT_OF_INDEXES){ Exit(); }

	HANDLE hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SOCKET*) + sizeof(HANDLE) + sizeof(DWORD), MAPPINGID);
	if(hMap == NULL){ Exit(); }

	TCHAR* wrPtr = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, sizeof(SOCKET*));
	if(wrPtr == NULL){ Exit(); }

	memcpy(wrPtr, &sock, sizeof(sock));
	memcpy((wrPtr + sizeof(sock)), &hCP, sizeof(hCP));
	memcpy(wrPtr + sizeof(sock) + sizeof(hCP), &TlsIndex, sizeof(TlsIndex));

	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		NULL,
		CLASSNAME
	};
	RegisterClass(&wc);

	wc.lpfnWndProc = DisplayPannelProc;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = TCLASSNAME;
	RegisterClass(&wc);

	wc.lpfnWndProc = EditPannelProc;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = LCLASSNAME;
	RegisterClass(&wc);

	wc.lpfnWndProc = BtnPannelProc;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = RCLASSNAME;
	RegisterClass(&wc);

	HWND hWnd = CreateWindow(
				CLASSNAME,
				TEXT("Client"),
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				NULL,
				(HMENU)NULL,
				hInst,
				NULL
			);

	ShowWindow(hWnd, nCmdShow);

	HANDLE hRecvEvent, hSendEvent, hConnectEvent;
	hRecvEvent = CreateEvent(NULL, FALSE, FALSE, RECVEVENTID);
	if(hRecvEvent == NULL){ Exit(); }
	hSendEvent = CreateEvent(NULL, FALSE, FALSE, SENDEVENTID);
	if(hSendEvent == NULL){ Exit(); }
	hConnectEvent = CreateEvent(NULL, TRUE, FALSE, CONNECTEVENTID);
	if(hConnectEvent == NULL){ Exit(); }

	MSG msg;
	while(GetMessage(&msg, NULL, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CloseHandle(hTimer);
	CloseHandle(hRecvEvent);
	CloseHandle(hSendEvent);
	CloseHandle(hConnectEvent);

	HeapFree(GetProcessHeap(), 0, sock);
	TlsFree(TlsIndex);

	UnmapViewOfFile(wrPtr);
	CloseHandle(hMap);

	WSACleanup();
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static HWND hTopPannel, hLeftPannel, hRightPannel;
	static BOOL bSplit = FALSE;
	static int DisplayPannelRatio;

	int DisplayHeight;

	PAINTSTRUCT ps;
	HDC hdc;

	RECT rt, srt;
	static RECT mrt;

	POINT pt;
	HMENU hMenu, hPopup;
	
	static DWORD dwThreadID;
	HANDLE hClientMainThread, hConnectEvent;
	DWORD dwThread, dwConnect, dwExitCode;

	static struct tag_CustomDialog MyDlg = {
		WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_3DLOOK | DS_SETFONT,
		0x0,
		0,
		0,0,0,0,
		0,
		0,
		DLGTITLE,
		8,
		DLGFONT,
	};

	static struct tag_DlgInOut DlgInOut;

	switch(iMessage){
		case WM_CREATE:
			hTopPannel = CreateWindowEx(WS_EX_CLIENTEDGE, TCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0,0,0,0, hWnd, (HMENU)NULL, GetModuleHandle(NULL), NULL);
			hLeftPannel = CreateWindowEx(WS_EX_COMPOSITED | WS_EX_CLIENTEDGE, LCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0,0,0,0, hWnd, (HMENU)NULL, GetModuleHandle(NULL), NULL);
			hRightPannel = CreateWindowEx(WS_EX_COMPOSITED | WS_EX_CLIENTEDGE, RCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0,0,0,0, hWnd, (HMENU)NULL, GetModuleHandle(NULL), NULL);
			DisplayPannelRatio = 70;

			hMenu = CreateMenu();
			hPopup = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hPopup, TEXT("설정"));
			AppendMenu(hPopup, MF_STRING, ID_CONFIGCONNECT,TEXT("연결 설정"));
			SetMenu(hWnd, hMenu);

			SetRect(&mrt, 0,0, 530, 640);
			AdjustWindowRect(&mrt, GetWindowLongPtr(hWnd, GWL_STYLE), GetMenu(hWnd) != NULL);
			SetWindowPos(hWnd, NULL, mrt.left, mrt.top, mrt.right - mrt.left, mrt.bottom - mrt.top, SWP_NOZORDER | SWP_NOMOVE);
			return 0;

		case WM_INITMENU:
			/* TODO: 연결 상태를 체크 모양으로 표시 */ 
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case ID_CONFIGCONNECT:
					switch(HIWORD(wParam)){
						case BN_CLICKED:
							if(IDOK == CreateCustomDialog(MyDlg, hWnd, &DlgInOut)){
								/* TODO: 아이템 확인 후 데이터 유효한지 검사하고 스레드 실행 */
								hClientMainThread = CreateThread(NULL, 0, ClientMain, &DlgInOut, 0, &dwThreadID);
								if(hClientMainThread){ 
									CloseHandle(hClientMainThread);
									SetTimer(hWnd, 1, 60000, NULL);
								}
								else{ ErrorMessage(ERR(FAILED_CREATETHREAD)); }
							}
							break;
					}
					break;
			}
			return 0;

		case WM_TIMER:
			switch(wParam){
				case 1:
					/* 스레드가 종료된 이후 새로운 ID가 할당될 수 있다. 즉, 유효하지 않을 수 있다.  */
					hClientMainThread = OpenThread(THREAD_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwThreadID);
					if(hClientMainThread == NULL){ ErrorMessage(ERR(FAILED_LOADTHREAD)); break; }

					dwThread = WaitForSingleObject(hClientMainThread, 0);
					if(dwThread != WAIT_OBJECT_0){ CloseHandle(hClientMainThread); break; }

					if(GetExitCodeThread(hClientMainThread, &dwExitCode)){
						/* 에러 코드 확인 후 분기 */
						switch(dwExitCode){
							default:
								// 연결 상태일 경우 스레드 재생성
								hConnectEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, CONNECTEVENTID);
								if(hConnectEvent == NULL){ ShowMessage(TEXT("The data required for communication has not been initialized. Please re-run the program.")); ExitProcess(GetLastError()); }

								dwConnect = WaitForSingleObject(hConnectEvent, 0);
								if(dwConnect == WAIT_OBJECT_0){
									ShowMessage(TEXT("An error occurred during communication with the server and the operation was terminated.\r\nSince this is an error that can be fixed, click the OK button and it will run again."));
									hClientMainThread = CreateThread(NULL, 0, ClientMain, NULL, 0, &dwThreadID);
									if(hClientMainThread){ CloseHandle(hClientMainThread); }
								}
								break;
						}
					}else{
						return ErrorMessage(ERRFUNC(GetExitCodeThread(), FAILED_FUNCTION));
					}

					if(hClientMainThread){ CloseHandle(hClientMainThread); }
					break;
			}
			return 0;

		case WM_SIZE:
			if(wParam != SIZE_MINIMIZED){
				GetClientRect(hWnd, &rt);
				int iWidth = rt.right - rt.left;
				int iHeight = rt.bottom - rt.top;

				/* Top : Bottom = 7 : 3 */
				DisplayHeight = iHeight * DisplayPannelRatio / 100;
				SetRect(&srt, rt.left, rt.top, iWidth, DisplayHeight - THICKNESS);
				SetWindowPos(hTopPannel, NULL, srt.left, srt.top, srt.right, srt.bottom, SWP_NOZORDER);
				SetRect(&srt, rt.left, DisplayHeight, iWidth * 0.7f, iHeight - DisplayHeight);
				SetWindowPos(hLeftPannel, NULL, srt.left, srt.top, srt.right, srt.bottom, SWP_NOZORDER);
				SetRect(&srt, iWidth * 0.7f, DisplayHeight, iWidth * 0.3f, iHeight - DisplayHeight);
				SetWindowPos(hRightPannel, NULL, srt.left, srt.top, srt.right, srt.bottom, SWP_NOZORDER);
			}
			return 0;

		case WM_LBUTTONDOWN:
			pt.x=LOWORD(lParam);
			pt.y=HIWORD(lParam);
			bSplit = CheckSplitBar(hWnd, pt, DisplayPannelRatio);
			if(bSplit){
				SetCapture(hWnd);
			}
			return 0;

		case WM_SETCURSOR:
			if(LOWORD(lParam) == HTCLIENT){
				GetCursorPos(&pt);
				ScreenToClient(hWnd, &pt);
				if(CheckSplitBar(hWnd, pt, DisplayPannelRatio)){
					SetCursor(LoadCursor(NULL, IDC_SIZENS));
					return TRUE;
				}
			}
			break;

		case WM_MOUSEMOVE:
			if(bSplit){
				GetClientRect(hWnd, &rt);

				DisplayHeight = (int)(short)HIWORD(lParam);
				DisplayPannelRatio = DisplayHeight * 100 / rt.bottom;

				#define min(a,b) (((a) < (b)) ? (a) : (b))
				#define max(a,b) (((a) > (b)) ? (a) : (b))
				DisplayPannelRatio = min(max(DisplayPannelRatio, 20), 80);
				SendMessage(hWnd,WM_SIZE,SIZE_RESTORED,0);
			}
			return 0;

		case WM_LBUTTONUP:
			bSplit = FALSE;
			ReleaseCapture();
			return 0;

		case WM_DESTROY:
			KillTimer(hWnd, 1);
			if(FindAtomA((LPCSTR)DlgInOut.IPAtom) != 0) { DeleteAtom(DlgInOut.IPAtom); }
			if(FindAtomA((LPCSTR)DlgInOut.PortAtom) != 0) { DeleteAtom(DlgInOut.PortAtom); }
			// if(FindAtom((LPTSTR)DlgInOut.IPAtom) != 0) { DeleteAtom(DlgInOut.IPAtom); }
			// if(FindAtom((LPTSTR)DlgInOut.PortAtom) != 0) { DeleteAtom(DlgInOut.PortAtom); }
			if(hClientMainThread != NULL){
				/* TODO: 종료전에 종료 데이터 전송 - OOB보단 일반 데이터 처리가 편리 */
				CloseHandle(hClientMainThread);
			}
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

/* 여분 메모리 이용해도됨 */
static WNDPROC OldEditProc;
LRESULT CALLBACK EditPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static HWND hMsgEdit;
	RECT rt;

	switch(iMessage){
		case WM_CREATE:
			hMsgEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN, 0,0,0,0, hWnd, (HMENU)IDC_EDMSG, GetModuleHandle(NULL), NULL);
			SetFocus(hMsgEdit);
			OldEditProc = (WNDPROC)SetWindowLongPtr(hMsgEdit, GWLP_WNDPROC, (LONG_PTR)EditSubProc);
			return 0;

		case WM_SIZE:
			GetClientRect(hWnd, &rt);
			SetWindowPos(hMsgEdit, NULL, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top, SWP_NOZORDER);
			return 0;

		case WM_COMMAND:
			return 0;

		case WM_DESTROY:
			SetWindowLongPtr(hMsgEdit, GWLP_WNDPROC, (LONG_PTR)OldEditProc);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

/*
   기본 에디트가 완벽한 개행 기능을 제공하므로 직접 작성할 필요없다.
   단순히 반응하고자 하는 문자와 메시지 발생 순간의 키 입력 상태를 조사하여 리턴하고 나머지 처리는 기본 에디트에게 맡기면 된다.

   LShift와 LCtrl키를 누른 상태로 Enter를 입력하면 개행하며 단일키 Enter만 입력하면 메시지를 보낸다.
*/
LRESULT CALLBACK EditSubProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam) {
	static HWND hBtnPannel, hBtn;

	switch (iMessage) {
		case WM_CHAR:
			if(((GetKeyState(VK_LSHIFT) & 0x8000) == 0) && ((GetKeyState(VK_LCONTROL) & 0x8000) == 0) && wParam == '\r'){
				return 0;
			}
			break;

		case WM_KEYDOWN:
			if(hBtn == NULL){
				hBtnPannel = FindWindowEx(GetParent(GetParent(hWnd)), NULL, RCLASSNAME, NULL);
				hBtn = FindWindowEx(hBtnPannel, NULL, TEXT("button"), TEXT("Send"));
			}

			if(((GetKeyState(VK_LSHIFT) & 0x8000) == 0) && ((GetKeyState(VK_LCONTROL) & 0x8000) == 0)&& GetKeyState(VK_RETURN) & 0x8000){
				SendMessage(hBtnPannel, WM_COMMAND, MAKEWPARAM(IDC_BTNSEND, BN_CLICKED), (LPARAM)hBtn);
				return 0;
			}
			break;
	}

	return CallWindowProc(OldEditProc,hWnd,iMessage,wParam,lParam);
}

LRESULT CALLBACK BtnPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static HWND hSendBtn, hEditPannel, hMsgEdit;
	RECT rt;

	char* bufA;
	wchar_t* bufW;
	TCHAR* bufT;
	int LengthW, LengthA;

	static HANDLE hSendEvent, hMap;
	static SOCKET* sock;
	static TCHAR* rdPtr;
	WSABUF wsabuf;

	switch(iMessage){
		case WM_CREATE:
			hSendBtn = CreateWindow(TEXT("button"), TEXT("Send"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0,0,0,0, hWnd, (HMENU)IDC_BTNSEND, GetModuleHandle(NULL), NULL);

			return 0;

		case WM_SIZE:
			GetClientRect(hWnd, &rt);
			SetWindowPos(hSendBtn, NULL, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top, SWP_NOZORDER);
			return 0;
			
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDC_BTNSEND:
					if(HIWORD(wParam) == BN_CLICKED){
						if(hMsgEdit == NULL){
							hEditPannel = FindWindowEx(GetParent(hWnd), NULL, LCLASSNAME, NULL);
							hMsgEdit = FindWindowEx(hEditPannel, NULL, TEXT("edit"), NULL);
						}

						LengthW = SendMessage(hMsgEdit, WM_GETTEXTLENGTH, 0, 0);
						bufT = (TCHAR*)malloc(LengthW + 1);
						SendMessage(hMsgEdit, WM_GETTEXT, LengthW+1, (LPARAM)bufT);
						LengthA = WideCharToMultiByte(CP_ACP, 0, bufT, -1, NULL, 0, NULL, NULL);
						if(LengthA == 0){ return ErrorMessage(ERR(FAILED_ENCODING)); }
						if(WideCharToMultiByte(CP_ACP, 0, bufT, -1, bufA, LengthA, NULL, NULL) == 0){ return ErrorMessage(ERR(FAILED_ENCODING)); }
						wsabuf.buf = bufA;
						wsabuf.len = LengthA;

						// TODO : 연결상태 확인 - MenuItem

						// TODO : 정보 가져오기
						if(hMap == NULL){
							hMap = OpenFileMapping(FILE_MAP_READ, FALSE, MAPPINGID);
							if(hMap == NULL){ return ErrorMessage(ERR(FAILED_LOADSECTION)); }
						}

						if(rdPtr == NULL){
							rdPtr = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(SOCKET*) + sizeof(HANDLE) + sizeof(DWORD));
							if(rdPtr == NULL){ return ErrorMessage(ERR(FAILED_READSECTION)); }

							memcpy(&sock, rdPtr, sizeof(SOCKET*));
						}

						if(hSendEvent == NULL){
							hSendEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, SENDEVENTID);
							if(hSendEvent == NULL){ return ErrorMessage(ERR(FAILED_LOADEVENT)); }
						}

						if(sock != NULL){
							if(SetEvent(hSendEvent)){
								// TODO: 함수 호출


								// TODO: 데이터 전송 후 MsgEdit 기본 작업
								SetWindowText(hMsgEdit, TEXT(""));
							}else{
								return ErrorMessage(ERRFUNC(SetEvent(), FAILED_FUNCTION));
							}
						}else{
							return ErrorMessage(ERR(FAILED_CREATESOCKET));
						}

						// TODO: 공통 기본 작업
						SetFocus(hMsgEdit);
					}
					break;
			}
			return 0;

		case WM_DESTROY:
			if(hSendEvent){CloseHandle(hSendEvent);}
			if(rdPtr){UnmapViewOfFile(rdPtr);}
			if(hMap){CloseHandle(hMap);}
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT CALLBACK DisplayPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc;

	switch(iMessage){
		case WM_CREATE:
			return 0;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

/* DialogProc */
INT_PTR CALLBACK DialogProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static HWND hDlgControl[6];
	RECT prt, srt;
	LONG X,Y, iDlgWidth, iDlgHeight;

	DWORD dwAddress, dwPort;
	BOOL bSucceeded;
	wchar_t ResultW[0x100];
	char ResultA[0x50];

	static struct tag_DlgInOut* DlgInOut;

	switch(iMessage){
		case WM_INITDIALOG:
			#define OKCONTROL 0
			#define CANCELCONTROL 1
			#define IPSTATIC 2
			#define PORTSTATIC 3
			#define IPCONTROL 4
			#define PORTCONTROL 5

			DlgInOut = (struct tag_DlgInOut*)lParam;

			/* 중앙 정렬 */
			GetWindowRect(hWnd, &srt);
			SetRect(&srt, srt.left, srt.top, srt.left + 300, srt.top + 180);
			MapDialogRect(hWnd, &srt);
			AdjustWindowRect(&srt, WS_POPUPWINDOW | WS_DLGFRAME, FALSE);
			GetWindowRect(GetWindow(hWnd, GW_OWNER), &prt);

			iDlgWidth = srt.right - srt.left;
			iDlgHeight = srt.bottom - srt.top;
			X = prt.left + ((prt.right - prt.left) - iDlgWidth) / 2;
			Y = prt.top + ((prt.bottom  - prt.top) - iDlgHeight) / 2;
			SetRect(&srt, X, Y, X + iDlgWidth, Y+ iDlgHeight);

			SetWindowPos(hWnd, NULL, srt.left, srt.top, srt.right - srt.left, srt.bottom - srt.top, SWP_NOZORDER);

			/* 컨트롤 생성 */
			SetRect(&srt, 6, 6, 6 + 120, 6 + 18);
			MapDialogRect(hWnd, &srt);
			hDlgControl[IPSTATIC] = CreateWindow(TEXT("static"), TEXT("IP"), WS_VISIBLE | WS_CHILD | SS_LEFT, srt.left, srt.top, srt.right - srt.left, srt.bottom - srt.top, hWnd, (HMENU)NULL, GetModuleHandle(NULL), NULL);

			SetRect(&srt, 6, 26, 6 + 120, 26 + 18);
			MapDialogRect(hWnd, &srt);
			hDlgControl[IPCONTROL] = CreateWindowEx(WS_EX_CLIENTEDGE, WC_IPADDRESS, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP, srt.left, srt.top, srt.right - srt.left, srt.bottom - srt.top, hWnd, (HMENU)IDC_DLGEDIPADDRESS, GetModuleHandle(NULL), NULL);
			SendMessage(hDlgControl[IPCONTROL], IPM_SETRANGE, (WPARAM)0, (LPARAM)MAKEIPRANGE(127, 255));		// 0번째 필드는 상한, 하한 설정

			SetRect(&srt, 6, 46, 6 + 120, 46 + 18);
			MapDialogRect(hWnd, &srt);
			hDlgControl[PORTSTATIC] = CreateWindow(TEXT("static"), TEXT("PORT"), WS_VISIBLE | WS_CHILD | SS_LEFT, srt.left, srt.top, srt.right - srt.left, srt.bottom - srt.top, hWnd, (HMENU)NULL, GetModuleHandle(NULL), NULL);

			SetRect(&srt, 6, 66, 6 + 120, 66 + 18);
			MapDialogRect(hWnd, &srt);
			hDlgControl[PORTCONTROL] = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT(""), WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP, srt.left, srt.top, srt.right - srt.left, srt.bottom - srt.top, hWnd, (HMENU)IDC_DLGEDPORT, GetModuleHandle(NULL), NULL);
			SendMessage(hDlgControl[PORTCONTROL], EM_LIMITTEXT, (WPARAM)5, (LPARAM)0);							// Port는 최대 5자리까지만 입력

			SetRect(&srt, 190, 160, 190 + 50, 160 + 18);
			MapDialogRect(hWnd, &srt);
			hDlgControl[OKCONTROL] = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("button"), TEXT("OK"), WS_VISIBLE | WS_CHILD | WS_GROUP | WS_TABSTOP | BS_DEFPUSHBUTTON, srt.left, srt.top, srt.right - srt.left, srt.bottom - srt.top, hWnd, (HMENU)IDOK, GetModuleHandle(NULL), NULL);

			SetRect(&srt, 244, 160, 244 + 50, 160 + 18);
			MapDialogRect(hWnd, &srt);
			hDlgControl[CANCELCONTROL] = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("button"), TEXT("Cancel"), WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | BS_PUSHBUTTON, srt.left, srt.top, srt.right - srt.left, srt.bottom - srt.top, hWnd, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);
			return TRUE;

			// typedef struct tagNMIPADDRESS {
			// 	NMHDR	hdr;
			// 	int		iField;
			// 	int		iValue;
			// }NMIPADDRESS, *LPNMIPADDRESS;

			/*
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->code){
				case IPN_FIELDCHANGED:
					((LPNMIPADDRESS)lParam)->;
					return TRUE;
			}
			break;
			*/

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDC_DLGEDPORT:
					switch(HIWORD(wParam)){
						case EN_SETFOCUS:
							SendMessage(hDlgControl[PORTCONTROL], EM_SETSEL, (WPARAM)0, (LPARAM)-1);
							return TRUE;
					}
					break;

				case IDOK:
					#define RESERVE		1024
					#define WELLKNOWN	1024
					#define DYNAMIC		0xC000		// 49152
					#define PRIVATE		0xC000		// 49152
					if(SendMessage(hDlgControl[IPCONTROL], IPM_GETADDRESS, 0, (LPARAM)&dwAddress) != 4){ 
						ShowMessage(TEXT("Invalid IP.\r\nPlease fill it out correctly and try again.")); 
						return TRUE;
					}

					if(GetDlgItemInt(hWnd, IDC_DLGEDPORT, &bSucceeded, FALSE) < RESERVE){ 
						return TRUE;
					}

					if(GetDlgItemInt(hWnd, IDC_DLGEDPORT, &bSucceeded, FALSE) >= DYNAMIC ){
						ShowMessage(TEXT("You can't use port numbers between 49152 ~ 65535.\r\nPlease try again (Error: Private Zone)"));
						return TRUE;
					}

					StringCbPrintf(ResultW, sizeof(ResultW), TEXT("%d.%d.%d.%d"), FIRST_IPADDRESS(dwAddress), SECOND_IPADDRESS(dwAddress), THIRD_IPADDRESS(dwAddress), FOURTH_IPADDRESS(dwAddress));
					if(WideCharToMultiByte(CP_ACP, 0, ResultW, -1, ResultA, 0x50, NULL, NULL) == 0){
						ErrorMessage(ERR(FAILED_ENCODING));
						return TRUE;
					}

					dwPort = (DWORD)GetDlgItemInt(hWnd, IDC_DLGEDPORT, &bSucceeded, FALSE);
					DlgInOut->IPAtom = AddAtomA(ResultA);									// 0xFFFF까지 유효범위 대소문자 구분
					// DlgInOut->IPAtom = AddAtom(ResultW);
					if(DlgInOut->IPAtom == 0){
						ErrorMessage(ERRFUNC(AddAtom(), FAILED_FUNCTION));
						return TRUE;
					}

					DlgInOut->PortAtom = AddAtomA((LPCSTR)(WORD)(DWORD)(dwPort));			// 0xBFFF까지 유효범위 #dddd형태
					// DlgInOut->PortAtom = AddAtom((LPTSTR)(WORD)(DWORD)dwPort);
					if(DlgInOut->PortAtom == 0){
						ErrorMessage(ERRFUNC(AddAtom(), FAILED_FUNCTION));
						return TRUE;
					}

				case IDCANCEL:
					EndDialog(hWnd, LOWORD(wParam));
					return TRUE;
			}
			break;
	}

	return FALSE;
}

/* Multi Thread */
struct tag_Info{
	SOCKET sock;
	HANDLE hMap, hCP;
	DWORD TlsIndex;			// 추가 및 할당은 완료, 필요시 사용
};

DWORD WINAPI ClientMain(LPVOID lpArg){
	HANDLE hMap, hCP, hConnectEvent;
	SOCKET *sock;
	TCHAR* rdPtr;
	DWORD TlsIndex;

	struct tag_DlgInOut* DlgInOut = (struct tag_DlgInOut*)lpArg;

	hMap = OpenFileMapping(FILE_MAP_READ, FALSE, MAPPINGID);
	if(hMap == NULL){ return ErrorMessage(ERR(FAILED_LOADSECTION));}

	rdPtr = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(SOCKET*) + sizeof(HANDLE) + sizeof(DWORD));
	if(rdPtr == NULL){ return ErrorMessage(ERR(FAILED_READSECTION));}

	memcpy(&sock, rdPtr, sizeof(SOCKET*));
	memcpy(&hCP, rdPtr + sizeof(SOCKET*) + 1, sizeof(HANDLE));
	memcpy(&TlsIndex, rdPtr + sizeof(SOCKET*) + sizeof(HANDLE) + 1, sizeof(DWORD));

	/* 
		핸들은 카운팅 횟수를 관리하기 때문에 메인 컨텍스트에서 핸들을 닫지 않는 한 인스턴스화 된 객체는 유지된다.
		핸들을 복사하는 이유는 카운팅 횟수를 유지하기 위해서이며 
	*/
	// UnmapViewOfFile(rdPtr);
	// CloseHandle(hMap);

	*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(*sock == INVALID_SOCKET){ return WSAErrorMessage(ERR(FAILED_CREATESOCKET)); }

	struct sockaddr_in Server;
	// TCHAR IPAddressW[INET_ADDRSTRLEN * 2];
	// TCHAR szPortW[0x10 * 2];
	char IPAddressA[INET_ADDRSTRLEN];
	char szPortA[0x10];

	// if(GetAtomName(DlgInOut->IPAtom, IPAddressW, INET_ADDRSTRLEN * 2) == 0){ return ErrorMessage(); }
	// if(GetAtomName(DlgInOut->PortAtom, szPortW, 0x10 * 2) == 0){ return ErrorMessage(); }
	// if(WideCharToMultiByte(CP_UTF8, 0, IPAddressW, -1, IPAddressA, INET_ADDRSTRLEN, NULL, NULL) == 0){ return ErrorMessage(); }
	// short unsigned int uhdPort = _wtoi(szPortW + 1);

	if(GetAtomNameA(DlgInOut->IPAtom, IPAddressA, INET_ADDRSTRLEN) == 0){ return ErrorMessage(ERRFUNC(GetAtomName(), FAILED_FUNCTION)); }
	if(GetAtomNameA(DlgInOut->PortAtom, szPortA, 0x10) == 0){ return ErrorMessage(ERRFUNC(GetAtomName(), FAILED_FUNCTION)); }
	short unsigned int uhdPort = atoi(szPortA + 1);

	Server.sin_family = AF_INET;
	inet_pton(AF_INET, IPAddressA /* SERVERIP */, &Server.sin_addr);
	Server.sin_port = htons(uhdPort /* SERVERPORT */);

	int ret = connect(*sock, (struct sockaddr*)&Server, sizeof(Server));
	if(ret == SOCKET_ERROR){ return WSAErrorMessage(ERR(FAILED_CONNECTSOCKET));}

	if(CreateIoCompletionPort((HANDLE)(*sock), hCP, 0, 0) == NULL){ return ErrorMessage(ERR(FAILED_LOADCOMPLETIONPORT)); }

	hConnectEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, CONNECTEVENTID);
	if(hConnectEvent == NULL){ return ErrorMessage(ERR(FAILED_LOADEVENT)); }

	if(SetEvent(hConnectEvent)){
		CloseHandle(hConnectEvent);
	}else{
		return ErrorMessage(ERRFUNC(SetEvent(), FAILED_FUNCTION));
	}

	DWORD dwThreadID;
	struct tag_Info Info = {*sock, hMap, hCP, 0};
	HANDLE hWorkerThread = CreateThread(NULL, 0, WorkerThread, &Info, 0, &dwThreadID);
	if(hWorkerThread == NULL){ return ErrorMessage(ERR(FAILED_CREATETHREAD)); }

	HANDLE hTimer = OpenWaitableTimer(TIMER_MODIFY_STATE | TIMER_QUERY_STATE | SYNCHRONIZE, FALSE, TIMERID);
	if(hTimer == NULL){ return ErrorMessage(ERR(FAILED_LOADTIMER)); }

	#define PERIOD 60000
	LARGE_INTEGER DueTime = {0,};
	SetWaitableTimer(hTimer, &DueTime, (LONG)PERIOD, NULL, NULL, FALSE);

	DWORD dwThread, dwTimer, dwExitCode;

	while(1){
		/* 60초마다 실행 */
		dwTimer = WaitForSingleObject(hTimer, INFINITE);
		dwThread = WaitForSingleObject(hWorkerThread, 0);

		if(dwThread != WAIT_OBJECT_0){ continue; }
		if(dwTimer == WAIT_FAILED || dwThread == WAIT_FAILED){ ErrorMessage(ERRFUNC(WaitForSingleObject(), FAILED_FUNCTION)); continue;}
		if(GetExitCodeThread(hWorkerThread, &dwExitCode) == FALSE){ ErrorMessage(ERRFUNC(GetExitCodeThread(), FAILED_FUNCTION)); continue; }

		/* 에러 점검 */
		switch(dwExitCode){
			/* OpenEvent Failed */
			case ERROR_INVALID_HANDLE:
				ShowMessage(TEXT("The data required for communication has not been initialized.\r\nPlease re-run the program."));
				ExitProcess(GetLastError());
				break;

			default:
				break;
		}
	}
		
	return 0;
}

DWORD WINAPI WorkerThread(LPVOID lpArg){
	/* 여기서 핸들 전달하면 유효하지 않은 값으로 나오는 것으로 보임 */
	struct tag_Info* Info = (struct tag_Info*)lpArg;

	OVERLAPPED ov;
	HANDLE hSendEvent, hConnectEvent;
	DWORD dwTransferred, dwEvent, dwError;

	hSendEvent = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, SENDEVENTID);
	if(hSendEvent == NULL){ return ErrorMessage(ERR(FAILED_LOADEVENT)); }

	hConnectEvent = OpenEvent(SYNCHRONIZE, FALSE, CONNECTEVENTID);
	if(hConnectEvent == NULL){ return ErrorMessage(ERR(FAILED_LOADEVENT)); }

	while(1){
		dwEvent = WaitForSingleObject(hConnectEvent, 0);
		if(dwEvent == WAIT_OBJECT_0){
			if(GetQueuedCompletionStatus(Info->hCP, &dwTransferred, NULL, (LPOVERLAPPED*)&ov, INFINITE)){
				dwEvent = WaitForSingleObject(hSendEvent, 0);

				switch(dwEvent){
					case WAIT_TIMEOUT:
						//TODO : 통신 작업
						/* Recv Event */
						break;

					case WAIT_OBJECT_0:
						/* Send Event */
						break;

					case WAIT_FAILED:
						/* 구분 불가능한 상태이므로 에러 코드 출력 후 루프 재실행*/
						ErrorMessage(ERRFUNC(GetCompletionStatus(), FAILED_FUNCTION));
						break;
				}
			}else{
				/* 호출 처리중 CompletionPort 핸들 닫히면 실패하고 FALSE 반환 */
				dwError = GetLastError();

				switch(dwError){
					/* 작업 실패 및 저장한 정보가 없는 상태 */
					case ERROR_ABANDONED_WAIT_0:
						if((LPOVERLAPPED*)&ov == NULL){Exit();}
						break;

					case ERROR_SUCCESS:
						break;

					/* 작업 실패 후 실패한 작업에 대한 정보를 저장한 상태 */
					default:
						// TODO: dwTransferred, lpCompletionKey(3), ov 정보 저장 후 스레드 재실행 시 우선 처리
						return ErrorMessage(ERRFUNC(GetQueuedCompltionStatus(), FAILED_LOADQUEUE));
				}
			}
		}else{
			if(hSendEvent){CloseHandle(hSendEvent);}
			if(hConnectEvent){CloseHandle(hConnectEvent);}

			switch(dwEvent){
				case WAIT_TIMEOUT:
					return ErrorMessage(ERR(FAILED_LOSTCONNECTION));

				case WAIT_FAILED:
					return ErrorMessage(ERRFUNC(WaitForSingleObject(), FAILED_FUNCTION));
			}
		}
	}

	return 0;
}

/* Utility */
LRESULT CreateCustomDialog(struct tag_CustomDialog Template, HWND hOwner, LPVOID lpArg){
	HINSTANCE hInst;

	if(hOwner == NULL){ hInst = GetModuleHandle(NULL); }
	else{ hInst = (HINSTANCE)GetWindowLongPtr(hOwner, GWLP_HINSTANCE); }

	return DialogBoxIndirectParamW(hInst, (LPCDLGTEMPLATEW)&Template, hOwner, (DLGPROC)DialogProc, (LPARAM)lpArg);
}

BOOL CheckSplitBar(HWND hWnd, POINT pt, int DisplayPannelRatio){
	RECT rt, srt;
	GetClientRect(hWnd, &rt);

	int DisplayHeight = (rt.bottom - rt.top) * DisplayPannelRatio / 100;
	SetRect(&srt, rt.left, DisplayHeight - THICKNESS, rt.right - rt.left, DisplayHeight);

	if(PtInRect(&srt, pt)){
		return TRUE;
	}

	return FALSE;
}

void ShowMessage(TCHAR* buf){
	MessageBox(HWND_DESKTOP, buf, TEXT("Warning"), MB_ICONWARNING | MB_OK);
}

/* Fatal */
void Exit(TCHAR* msg) { 
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError(); 

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL) == 0) {
		MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_ICONERROR | MB_OK);
		ExitProcess(dw);
	}

	TCHAR buf[0x1000];
	StringCbPrintf(buf, sizeof(buf), TEXT("%s(%d)\r\n%s"), lpMsgBuf, dw, msg);
	MessageBox(HWND_DESKTOP, (LPCTSTR)buf, TEXT("Error"), MB_ICONERROR | MB_OK);

	LocalFree(lpMsgBuf);
	ExitProcess(dw); 
}

void WSAExit(TCHAR* msg) { 
	LPVOID lpMsgBuf;
	int err = WSAGetLastError(); 

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL) == 0) {
		MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_ICONERROR | MB_OK);
		ExitProcess(err);
	}

	TCHAR buf[0x1000];
	StringCbPrintf(buf, sizeof(buf), TEXT("%s(%d)\r\n%s"), lpMsgBuf, err, msg);
	MessageBox(HWND_DESKTOP, (LPCTSTR)buf, TEXT("Error"), MB_ICONERROR | MB_OK);

	LocalFree(lpMsgBuf);
	ExitProcess(err); 
}

/* Nonfatal */
DWORD ErrorMessage(TCHAR* msg){
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError(); 

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL) == 0) {
		MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_ICONWARNING | MB_OK);
		return 0x10000000; /* Custom Error(29bit) : MessageFunction Failed */
	}

	TCHAR buf[0x1000];
	StringCbPrintf(buf, sizeof(buf), TEXT("%s(%d)\r\n%s"), lpMsgBuf, dw, msg);
	MessageBox(HWND_DESKTOP, (LPCTSTR)buf, TEXT("Error"), MB_ICONWARNING | MB_OK);
	LocalFree(lpMsgBuf);

	return dw;
}

DWORD WSAErrorMessage(TCHAR* msg){
	LPVOID lpMsgBuf;
	int err = WSAGetLastError(); 

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL) == 0) {
		MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_ICONWARNING | MB_OK);
		return 0x10000000; /* Custom Error(29bit) : MessageFunction Failed */
	}

	TCHAR buf[0x1000];
	StringCbPrintf(buf, sizeof(buf), TEXT("%s(%d)\r\n%s"), lpMsgBuf, err, msg);
	MessageBox(HWND_DESKTOP, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_ICONWARNING | MB_OK);
	LocalFree(lpMsgBuf);

	return (DWORD)err;
}

