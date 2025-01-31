// 서버와 클라이언트 프로그램 작성시 사용자 정의 에러 타입을 설정하여 적절한 조치를 취할 수 있는 분기를 만드는 것이 좋다.
// 이때 SetLastError 함수를 이용하면 응용 프로그램 수준의 사용자 정의 에러를 설정할 수 있다.
// 이와 관련된 예시는 ErrorMessage 함수 내부의 예외 처리 구문을 참고하자.
#define UNICODE
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#include <commctrl.h>
#include <strsafe.h>

#define DEFAULT_BUFLEN 0x400

// Main Window Control ID
#define IDC_BTNSEND			101
#define IDC_EDMSG			201

// Dialog Control ID
#define IDC_DLGEDIPADDRESS	1101
#define IDC_DLGEDPORT		1201

// Menu ID
#define ID_CONFIGCONNECT	40001

#define THICKNESS			3
//#define THICKNESS			GetSystemMetrics(SM_CYSIZEFRAME)
//#define SERVERIP			"127.0.0.1"
//#define SERVERPORT		9000

// ID of the Sync Object.
#define RECVEVENTID			TEXT("Local\\ExampleRecvEvent_25_01_20")
#define SENDEVENTID			TEXT("Local\\ExampleSendEvent_25_01_20")
#define CONNECTEVENTID		TEXT("Local\\ExampleConnectEvent_25_01_20")

#define MAPPINGID			TEXT("Local\\ExampleMappingObject_25_01_20")
#define TIMERID				TEXT("Local\\ExampleTimerObject_25_01_20")

#define CLASSNAME			TEXT("ClientProgramLayout")
#define TCLASSNAME			TEXT("DisplayPannelClass")
#define LCLASSNAME			TEXT("EditPannelClass")
#define RCLASSNAME			TEXT("ButtonPannelClass")

#define WM_DATATRANSFER		WM_USER+1

// Declare a Procedure
INT_PTR CALLBACK DialogProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DisplayPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditSubProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK BtnPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

// Declare a SubThread
DWORD WINAPI ClientMain(LPVOID lpArg);
DWORD WINAPI SendThread(LPVOID lpArg);
DWORD WINAPI RecvThread(LPVOID lpArg);

// Declare a Util
void Exit(TCHAR* msg); 
void WSAExit(TCHAR* msg);
DWORD ErrorMessage(TCHAR* msg);
DWORD WSAErrorMessage(TCHAR* msg);
void ShowMessage(TCHAR* buf);
BOOL CheckSplitBar(HWND hWnd, POINT pt, int DisplayPannelRatio);

// Indirect Dialog
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
	BOOL bConnect;
};

LRESULT CreateCustomDialog(struct tag_CustomDialog Template, HWND hOwner, LPVOID lpArg);

struct tag_Packet{
	DWORD dwTransferred;
	BOOL bEcho;
};

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
// #define FAILED_FUNCTION				14
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
#define FAILED_LOADSOCKET				Failed to load IP Socket.
#define FAILED_CONNECTSOCKET			Failed to socket connection.
#define FAILED_FUNCTION					An error occurred while executing the function.
#define FAILED_ENCODING					Failed to change the encoding of the character.
#define FAILED_LOSTCONNECTION			The connection to the server has been lost.
#define FAILED_LOADDLL					Failed to load Dynamic Linked Library
#define FAILED_ALLOCATEMEMORY			Failed to allocate memory
#define INVALID_HANDLE_TERMINATED		The resources in the network communication model are invalid.
#define INVALID_HANDLE_ABANDONED_0		The operation was abandoned because the network communication model was invalid. No failed actions will be taken so the process will be terminated normally.
#define INVALID_HANDLE_ABANDONED_1		The operation was abandoned because the network communication model was invalid. Data transfer failed due to an invalid handle but relevant information was saved.
#define ACCESS_VIOLATION				Access violation occurred due to access to incorrect memory space.

// Debug Message Concatenate Macro
#define MSG(str)						#str
#define VALUE(macro)					macro
#define ERROR_MSG_GENERICA(text)		MSG(text)
#define ERROR_MSG_GENERICW(text)		TEXT(MSG(text))
#define ERROR_MSG_FUNCTION(func, text)	TEXT("["MSG(func)"] :"ERROR_MSG_GENERICA(text))

