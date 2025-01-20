#define _WIN32_WINNT 0x0A00
#define UNICODE

#include <ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>

#define IDC_BTNSEND 101
#define IDC_EDMSG 201
#define ID_CONFIGCONNECT 40001
// #define THICKNESS GetSystemMetrics(SM_CYSIZEFRAME)
#define THICKNESS 3

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000

#define RECVEVENTID TEXT("Local\\ExampleRecvEvent_25_01_20")
#define SENDEVENTID TEXT("Local\\ExampleSendEvent_25_01_20")
#define CONNECTEVENTID TEXT("Local\\ExampleConnectEvent_25_01_20")

#define MAPPINGID TEXT("Local\\ExampleMappingObject_25_01_20")
#define TIMERID TEXT("Local\\ExampleTimerObject_25_01_20")

#define CLASSNAME TEXT("ClientProgramLayout")
#define TCLASSNAME TEXT("DisplayPannelClass")
#define LCLASSNAME TEXT("EditPannelClass")
#define RCLASSNAME TEXT("ButtonPannelClass")


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DisplayPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditSubProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK BtnPannelProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

DWORD WINAPI ClientMain(LPVOID lpArg);
DWORD WINAPI WorkerThread(LPVOID lpArg);

void ShowMessage(TCHAR* buf);

void Exit(); 
void WSAExit();
DWORD ErrorMessage();
DWORD WSAErrorMessage();

BOOL CheckSplitBar(HWND hWnd, POINT pt, int DisplayPannelRatio);

/* 
	모듈 내부에서 생성한 핸들은 해당 프로세스가 생성한 스레드간에 공유될 수 있으나 다른 프로세스와는 공유할 수 없다.
	즉, 서버와 클라이언트가 동일한 객체의 핸들을 소유하고 있어야 할 필요가 없으며 서버는 서버 고유의 입출력 완료 포트 객체를 소유하고
	클라이언트는 클라이언트 고유의 입출력 완료 포트 객체를 소유하면 된다.
*/
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow){
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0){ Exit(); }

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
	hConnectEvent = CreateEvent(NULL, FALSE, FALSE, CONNECTEVENTID);
	if(hConnectEvent == NULL){ Exit(); }

	MSG msg;
	while(GetMessage(&msg, NULL, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CloseHandle(hTimer);
	CloseHandle(hRecvEvent);
	CloseHandle(hSendEvent);

	HeapFree(GetProcessHeap(), 0, sock);
	TlsFree(TlsIndex);

	UnmapViewOfFile(wrPtr);
	CloseHandle(hMap);

	WSACleanup();
	return msg.wParam;
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static HWND hTopPannel, hLeftPannel, hRightPannel;
	static BOOL bSplit = FALSE;
	static int DisplayPannelRatio;
	int DisplayHeight;

	PAINTSTRUCT ps;
	HDC hdc;

	RECT rt, srt;
	POINT pt;
	HMENU hMenu, hPopup;
	
	static DWORD dwThreadID;
	HANDLE hClientMainThread, hConnectEvent;
	DWORD dwThread, dwConnect, dwExitCode;

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

			SetRect(&srt, 0,0, 530, 640);
			AdjustWindowRect(&srt, GetWindowLongPtr(hWnd, GWL_STYLE), GetMenu(hWnd) != NULL);
			SetWindowPos(hWnd, NULL, srt.left, srt.top, srt.right - srt.left, srt.bottom - srt.top, SWP_NOZORDER | SWP_NOMOVE);
			return 0;

		case WM_INITMENU:
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case ID_CONFIGCONNECT:
					switch(HIWORD(wParam)){
						case BN_CLICKED:
							// TODO : 대화상자 만들고 서버 IP,Port 직접 입력하게 끔 설계, 상태값을 유지할 수 있는 체크 버튼으로 만들 것.
							SetTimer(hWnd, 1, 10000, NULL);
							hClientMainThread = CreateThread(NULL, 0, ClientMain, NULL, 0, &dwThreadID);
							if(hClientMainThread){ CloseHandle(hClientMainThread); }
							else{ ErrorMessage(); }
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
					if(hClientMainThread == NULL){ ErrorMessage(); break; }

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
						ErrorMessage();
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
			if(hClientMainThread != NULL){ CloseHandle(hClientMainThread); }
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
				hBtnPannel = FindWindowEx(GetParent(GetParent(hWnd)), NULL, TEXT("ButtonPannelClass"), NULL);
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

						{
							char mbuf[0x400];
							wchar_t wbuf[0x800];
							int Length = -1;
							HANDLE hRecvEvent, hSendEvent;

							GetDlgItemTextA(hWnd, IDC_EDMSG, mbuf, sizeof(mbuf));
							Length = MultiByteToWideChar(CP_ACP, 0, mbuf, -1, NULL, 0);
							if(Length == 0 && GetLastError() == ERROR_INVALID_FLAGS){
								// ShowMessage(TEXT("An error occurred during charset conversion."));
								return ErrorMessage();
							}
							MultiByteToWideChar(CP_ACP, 0, mbuf, -1, wbuf, Length);

							/*
								ClientMain 스레드 작성 완료 01.19.일
								서버 연결까지 완료

								TODO :	1.	서버에 데이터 전송할 스레드 실행(01.20.월 - 완료)
										2.	전송한 데이터는 곧바로 서버가 접속한 모든 사람에게 반환(에코)하므로 Recv 이벤트 실행(전송이 성공적으로 완료되었는지 확인 가능)
										3.	전송한 데이터를 DisplayPannelClass로 전달
							*/
						}

						/* 프로세스 후 기본 작업 */
						SetWindowText(hMsgEdit, TEXT(""));
						SetFocus(hMsgEdit);
					}
					break;
			}
			return 0;

		case WM_DESTROY:
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

/* Multi Thread */
struct tag_Info{
	SOCKET sock;
	HANDLE hMap, hCP;
	DWORD TlsIndex;
};

DWORD WINAPI ClientMain(LPVOID lpArg){
	HANDLE hMap, hCP, hConnectEvent;
	SOCKET *sock;
	TCHAR* rdPtr;
	DWORD TlsIndex;

	hMap = OpenFileMapping(FILE_MAP_READ, FALSE, MAPPINGID);
	if(hMap == NULL){ return ErrorMessage();}

	rdPtr = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(SOCKET*) + sizeof(HANDLE) + sizeof(DWORD));
	if(rdPtr == NULL){ return ErrorMessage();}

	memcpy(&sock, rdPtr, sizeof(SOCKET*));
	memcpy(&hCP, rdPtr + sizeof(SOCKET*), sizeof(HANDLE));
	memcpy(&TlsIndex, rdPtr + sizeof(SOCKET*) + sizeof(HANDLE), sizeof(DWORD));

	*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(*sock == INVALID_SOCKET){ return WSAErrorMessage(); }

	struct sockaddr_in Server;
	Server.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &Server.sin_addr);
	Server.sin_port = htons(SERVERPORT);

	int ret = connect(*sock, (struct sockaddr*)&Server, sizeof(Server));
	if(ret == SOCKET_ERROR){ return WSAErrorMessage();}

	if(CreateIoCompletionPort((HANDLE)(*sock), hCP, 0, 0) != hCP){ return ErrorMessage(); }

	hConnectEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, CONNECTEVENTID);
	if(hConnectEvent == NULL){ return ErrorMessage(); }

	if(SetEvent(hConnectEvent) == FALSE){ return ErrorMessage(); }

	DWORD dwThreadID;
	struct tag_Info Info = {*sock, hMap, hCP, 0};
	HANDLE hWorkerThread = CreateThread(NULL, 0, WorkerThread, &Info, 0, &dwThreadID);
	if(hWorkerThread == NULL){ return ErrorMessage(); }

	HANDLE hTimer = OpenWaitableTimer(TIMER_MODIFY_STATE, FALSE, TIMERID);
	if(hTimer == NULL){ return ErrorMessage(); }

	#define PERIOD 5000
	LARGE_INTEGER DueTime = {0,};
	SetWaitableTimer(hTimer, &DueTime, (LONG)PERIOD, NULL, NULL, FALSE);

	DWORD dwThread, dwTimer, dwExitCode;

	while(1){
		/* 5초마다 실행 */
		dwTimer = WaitForSingleObject(hTimer, INFINITE);
		dwThread = WaitForSingleObject(hWorkerThread, 0);

		if(dwTimer == WAIT_FAILED || dwThread == WAIT_FAILED){ continue; }

		if(dwThread != WAIT_OBJECT_0){ continue; }
		if(GetExitCodeThread(hWorkerThread, &dwExitCode) == FALSE){ continue; }

		/* 에러 점검 */
		switch(dwExitCode){
			/* OpenEvent Failed */
			case ERROR_INVALID_HANDLE:
				ShowMessage(TEXT("The data required for communication has not been initialized. Please re-run the program."));
				ExitProcess(GetLastError());
				break;

			default:
				break;
		}
	}
		
	return 0;
}