// Wrapper Macro
#define ERR(a)							ERROR_MSG_GENERICW(VALUE(a))
#define ERRFUNC(a,b)					ERROR_MSG_FUNCTION(a,VALUE(b))

/* 
	모듈 내부에서 생성한 핸들은 해당 프로세스가 생성한 스레드간에 공유될 수 있으나 다른 프로세스와는 공유할 수 없다.
	즉, 서버와 클라이언트가 동일한 객체의 핸들을 소유하고 있어야 할 필요가 없으며 서버는 서버 고유의 입출력 완료 포트 객체를 소유하고
	클라이언트는 클라이언트 고유의 입출력 완료 포트 객체를 소유하면 된다.
*/
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow){
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0){ Exit(ERR(FAILED_LOADDLL)); }

	INITCOMMONCONTROLSEX icex = {sizeof(icex), ICC_DATE_CLASSES};
	if(!InitCommonControlsEx(&icex)){ Exit(ERR(FAILED_LOADDLL)); }

	HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, TIMERID);
	if(hTimer == NULL){ Exit(ERR(FAILED_CREATETIMER)); }

	SOCKET*	sock = (SOCKET*)HeapAlloc(GetProcessHeap(), 0, sizeof(SOCKET));
	if(sock == NULL){ Exit(ERR(FAILED_ALLOCATEMEMORY)); }

	DWORD TlsIndex = TlsAlloc();
	if(TlsIndex == TLS_OUT_OF_INDEXES){ Exit(ERR(FAILED_ALLOCATEMEMORY)); }

	HANDLE hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SOCKET) + sizeof(HWND) + sizeof(DWORD), MAPPINGID);
	if(hMap == NULL){ Exit(ERR(FAILED_CREATESECTION)); }

	TCHAR* wrPtr = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0);
	if(wrPtr == NULL){ Exit(ERR(FAILED_READSECTION)); }

	memcpy(wrPtr, sock, sizeof(SOCKET));

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

	memcpy(wrPtr + sizeof(SOCKET*), &hWnd, sizeof(HWND));
	memcpy(wrPtr + sizeof(SOCKET*) + sizeof(HWND), &TlsIndex, sizeof(DWORD));

	HANDLE hRecvEvent, hSendEvent, hConnectEvent;
	hRecvEvent = CreateEvent(NULL, TRUE, FALSE, RECVEVENTID);
	if(hRecvEvent == NULL){ Exit(ERR(FAILED_CREATEEVENT)); }
	hSendEvent = CreateEvent(NULL, TRUE, FALSE, SENDEVENTID);
	if(hSendEvent == NULL){ Exit(ERR(FAILED_CREATEEVENT)); }
	hConnectEvent = CreateEvent(NULL, TRUE, FALSE, CONNECTEVENTID);
	if(hConnectEvent == NULL){ Exit(ERR(FAILED_CREATEEVENT)); }

	MSG msg;
	while(GetMessage(&msg, NULL, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CloseHandle(hRecvEvent);
	CloseHandle(hSendEvent);
	CloseHandle(hConnectEvent);
	CloseHandle(hTimer);

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
	static HANDLE hClientMainThread;
	HANDLE hConnectEvent;
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
	static BOOL bConnect;

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
			if(DlgInOut.bConnect){
				CheckMenuRadioItem(GetSubMenu((HMENU)wParam, 0), 0, 0, 0, MF_BYPOSITION);
			}
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case ID_CONFIGCONNECT:
					switch(HIWORD(wParam)){
						case BN_CLICKED:
							if(IDOK == CreateCustomDialog(MyDlg, hWnd, &DlgInOut)){
								hClientMainThread = CreateThread(NULL, 0, ClientMain, &DlgInOut, 0, &dwThreadID);
								if(hClientMainThread){ 
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
					/* 스레드가 종료된 이후 새로운 ID가 할당될 수 있다. 즉, 유효하지 않을 수 있다. 따라서, 핸들을 유지하는 것이 좋다. */
					// hClientMainThread = OpenThread(THREAD_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwThreadID);
					if(hClientMainThread == NULL){ 
						DlgInOut.bConnect = FALSE;
						ErrorMessage(ERR(FAILED_LOADTHREAD));
						break;
					}

					dwThread = WaitForSingleObject(hClientMainThread, 0);
					if(dwThread != WAIT_OBJECT_0){ break; }

					if(GetExitCodeThread(hClientMainThread, &dwExitCode)){
						/* 에러 코드 확인 후 분기 */
						switch(dwExitCode){
							default:
								// 연결 상태일 경우 스레드 재생성
								hConnectEvent = OpenEvent(SYNCHRONIZE, FALSE, CONNECTEVENTID);
								if(hConnectEvent == NULL){ Exit(TEXT("The data required for communication has not been initialized. Please re-run the program.")); }

								dwConnect = WaitForSingleObject(hConnectEvent, 0);
								if(dwConnect == WAIT_OBJECT_0){
									ShowMessage(TEXT("An error occurred during communication with the server and the operation was terminated.\r\nSince this is an error that can be fixed, click the OK button and it will run again."));
									if(hClientMainThread){ CloseHandle(hClientMainThread); }
									DlgInOut.bConnect = FALSE;
									hClientMainThread = CreateThread(NULL, 0, ClientMain, &DlgInOut, 0, &dwThreadID);
								}
								CloseHandle(hConnectEvent);
								break;
						}
					}else{
						ErrorMessage(ERRFUNC(GetExitCodeThread(), FAILED_FUNCTION));
					}
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
			if(hConnectEvent){CloseHandle(hConnectEvent);}
			if(hClientMainThread){
				/* TODO: 종료전에 종료 데이터 전송 - OOB보단 일반 데이터 처리가 편리 */
				CloseHandle(hClientMainThread);
			}
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

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

	HANDLE hSendEvent, hMap;
	SOCKET sock;
	TCHAR* rdPtr;
	WSABUF wsabuf;
	DWORD dwError, dwSend, dwFlags;
	OVERLAPPED ov;

	struct tag_Packet *Header;

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

						hSendEvent = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, SENDEVENTID);
						if(hSendEvent == NULL){
							return ErrorMessage(ERR(FAILED_LOADEVENT));
						}

						if(WaitForSingleObject(hSendEvent, 0) == WAIT_TIMEOUT){
							SetEvent(hSendEvent);
						}
						/*

						// TODO : 연결상태 확인 - MenuItem

						// TODO : 정보 가져오기
						hMap = OpenFileMapping(FILE_MAP_READ, FALSE, MAPPINGID);
						if(hMap == NULL){ 
							dwError = ErrorMessage(ERR(FAILED_LOADSECTION));
							CloseHandle(hMap);
							return dwError;
						}

						rdPtr = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(SOCKET*));
						if(rdPtr == NULL){
							dwError = ErrorMessage(ERR(FAILED_READSECTION));
							CloseHandle(hMap);
							return dwError;
						}

						memcpy(&sock, rdPtr, sizeof(SOCKET*));

						if(sock != NULL){
							if(SetEvent(hSendEvent)){
								// TODO: 함수 호출
								LengthW = SendMessage(hMsgEdit, WM_GETTEXTLENGTH, 0, 0);
								bufT = (TCHAR*)malloc(LengthW + 1);
								SendMessage(hMsgEdit, WM_GETTEXT, LengthW, (LPARAM)bufT);

								LengthA = WideCharToMultiByte(CP_ACP, 0, bufT, -1, NULL, 0, NULL, NULL);
								if(LengthA == 0){
									dwError = ErrorMessage(ERR(FAILED_ENCODING));
									if(bufT){free(bufT);}
									UnmapViewOfFile(rdPtr);
									CloseHandle(hMap);
									CloseHandle(hSendEvent);
									return dwError;
								}

								bufA = (char*)malloc(LengthA + sizeof(struct tag_Packet));
								if(WideCharToMultiByte(CP_ACP, 0, bufT, -1, bufA, LengthA, NULL, NULL) == 0){ 
									dwError = ErrorMessage(ERR(FAILED_ENCODING));
									if(bufA){free(bufA);}
									if(bufT){free(bufT);}
									UnmapViewOfFile(rdPtr);
									CloseHandle(hMap);
									CloseHandle(hSendEvent);
									return dwError;
								}

								Header = (struct tag_Packet*)bufA;
								Header->dwTransferred = LengthA;
								Header->bEcho = TRUE;
								wsabuf.buf = bufA;
								wsabuf.len = LengthA + sizeof(struct tag_Packet);

								dwFlags = 0;
								memset(&ov, 0, sizeof(ov));
								if(WSASend(*sock, &wsabuf, 1, &dwSend, dwFlags, &ov, NULL) == SOCKET_ERROR){
									if(WSAGetLastError() != WSA_IO_PENDING){
										dwError = WSAErrorMessage(ERRFUNC(WSASend(), FAILED_FUNCTION));
										if(bufA){free(bufA);}
										if(bufT){free(bufT);}
										UnmapViewOfFile(rdPtr);
										CloseHandle(hMap);
										CloseHandle(hSendEvent);
										return dwError;
									}
								}
								// TODO: 데이터 전송 후 MsgEdit 기본 작업
								SetWindowText(hMsgEdit, TEXT(""));
							}else{
								dwError = ErrorMessage(ERRFUNC(SetEvent(), FAILED_FUNCTION));
								UnmapViewOfFile(rdPtr);
								CloseHandle(hMap);
								CloseHandle(hSendEvent);
								return dwError;
							}
						}else{
							dwError = ErrorMessage(ERR(FAILED_LOADSCOKET));
							UnmapViewOfFile(rdPtr);
							CloseHandle(hMap);
							CloseHandle(hSendEvent);
							return dwError;
						}

						if(bufA){free(bufA); bufA = NULL;}
						if(bufT){free(bufT); bufT = NULL;}
						CloseHandle(hSendEvent);
						UnmapViewOfFile(rdPtr);
						CloseHandle(hMap);
						*/

						// TODO: 공통 기본 작업
						SetWindowText(hMsgEdit, TEXT(""));
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

	BYTE* pData;
	static BYTE* pBuffer;
	struct tag_Packet* Header;

	DWORD dwTransferred;
	BOOL bEcho;

	switch(iMessage){
		case WM_CREATE:
			return 0;

		case WM_DATATRANSFER:
			return 0;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			if(pBuffer != NULL){
				TextOutA(hdc, 10, 10, pBuffer, lstrlenA(pBuffer));
			}
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			free(pBuffer);
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

DWORD WINAPI ClientMain(LPVOID lpArg){
	HANDLE hMap, hCP, hConnectEvent;
	DWORD dwLastError;
	SOCKET sock;
	TCHAR* rdPtr;

	struct tag_DlgInOut* DlgInOut = (struct tag_DlgInOut*)lpArg;

	hMap = OpenFileMapping(FILE_MAP_READ, FALSE, MAPPINGID);
	if(hMap == NULL){ return ErrorMessage(ERR(FAILED_LOADSECTION)); }

	rdPtr = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(SOCKET));
	if(rdPtr == NULL){ CloseHandle(hMap); return ErrorMessage(ERR(FAILED_READSECTION));}

	memcpy(&sock, rdPtr, sizeof(SOCKET));

	UnmapViewOfFile(rdPtr);
	CloseHandle(hMap);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET){ return WSAErrorMessage(ERR(FAILED_CREATESOCKET)); }

	struct sockaddr_in Server;
	char IPAddressA[INET_ADDRSTRLEN];
	char szPortA[0x10];

	// 시간되면 에러값 반응하는 콜백 함수나 플래그 활용해서 메세지 처리
	if(GetAtomNameA(DlgInOut->IPAtom, IPAddressA, INET_ADDRSTRLEN) == 0 || GetAtomNameA(DlgInOut->PortAtom, szPortA, 0x10) == 0){
		dwLastError = ErrorMessage(ERRFUNC(GetAtomName(), FAILED_FUNCTION));
		// closesocket(*sock);
		return dwLastError;
	}
	short unsigned int uhdPort = atoi(szPortA + 1);

	Server.sin_family = AF_INET;
	inet_pton(AF_INET, IPAddressA /* SERVERIP */, &Server.sin_addr);
	Server.sin_port = htons(uhdPort /* SERVERPORT */);

	int ret = connect(sock, (struct sockaddr*)&Server, sizeof(Server));
	if(ret == SOCKET_ERROR){
		// WSAECONNREFUSED : 연결 거부
		dwLastError = WSAErrorMessage(ERR(FAILED_CONNECTSOCKET));
		// closesocket(*sock);
		return dwLastError;
	}

	hConnectEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, CONNECTEVENTID);
	if(hConnectEvent == NULL){ 
		dwLastError = ErrorMessage(ERR(FAILED_CONNECTSOCKET));
		// closesocket(*sock);
		return dwLastError;
	}

	if(SetEvent(hConnectEvent)){
		DlgInOut->bConnect = TRUE;
		CloseHandle(hConnectEvent);
	}else{
		dwLastError = ErrorMessage(ERR(FAILED_CONNECTSOCKET));
		// closesocket(*sock);
		return dwLastError;
	}

	// 핸들은 카운팅 횟수를 관리하기 때문에 메인 컨텍스트에서 핸들을 닫지 않는 한 인스턴스화 된 객체는 유지된다.
	hMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, MAPPINGID);
	if(hMap == NULL){ return ErrorMessage(ERR(FAILED_LOADSECTION));}

	TCHAR* wrPtr;
	wrPtr = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, sizeof(SOCKET*));
	if(wrPtr == NULL){ return ErrorMessage(ERR(FAILED_READSECTION));}

	memcpy(wrPtr, &sock, sizeof(SOCKET));

	UnmapViewOfFile(wrPtr);
	CloseHandle(hMap);

	#define RECV	0
	#define SEND	1
	#define ALL		SEND+1
	DWORD dwThreadID[ALL];
	HANDLE hTransferThread[ALL];

	hTransferThread[RECV] = CreateThread(NULL, 0, RecvThread, NULL, 0, &dwThreadID[RECV]);
	hTransferThread[SEND] = CreateThread(NULL, 0, SendThread, NULL, 0, &dwThreadID[SEND]);

	if(hTransferThread[RECV] == NULL || hTransferThread[SEND] == NULL){ 
		dwLastError = ErrorMessage(ERR(FAILED_CREATETHREAD));
		// closesocket(*sock);
		return dwLastError;
	}

	HANDLE hTimer = OpenWaitableTimer(TIMER_MODIFY_STATE | TIMER_QUERY_STATE | SYNCHRONIZE, FALSE, TIMERID);
	if(hTimer == NULL){
		dwLastError = ErrorMessage(ERR(FAILED_LOADTIMER));
		// closesocket(*sock);
		return dwLastError;
	}

	#define PERIOD 60000
	LARGE_INTEGER DueTime = {0,};
	SetWaitableTimer(hTimer, &DueTime, (LONG)PERIOD, NULL, NULL, FALSE);

	DWORD dwThread, dwTimer, dwExitCode;
	BOOL bClientMainExceptLoop = TRUE;

	while(bClientMainExceptLoop){
		// 60초마다 실행
		dwTimer = WaitForSingleObject(hTimer, INFINITE);
		dwThread = WaitForMultipleObjects(2, hTransferThread, FALSE, 0);

		if(dwThread == WAIT_TIMEOUT){ continue; }
		if(dwTimer == WAIT_FAILED || dwThread == WAIT_FAILED){
			Exit(ERRFUNC(Timer or WaitObjects, FAILED_FUNCTION));
			continue;
		}

		if(GetExitCodeThread(hTransferThread[dwThread - WAIT_OBJECT_0], &dwExitCode) == FALSE){
			Exit(ERRFUNC(GetExitCodeThread(), FAILED_FUNCTION));
		} else {
			// 입출력 작업을 담당하는 스레드가 리턴한 에러 점검
			switch(dwExitCode){
				// 실행될 일은 없지만 에러 코드 확인겸 추가
				case STILL_ACTIVE:
				case ERROR_SUCCESS:
					break;

				// 사용자에 의한 정상 종료 
				case WAIT_TIMEOUT:
					ErrorMessage(TEXT("The connection was gracefully terminated by the user."));
					bClientMainExceptLoop = FALSE;
					break;

				// 통신 작업 실행 전 필요한 리소스를 얻지 못한 경우
				default:
					bClientMainExceptLoop = FALSE;

					{
						LPVOID lpMsgBuf;
						TCHAR buf[0x1000];

						if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwExitCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL) == 0) {
							MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_ICONWARNING | MB_OK);
							Exit(ERRFUNC(FormatMessage(), FAILED_FUNCTION));
						}

						StringCbPrintf(buf, sizeof(buf), TEXT("(%d)%s"), lpMsgBuf, dwExitCode);
						MessageBox(HWND_DESKTOP, (LPCTSTR)buf, TEXT("Error"), MB_ICONWARNING | MB_OK);
						LocalFree(lpMsgBuf);
					}
					break;
			}
		}
	}

	if(hTimer){CloseHandle(hTimer);}
	if(hTransferThread[0]){CloseHandle(hTransferThread[0]);}
	if(hTransferThread[1]){CloseHandle(hTransferThread[1]);}
	if(sock){closesocket(sock);}
	//if(hConnectEvent){CloseHandle(hConnectEvent);}
		
	return 0; //dwExitCode;
}

DWORD WINAPI RecvThread(LPVOID lpArg){
	HANDLE hMap, hConnectEvent; // hRecvevent;
	SOCKET sock;
	TCHAR* rdPtr;
	HWND hParent;
	int ret;

	hMap = OpenFileMapping(FILE_MAP_READ, FALSE, MAPPINGID);
	if(hMap == NULL){ return ErrorMessage(ERR(FAILED_LOADSECTION)); }

	rdPtr = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(SOCKET) + sizeof(HWND));
	if(rdPtr == NULL){ CloseHandle(hMap); return ErrorMessage(ERR(FAILED_READSECTION));}

	memcpy(&sock, rdPtr, sizeof(SOCKET));
	memcpy(&hParent, rdPtr + sizeof(SOCKET), sizeof(HWND));

	UnmapViewOfFile(rdPtr);
	CloseHandle(hMap);

	HWND hDisplayWnd = FindWindowEx(hParent, NULL, TCLASSNAME, NULL);
	if(hDisplayWnd == NULL){ return ErrorMessage(TEXT("Not Found Wnd Handle")); }

	// hRecvEvent = OpenEvent(MODIFY_EVENT_STATE | SYNCHRONIZE, FALSE, RECVEVENTID);
	// if(hRecvEvent == NULL){ Exit(ERR(FAILED_LOADEVENT)); }

	hConnectEvent = OpenEvent(SYNCHRONIZE, FALSE, CONNECTEVENTID);
	if(hConnectEvent == NULL){ return ErrorMessage(ERR(FAILED_CONNECTSOCKET)); }

	char buf[0x400];
	struct tag_Packet* Header;
	while(WaitForSingleObject(hConnectEvent, 0) == WAIT_OBJECT_0){
		ret = recv(sock, buf, DEFAULT_BUFLEN, MSG_WAITALL);
		if(ret == 0 || ret == SOCKET_ERROR){
			// 연결이 정상 종료(0) 또는 오류 발생(SOCKET_ERROR)
			break;
		}

		//TODO: 서버랑 프로토콜 확실히 정할것, 이후 Recv Parse 동작 수행
		Header = (struct tag_Packet*)buf;
		int MsgLength = Header->dwTransferred;

		//TODO: 데이터 Parse 후 DisplayWnd에 전달
	}

	if(hConnectEvent){CloseHandle(hConnectEvent);}

	return 0;
}