DWORD WINAPI WorkerThread(LPVOID lpArg){
	struct tag_Info* Info = (struct tag_Info*)lpArg;

	OVERLAPPED ov;
	HANDLE hSendEvent;
	DWORD dwTransferred, dwEvent, dwError;

	hSendEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, SENDEVENTID);
	if(hSendEvent == NULL){ return ErrorMessage(); }

	while(1){
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
					ErrorMessage();
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

				/* 작업 실패 후 실패한 작업에 대한 정보를 저장한 상태 */
				default:
					// TODO: dwTransferred, lpCompletionKey(3), ov 정보 저장 후 스레드 재실행 시 우선 처리
					break;
			}
		}
	}

	return 0;
}

/* Utility */
void ShowMessage(TCHAR* buf){
	MessageBox(HWND_DESKTOP, buf, TEXT("Warning"), MB_ICONWARNING | MB_OK);
}

/* Fatal */
void Exit() { 
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError(); 

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL) == 0) {
		MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_ICONERROR | MB_OK);
		ExitProcess(dw);
	}

	MessageBox(HWND_DESKTOP, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_ICONERROR | MB_OK);

	LocalFree(lpMsgBuf);
	ExitProcess(dw); 
}

void WSAExit() { 
	LPVOID lpMsgBuf;
	int err = WSAGetLastError(); 

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL) == 0) {
		MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_ICONERROR | MB_OK);
		ExitProcess(err);
	}

	MessageBox(HWND_DESKTOP, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_ICONERROR | MB_OK);

	LocalFree(lpMsgBuf);
	ExitProcess(err); 
}

/* Nonfatal */
DWORD ErrorMessage(){
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError(); 

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL) == 0) {
		MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_ICONWARNING | MB_OK);
		return 0x10000000; /* Custom Error(29bit) : MessageFunction Failed */
	}

	MessageBox(HWND_DESKTOP, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_ICONWARNING | MB_OK);
	LocalFree(lpMsgBuf);

	return dw;
}

DWORD WSAErrorMessage(){
	LPVOID lpMsgBuf;
	int err = WSAGetLastError(); 

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL) == 0) {
		MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_ICONWARNING | MB_OK);
		return 0x10000000; /* Custom Error(29bit) : MessageFunction Failed */
	}

	MessageBox(HWND_DESKTOP, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_ICONWARNING | MB_OK);
	LocalFree(lpMsgBuf);

	return (DWORD)err;
}