DWORD WINAPI SendThread(LPVOID lpArg){
	HANDLE hMap, hConnectEvent, hSendEvent;
	SOCKET sock;
	TCHAR* rdPtr;
	HWND hParent;
	int ret;

	hMap = OpenFileMapping(FILE_MAP_READ, FALSE, MAPPINGID);
	if(hMap == NULL){ return ErrorMessage(ERR(FAILED_LOADSECTION)); }

	rdPtr = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(SOCKET) + sizeof(HWND));
	if(rdPtr == NULL){ CloseHandle(hMap); return ErrorMessage(ERR(FAILED_READSECTION));}

	memcpy(&sock, rdPtr, sizeof(SOCKET));
	memcpy(&hParent, rdPtr + sizeof(SOCKET), sizeof(HWND));

	UnmapViewOfFile(rdPtr);
	CloseHandle(hMap);

	HWND hDisplayWnd = FindWindowEx(hParent, NULL, TCLASSNAME, NULL);
	if(hDisplayWnd == NULL){ return ErrorMessage(TEXT("Not Found Wnd Handle")); }

	HWND hEditPannelWnd = FindWindowEx(hParent, NULL, LCLASSNAME, NULL);
	if(hEditPannelWnd == NULL){ return ErrorMessage(TEXT("Not Found Wnd Handle")); }

	HWND hMsgEditWnd = FindWindowEx(hEditPannelWnd, NULL, TEXT("edit"), NULL);
	if(hMsgEditWnd == NULL){ return ErrorMessage(TEXT("Not Found Wnd Handle")); }

	hConnectEvent = OpenEvent(SYNCHRONIZE, FALSE, CONNECTEVENTID);
	if(hConnectEvent == NULL){ return ErrorMessage(ERR(FAILED_CONNECTSOCKET)); }

	hSendEvent = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, SENDEVENTID);
	if(hSendEvent == NULL){ Exit(ERR(FAILED_LOADEVENT)); }

	int Length, ConvertLength, CopyLength, HeaderSize;
	char bufA[DEFAULT_BUFLEN];
	wchar_t bufW[DEFAULT_BUFLEN];

	struct tag_Packet* Header;
	HeaderSize = sizeof(struct tag_Packet);

	while(WaitForSingleObject(hConnectEvent, 0) == WAIT_OBJECT_0){
		WaitForSingleObject(hSendEvent, INFINITE);

		BOOL bUnicode = IsWindowUnicode(hMsgEditWnd);
		Length = SendMessage(hMsgEditWnd, WM_GETTEXTLENGTH, 0,0);
		if(bUnicode){
			CopyLength = SendMessage(hMsgEditWnd, WM_GETTEXT, DEFAULT_BUFLEN, (LPARAM)bufW);
			bufW[CopyLength] = 0;
			ConvertLength = WideCharToMultiByte(CP_ACP, 0, bufW, -1, NULL, 0, NULL, NULL);
			WideCharToMultiByte(CP_ACP, 0, bufW, -1, bufA + HeaderSize, ConvertLength, NULL, NULL);
			CopyLength = ConvertLength + HeaderSize;
		}else{
			CopyLength = SendMessage(hMsgEditWnd, WM_GETTEXT, Length+1, (LPARAM)(bufA + HeaderSize));
			CopyLength += 1;	// NULL 포함
		}

		// 비트맵이나 아이콘일 경우 0
		if(CopyLength == 1 || CopyLength == HeaderSize){
			ResetEvent(hSendEvent);
			ErrorMessage(TEXT("It is not a supported format. You can only send text formats."));
			continue;
		}

		Header = (struct tag_Packet*)bufA;
		Header->dwTransferred = CopyLength;
		ret = send(sock, bufA, DEFAULT_BUFLEN, 0);
		if(ret == SOCKET_ERROR){
			// 오류 발생(SOCKET_ERROR)
			ResetEvent(hSendEvent);
			ErrorMessage(TEXT("Data transfer failed."));
			break;
		}

		//TODO: 전송한 데이터를 DisplayWnd에 전달하고 종료

		ResetEvent(hSendEvent);
	}

	if(hConnectEvent){CloseHandle(hConnectEvent);}
	if(hSendEvent){CloseHandle(hSendEvent);}

	return 0;
}

// Utility
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

void ShowMessage(TCHAR* msg){
	MessageBox(HWND_DESKTOP, msg, TEXT("Warning"), MB_ICONWARNING | MB_OK);
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
	StringCbPrintf(buf, sizeof(buf), TEXT("(%d)%s\r\n%s"), dw, lpMsgBuf, msg);
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
	StringCbPrintf(buf, sizeof(buf), TEXT("(%d)%s\r\n%s"), err, lpMsgBuf, msg);
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
		SetLastError(0x10000000);	/* Custom Error(29bit) : MessageFunction Failed */
		return dw;
	}

	TCHAR buf[0x1000];
	StringCbPrintf(buf, sizeof(buf), TEXT("(%d)%s\r\n%s"), dw, lpMsgBuf, msg);
	MessageBox(HWND_DESKTOP, (LPCTSTR)buf, TEXT("Error"), MB_ICONWARNING | MB_OK);
	LocalFree(lpMsgBuf);

	return dw;
}

DWORD WSAErrorMessage(TCHAR* msg){
	LPVOID lpMsgBuf;
	int err = WSAGetLastError(); 

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL) == 0) {
		MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_ICONWARNING | MB_OK);
		return err;
	}

	TCHAR buf[0x1000];
	StringCbPrintf(buf, sizeof(buf), TEXT("(%d)%s\r\n%s"), err, lpMsgBuf, msg);
	MessageBox(HWND_DESKTOP, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_ICONWARNING | MB_OK);
	LocalFree(lpMsgBuf);

	return (DWORD)err;
}
