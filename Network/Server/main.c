#define _WIN32_WINNT 0x0A00
#define UNICODE

#include <ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#include <commctrl.h>
#include <strsafe.h>
#include <math.h>

#define IDC_BTNOPEN		101
#define IDC_BTNCLOSE	102
#define IDC_BTNCLEAR	103
#define IDC_BTNSEND		104
#define IDC_EDPORT		105
#define IDC_EDSTAT		106
#define IDC_EDMSG		107

/*
	클라이언트가 보내는 데이터 구조의 크기와 동일하게 버퍼 크기를 설정해도 되고
	약 4배 정도의 차이가 나도록 설정해도 된다.

	클라이언트 측에서 보내는 데이터가 현재 서버 버퍼의 크기보다 4배 작은 1kb라고 해보자.
	즉, 어떠한 동작을 수행할 때 클라이언트는 고정 크기를 가지는 데이터 구조를 보내는데 이때 이 데이터 구조가 1kb이다.

	이러한 경우 프로그래머가 설정한 서버-클라이언트간 프로토콜에 따라 클라이언트가 4번의 동작을 수행하기 전에는
	데이터를 보내지 않도록 만들 수도 있고(클라이언트측), 서버측에서 설정된 고정 크기만큼의 데이터가 오면 무조건 브로드캐스팅을 할 수도 있다.

	버퍼 사이즈로 인해 프로토콜에 변화가 발생하지는 않으므로 임의로 정하면 된다.

	서버는 얼추 만들었으니 클라이언트 프로그램을 만들어보자.
	25.01.08.Thu
*/
#define BUFSIZE 0x400

/* Callback Procedure */
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PannelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChatProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditSubProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);

/* Wnd Layout & Conf */
struct tag_Position{
	int x,y,Width,Height;
};
const struct tag_Position WndPosition = {0,0, 600, 400};
const TCHAR *CLASS_NAME[3]= {
	TEXT("ChattingServer"),
	TEXT("PannelWndClass"),
	TEXT("ChatWndClass")
};

struct tag_InputArguments{
	int IP;
	int Port;
};

/* Util */
void ShowText(const char* fmt, ...);
void ShowError(const char* Error);
wchar_t* ConvertCharset(const char* MBuf);

/* SocketInfo & SocketNode & DoublyLinked List */
typedef enum tag_EventType {NONE, RECV, SEND} EVENT_TYPE;
typedef struct tag_SocketInfo{
	OVERLAPPED ov;
	SOCKET sock;
	BYTE buf[BUFSIZE+1];			// 데이터 저장용 버퍼
	int recv, send;
	WSABUF wsabuf;				// 송수신용 임시 버퍼
	EVENT_TYPE EventType;
	BOOL Connected;
}SocketInfo;

typedef struct tag_SocketNode{
	SocketInfo* Info;
	struct tag_SocketNode* Next;
	struct tag_SocketNode* Prev;
}SocketNode;

SocketInfo* CreateSocketInfo(SOCKET ClientSocket, EVENT_TYPE EventType);
void DestroySocketInfo(SocketInfo* Target);

SocketNode* CreateSocketNode(SocketInfo* NewInfo);
void DestroySocketNode(SocketNode* Target);
void AppendSocketNode(SocketNode** Head, SocketNode* NewNode);
void InsertAfter(SocketNode* Current, SocketNode* NewNode);
void RemoveSocketNode(SocketNode** Head, SocketNode* Remove);
SocketNode* GetNodeAtLocation(SocketNode* Head, int Location);
SocketNode* GetNodeAtSocketInfo(SocketNode* Head, SocketInfo* ClientSession);
int GetNodeCount(SocketNode* Head);

/* Buffer Node & Queue */
typedef struct tag_BufferNode{
	TCHAR * buf;
}BufferNode;

typedef struct tag_Queue{
	int Capacity;
	int Front;
	int Rear;

	BufferNode** Nodes;
}Queue;

BufferNode* CreateBufferNode(const TCHAR* Data);
void DestroyBufferNode(BufferNode* Target);
void CreateQueue(Queue** Q, int Capacity);
void DestroyQueue(Queue* Q);
BOOL Enqueue(Queue* Q, BufferNode* NewNode);
BufferNode* Dequeue(Queue* Q);
int GetSize(Queue* Q);
BOOL IsEmpty(Queue* Q);
BOOL IsFull(Queue* Q);

/* Main Server Thread & Worker Thread */
DWORD WINAPI ServerMain(LPVOID lpArg);
DWORD WINAPI Processing(LPVOID lpArg);

/* Common - Use the static keyword to avoid duplicate variable names." */
static Queue* Q;
static SocketNode* L;
static CRITICAL_SECTION cs;
static BOOL bRunning;
static HWND hEdit;

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow){
	InitializeCriticalSection(&cs);
	CreateQueue(&Q, 0x1000);

	WNDCLASS wc = { };

	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = CLASS_NAME[0];
	RegisterClass(&wc);

	wc.lpfnWndProc = PannelProc;
	wc.hInstance = hInst;
	wc.lpszClassName = CLASS_NAME[1];
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	RegisterClass(&wc);

	wc.lpfnWndProc = ChatProc;
	wc.hInstance = hInst;
	wc.lpszClassName = CLASS_NAME[2];
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	RegisterClass(&wc);

	// Create the window.
	HWND hWnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME[0],                  // Window class
		TEXT("Server"),					// Window title
		WS_OVERLAPPEDWINDOW,            // Window style
		WndPosition.x, WndPosition.y,
		WndPosition.Width, WndPosition.Height,	// x,y,Width,Height
		NULL,							// Parent window
		NULL,							// Menu
		hInst,							// Instance handle
		NULL							// Additional application data
	);

	if (hWnd == NULL) { return 0; }

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DeleteCriticalSection(&cs);
	DestroyQueue(Q);

	return (int)msg.wParam;				// return 0; - WINAPI
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HWND hPannel, hChat;
	static struct tag_Position PannelWndPosition, ChatWndPosition;
	LPMINMAXINFO lpmi;

	switch (uMsg) {
		case WM_CREATE:
			hPannel = CreateWindow(CLASS_NAME[1], NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0,0,0,0, hWnd, NULL, GetModuleHandle(NULL), NULL);
			hChat = CreateWindow(CLASS_NAME[2], NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0,0,0,0, hWnd, NULL, GetModuleHandle(NULL), NULL);
			return 0;

		case WM_GETMINMAXINFO:
			lpmi = (LPMINMAXINFO)lParam;
			lpmi->ptMinTrackSize.x = WndPosition.Width;
			lpmi->ptMinTrackSize.y = WndPosition.Height;
			return 0;

		case WM_SIZE:
			if(wParam != SIZE_MINIMIZED){
				RECT crt;
				GetClientRect(hWnd, &crt);
				ChatWndPosition.Width = PannelWndPosition.Width = crt.right - crt.left;
				ChatWndPosition.Height = PannelWndPosition.Height = crt.bottom - crt.top;

				PannelWndPosition.Height = (int)((float)(PannelWndPosition.Height * 0.3f));
				ChatWndPosition.Height -= PannelWndPosition.Height;

				SetWindowPos(hPannel, NULL, crt.left, crt.top, PannelWndPosition.Width, PannelWndPosition.Height, SWP_NOZORDER);
				SetWindowPos(hChat, NULL, 0, PannelWndPosition.Height, ChatWndPosition.Width, ChatWndPosition.Height, SWP_NOZORDER);
			}
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK PannelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	static HWND hPort, hBtns[3];
	static struct tag_Position IPWndPosition, BtnWndPosition[3];
	INITCOMMONCONTROLSEX icex;
	DWORD dwAddress, dwPort;

	static const int Field = 4;

	RECT crt;
	static HANDLE hServerMainThread;
	static DWORD dwServerMainThreadID;
	DWORD dwExitCode;
	
	static struct tag_InputArguments InputArguments;
	BOOL bTranslated;
	
	switch (uMsg) {
		case WM_CREATE:
			hPort = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER, 16, 10, 60, 20, hWnd, (HMENU)IDC_EDPORT, GetModuleHandle(NULL), NULL); 
			SendMessage(hPort, EM_LIMITTEXT, (WPARAM)5, 0);

			BtnWndPosition[0].Width = BtnWndPosition[1].Width = BtnWndPosition[2].Width = 60;
			BtnWndPosition[0].Height = BtnWndPosition[1].Height = BtnWndPosition[2].Height = 20;

			hBtns[0] = CreateWindow(TEXT("button"), TEXT("Open"), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_PUSHBUTTON, 0,0, BtnWndPosition[0].Width, BtnWndPosition[0].Height, hWnd, (HMENU)IDC_BTNOPEN, GetModuleHandle(NULL), NULL);
			hBtns[1] = CreateWindow(TEXT("button"), TEXT("Close"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0,0, BtnWndPosition[1].Width, BtnWndPosition[1].Height, hWnd, (HMENU)IDC_BTNCLOSE, GetModuleHandle(NULL), NULL);
			hBtns[2] = CreateWindow(TEXT("button"), TEXT("Clear"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0,0, BtnWndPosition[2].Width, BtnWndPosition[2].Height, hWnd, (HMENU)IDC_BTNCLEAR, GetModuleHandle(NULL), NULL);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDC_BTNCLEAR:
					SetWindowText(hEdit, TEXT(""));
					break;

				case IDC_BTNOPEN:
					/* Server Main Process  */
					InputArguments.Port = GetDlgItemInt(hWnd, IDC_EDPORT, &bTranslated, FALSE);
					if(bTranslated && InputArguments.Port >= 0x400 && InputArguments.Port < 0xffff){
						hServerMainThread = CreateThread(NULL, 0, ServerMain, &InputArguments, 0, &dwServerMainThreadID);
						if(hServerMainThread == NULL){
							if(MessageBox(hWnd, TEXT("서버 생성 실패"), TEXT("Warning"),
										MB_ICONWARNING | MB_RETRYCANCEL | MB_DEFBUTTON1) == IDRETRY){
								SendMessage(hWnd, WM_COMMAND, wParam, lParam);
							}
						}
						else{
							bRunning = TRUE;
							SetWindowText(GetParent(hWnd), TEXT("Server - 실행중"));
							ShowText("서버가 실행되었습니다.\r\n[IP = %d(%d)]\r\n", 0x12345678, InputArguments.Port);
							SetTimer(hWnd, 1, 3600000, NULL);
						}
					}else{
						ShowText("Port number is invalid");
					}
					break;

				case IDC_BTNCLOSE:
					/* Server Close */
					if(bRunning){
						bRunning = FALSE;
						ShowText("서버가 종료되었습니다.\r\n종료 코드 : Close\r\n");
						SetWindowText(GetParent(hWnd), TEXT("Server"));
					}
					break;
			}
			return 0;

		case WM_TIMER:
			switch(wParam){
				case 1:
					if(!GetExitCodeThread(hServerMainThread, &dwExitCode)){
						ShowError("GetExitCodeThread()");
					}else{
						if(dwExitCode != STILL_ACTIVE){
							if(bRunning){
								bRunning = FALSE;
								/* 코드 분기 처리 세분화해도 좋을듯 */
								ShowText("서버가 종료되었습니다.\r\n종료 코드 : %d\r\n", dwExitCode);
								SetWindowText(GetParent(hWnd), TEXT("Server"));
							}
						}
					}
					break;
			}
			return 0;

		case WM_SIZE:
			GetClientRect(hWnd, &crt);
			BtnWndPosition[0].x = crt.right - crt.left - 70;
			BtnWndPosition[0].y = 10;
			
			BtnWndPosition[1].x = crt.right - crt.left - 70;
			BtnWndPosition[1].y = BtnWndPosition[0].y + BtnWndPosition[0].Height + 5;

			BtnWndPosition[2].x = crt.right - crt.left - 70;
			BtnWndPosition[2].y = BtnWndPosition[1].y + BtnWndPosition[1].Height + 5;

			SetWindowPos(hBtns[0], NULL, BtnWndPosition[0].x, BtnWndPosition[0].y, BtnWndPosition[0].Width, BtnWndPosition[0].Height, SWP_NOZORDER);
			SetWindowPos(hBtns[1], NULL, BtnWndPosition[1].x, BtnWndPosition[1].y, BtnWndPosition[1].Width, BtnWndPosition[1].Height, SWP_NOZORDER);
			SetWindowPos(hBtns[2], NULL, BtnWndPosition[2].x, BtnWndPosition[2].y, BtnWndPosition[2].Width, BtnWndPosition[2].Height, SWP_NOZORDER);
			return 0;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				EndPaint(hWnd, &ps);
			}
			return 0;

		case WM_DESTROY:
			if(hServerMainThread){CloseHandle(hServerMainThread);}
			KillTimer(hWnd, 1);
			return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// static WNDPROC OldEditProc;
LRESULT CALLBACK ChatProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	static HWND hMsgEdit, hSendBtn;
	char temp[0x800];				// 설정없이 최대 크기 약 32k
	RECT rt,srt;

	switch (uMsg) {
		case WM_CREATE:
			hEdit = CreateWindow(TEXT("edit"), NULL, WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL | ES_READONLY | WS_VSCROLL | WS_HSCROLL, 0,0,0,0, hWnd, (HMENU)IDC_EDSTAT, GetModuleHandle(NULL), NULL);

			/*
			   서버측에서 접속자에게 메세지를 보내려면 주석 제거 후 활성화
			hMsgEdit = CreateWindow(TEXT("edit"), NULL, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 0,0,0,0, hWnd, (HMENU)IDC_EDMSG, GetModuleHandle(NULL), NULL);
			hSendBtn = CreateWindow(TEXT("button"), TEXT("Send"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0,0,0,0, hWnd, (HMENU)IDC_BTNSEND, GetModuleHandle(NULL), NULL);
			SetFocus(hMsgEdit);
			OldEditProc = (WNDPROC)SetWindowLongPtr(hMsgEdit, GWLP_WNDPROC, (LONG_PTR)EditSubProc);
			*/
			return 0;

			/*
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDC_BTNSEND:
					GetDlgItemTextA(hWnd, IDC_EDMSG, temp, sizeof(temp));
					ShowText(temp);
					SetFocus(hMsgEdit);
					break;
			}
			return 0;
			*/

		case WM_SIZE:
			GetClientRect(hWnd, &rt);
			SetRect(&srt, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top /* - 20 */);
			SetWindowPos(hEdit, NULL, srt.left, srt.top, srt.right, srt.bottom, SWP_NOZORDER);

			/*
			SetRect(&srt, rt.left, rt.bottom - rt.top - 20, rt.right - rt.left - 60, 20);
			SetWindowPos(hMsgEdit, NULL, srt.left, srt.top, srt.right, srt.bottom, SWP_NOZORDER);

			SetRect(&srt, rt.right - rt.left - 60, rt.bottom - rt.top - 20, 60, 20);
			SetWindowPos(hSendBtn, NULL, srt.left, srt.top, srt.right, srt.bottom, SWP_NOZORDER);
			*/
			return 0;

		case WM_DESTROY:
			/*
			SetWindowLongPtr(hMsgEdit, GWLP_WNDPROC, (LONG_PTR)OldEditProc);
			*/
			return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);

}

/*
	서버 측에서 접속자에게 메세지를 보내고 싶은 경우를 위해 만들어뒀다.
	단순히 하나의 윈도우에만 서브 클래싱을 적용한 것이므로 범용적이지 않으며 필요시 주석을 지우고 활성하면 된다.

LRESULT CALLBACK EditSubProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam) {
	static HWND hBtn;
	switch (iMessage) {
	case WM_CHAR:
		if(wParam == VK_RETURN){return 0;}
		break;

	case WM_KEYDOWN:
		if(hBtn == NULL){ hBtn = FindWindow(TEXT("button"), TEXT("Send")); }

		if(wParam == VK_RETURN) {
			SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)IDC_BTNSEND, (LPARAM)hBtn);
		}
		break;

	case WM_LBUTTONDOWN:
		SendMessage(hWnd,EM_SETSEL,0,-1);
		break;
	}

	return CallWindowProc(OldEditProc,hWnd,iMessage,wParam,lParam);
}
*/
DWORD WINAPI ServerMain(LPVOID lpArg){
	if(hEdit == NULL){
		MessageBox(HWND_DESKTOP, TEXT("윈도우 생성에 실패하였습니다.\r\n프로그램을 다시 실행해주세요.\r\n"), TEXT("Alarm"),MB_ICONINFORMATION | MB_OK);
		return -1;
	}

	/* 서버 입장에서는 IP가 bind에서 확정되므로 필요하지 않다. */
	struct tag_InputArguments arg = *(struct tag_InputArguments*)lpArg;
	int SERVERPORT = arg.Port;
	int ret;

	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0,0);
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	HANDLE hThread;
	
	for(int i=0; i<(int)si.dwNumberOfProcessors * 2; i++){
		hThread = CreateThread(NULL, 0, Processing, hcp, 0, NULL);
		if(hThread == NULL){return -1;}
		CloseHandle(hThread);
	}

	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0){return -1;}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET){ return 1; }

	/*
	   소켓 모드 설정 불필요 - 설정 후 Async I/O 오류 발생, 비동기 입출력과 비동기 통지를 이용하므로 넌블로킹 소켓일 필요가 없으며 실제 비동기 함수를 이용해 MSG_WAITALL 등의 플래그로 확인해본 결과 오류가 발생하지 않는 것으로 보아 넌블로킹 소켓이 아님을 알 수 있었다.


		ULONG On = 1;
		ret = ioctlsocket(sock, AF_INET, FIONBIO, &On);
		if(ret == SOCKET_ERROR){ShowError(L"ioctlsocket()"); return 1;}
	*/
	
	struct sockaddr_in ServerAddress = {0,};
	ServerAddress.sin_port = htons(SERVERPORT);
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerAddress.sin_family = AF_INET;

	ret = bind(sock, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress));
	if(ret == SOCKET_ERROR){return 1;}
	
	ret = listen(sock, SOMAXCONN);
	if(ret == SOCKET_ERROR){return 1;}

	DWORD dwReuseAddressOption = 1;
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&dwReuseAddressOption, sizeof(dwReuseAddressOption));
	if(ret == SOCKET_ERROR){return 2;}

	DWORD dwKeepAliveOption = 1;
	ret = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&dwKeepAliveOption, sizeof(dwKeepAliveOption));
	if(ret == SOCKET_ERROR){return 2;}

	struct linger l_LingerOption = {1, 10};
	ret = setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*)&l_LingerOption, sizeof(l_LingerOption));
	if(ret == SOCKET_ERROR){return 2;}

	struct sockaddr_in Client;
	SOCKET Client_Socket;
	SocketInfo *NewSocketInfo = NULL;

	while(bRunning){
		int cbAddress = sizeof(Client);
		Client_Socket = accept(sock, (struct sockaddr*)&Client, &cbAddress);
		if(Client_Socket == INVALID_SOCKET){
			ShowError("accept()"); break;
		}

		char IP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &Client.sin_addr, IP, sizeof(IP));
		ShowText("\r\n[TCP Server] Client Accept : IP = %s(%d)\r\n", IP, ntohs(Client.sin_port));

		CreateIoCompletionPort((HANDLE)Client_Socket, hcp, 0, 0);
		NewSocketInfo = CreateSocketInfo(Client_Socket, RECV);
				
		WSABUF wsabuf;
		wsabuf.buf = NewSocketInfo->buf;
		wsabuf.len = sizeof(NewSocketInfo->buf)-1;

		DWORD Flags = 0, cbRecv = 0;
		ret = WSARecv(Client_Socket, &wsabuf, 1, &cbRecv, &Flags, &NewSocketInfo->ov, NULL);
		if(ret == SOCKET_ERROR){
			if(WSAGetLastError() != WSA_IO_PENDING){
				ShowError("WSARecv()");
			}

			continue;
		}
	}

	closesocket(sock);
	WSACleanup();

	CloseHandle(hcp);
	return 0;
}

DWORD WINAPI Processing(LPVOID lpArg){
	if(L == NULL){return -1;}

	int ret;
	DWORD cbTransferred;
	SocketInfo* ClientSession;
	SocketNode* CurrentNode;

	ULONG_PTR CompletionKey;
	HANDLE hcp = (HANDLE)lpArg;

	while(bRunning){
		// 감시 대상에 대한 이벤트가 발생하면 스레드를 깨운다.
		// cbTransferred는 WSARecv 함수를 호출할 때 전달한 WSABUF 구조체의 멤버 변수인 len보다 절대 클 수 없다.
		// TODO: 코드 전체 수정 필요
		ret = GetQueuedCompletionStatus(hcp, &cbTransferred, (PULONG_PTR)&CompletionKey, (LPOVERLAPPED*)&ClientSession, INFINITE);
		if(ClientSession->Connected == FALSE){
			/*
				가능성은 희박하나 다중 클라이언트를 이용한 동시 접속이 이뤄질 수 있다.
				때문에 임계구역에 변수를 밀어넣고 값을 수정하는 것이 좋으나(원자성)
				값을 수정하는 구간이 해당 분기 외에는 존재하지 않고 소켓 정보를 저장하는
				자료구조가 연결 리스트 형태이므로 위 경우는 고려하지 않기로 한다.
			*/
			ClientSession->Connected = TRUE;
			CurrentNode = CreateSocketNode(ClientSession);

			EnterCriticalSection(&cs);
			AppendSocketNode(&L, CurrentNode);
			LeaveCriticalSection(&cs);
		}else{
			EnterCriticalSection(&cs);
			CurrentNode = GetNodeAtSocketInfo(L, ClientSession);
			LeaveCriticalSection(&cs);
		}

		struct sockaddr_in Client;
		int cbAddress = sizeof(Client);

		getpeername(ClientSession->sock, (struct sockaddr*)&Client, &cbAddress);
		char IP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &Client.sin_addr, IP, sizeof(IP));

		/* 
		   윈도우 전용 비동기 함수는 동작이 조금 다르다.
		*/
		if(ret == 0 || cbTransferred == 0){
			ShowText("[TCP Server] Disconnected : %s(%d)\r\n", IP, ntohs(Client.sin_port));

			EnterCriticalSection(&cs);
			RemoveSocketNode(&L, CurrentNode);
			LeaveCriticalSection(&cs);

			/*
				free 함수는 delete와 다르게 NULL에 대한 처리를 하지 않는 것으로 알고있다.
				때문에 임계영역 내에서 한번에 동작하도록(원자성) 하는 것이 좋으나
				위에서 말한 경우와 해당 분기가 여러번 발생하는 상황은 같이 극히 드문 경우이므로
				고려하지 않기로 한다.
			*/
			DestroySocketNode(CurrentNode);
			continue;
		}

		/* 
			TODO : 데이터 송수신 응용프로그램간 프로토콜 설정

			서버의 세부적인 동작을 정의하는 부분이다.
			확장을 생각한다면 상황에 따라 비동기 함수를 호출할 수 있도록 이벤트 타입을 설정하고
			이에 반응하는 콜백 함수를 활용하는 구조로 만드는 것이 좋다.

			크게 확장할 생각은 없으므로 단순한 구조를 가지는 분기 처리 형태로 작성할 예정이다.

			또한, 자동화 버퍼를 사용해도 좋으나 중계 역할만을 하는 서버 프로그램이므로
			이는 생략한다.
		*/
		if(ClientSession->recv == 0){
			ClientSession->recv = cbTransferred;
			ClientSession->send = 0;
			ClientSession->buf[ClientSession->recv] = 0;
			ShowText("[Addr : %s(%d)] %s\r\n", IP, ntohs(Client.sin_port), ClientSession->buf);
		}else{
			/*if(ClientSession->EventType == SEND)*/
			ClientSession->send += cbTransferred;
		}

		if(ClientSession->recv > ClientSession->send){
			memset(&ClientSession->ov, 0, sizeof(ClientSession->ov));

			WSABUF wsabuf;
			wsabuf.buf = ClientSession->buf + ClientSession->send;
			wsabuf.len = ClientSession->recv - ClientSession->send;

			DWORD dwSend, dwFlags = 0;
			// ClientSession->EventType = SEND;

			/* 
				TODO : 브로드캐스팅 기능을 추가해야 한다.
				윈속 함수 특성상 한 번에 보내고 받는게 가능하기 때문에
				연결된 소켓의 개수를 확인하고 각 소켓에 있는 버퍼를 복사하여 한 번에 처리할 수 있다.

				단, 해당 프로젝트에서는 단순 반복문으로 처리하기로 한다.
				스레드가 어떠한 이유로 대기상태에 빠지거나 중단된 상태가 아니라면 사실상
				하나씩 실행되어 통신을 수행하기 때문에 큰 문제는 발생하지 않는다.
			*/
			EnterCriticalSection(&cs);
			int N = GetNodeCount(L);
			LeaveCriticalSection(&cs);

			SocketNode *Target = NULL;
			for(int i = 0; i < N; i++){
				EnterCriticalSection(&cs);
				Target = GetNodeAtLocation(L, i);
				LeaveCriticalSection(&cs);

				ret = WSASend(Target->Info->sock, &wsabuf, 1, &dwSend, dwFlags, &ClientSession->ov, NULL);
				if(ret == SOCKET_ERROR){
					if(WSAGetLastError() != WSA_IO_PENDING){
						ShowError("WSASend()");
					}
					continue;
				}else{
					DWORD dwError = GetLastError();
					TCHAR buf[256];
					StringCbPrintf(buf, sizeof(buf), TEXT("GetLastError() = %d"), dwError);
					MessageBox(HWND_DESKTOP, buf, TEXT("Alarm"), MB_OK);

				}
			}
		}else{
			/* recv == send | recv < send */
			memset(&ClientSession->ov, 0, sizeof(ClientSession->ov));
			ClientSession->recv = 0;
			// ClientSession->EventType = RECV;		// 클라이언트 통신을 위한 RECV 이벤트 재등록
			
			WSABUF wsabuf;
			wsabuf.buf = ClientSession->buf;
			wsabuf.len = BUFSIZE;

			DWORD dwRecv, dwFlags = 0;
			ret = WSARecv(ClientSession->sock, &wsabuf, 1, &dwRecv, &dwFlags, &ClientSession->ov, NULL);

			if(ret == SOCKET_ERROR){
				if(WSAGetLastError() != WSA_IO_PENDING){
					ShowError("WSARecv()");
				}
				continue;
			}
		}
	}

	return 0;
}

/* Utility Function */
void ShowText(const char* fmt, ...){
	char mbuf[0x100];
	TCHAR Buf[0x300];

	va_list arg;
	va_start(arg, fmt);
	StringCbVPrintfA(mbuf, sizeof(mbuf), fmt, arg);
	va_end(arg);

	int cvLength = MultiByteToWideChar(CP_ACP, 0, mbuf, -1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, mbuf, -1, Buf, cvLength);

	int nLength = GetWindowTextLength(hEdit);

	EnterCriticalSection(&cs);
	SendMessage(hEdit, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)Buf);
	LeaveCriticalSection(&cs);
}

void ShowError(const char* Error){
	LPVOID lpMsgBuf;
	
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&lpMsgBuf, 0, NULL);

	ShowText("%s : %s\r\n", Error, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

wchar_t* ConvertCharset(const char* MBuf){
	wchar_t *Result = NULL;

	int Length = MultiByteToWideChar(CP_ACP, 0, MBuf, -1, NULL, 0);
	Result = (wchar_t*)malloc(sizeof(wchar_t) * Length);
	MultiByteToWideChar(CP_ACP, 0, MBuf, -1, Result, Length);

	return Result;
}

/* BufferNode & BufferQueue */
BufferNode* CreateBufferNode(const TCHAR* Data){
	int Length = lstrlen(Data);
	BufferNode* NewNode = (BufferNode*)malloc(sizeof(BufferNode));

	NewNode->buf = (TCHAR*)malloc(sizeof(TCHAR) * (Length+1));
	memcpy(NewNode->buf, Data, sizeof(TCHAR) * Length);
	NewNode->buf[Length] = '\0';

	return NewNode;
}

void DestroyBufferNode(BufferNode* Target){
	free(Target->buf);
	free(Target);
}

void CreateQueue(Queue** Q, int Capacity){
	(*Q) = (Queue*)malloc(sizeof(Queue));

	(*Q)->Nodes = (BufferNode**)malloc(sizeof(BufferNode*) * Capacity);

	(*Q)->Capacity = Capacity;
	(*Q)->Front = (*Q)->Rear = 0;
}

void DestroyQueue(Queue* Q){
	for(int i=0; i<Q->Capacity; i++){
		if(Q->Nodes[i] != NULL){
			DestroyBufferNode(Q->Nodes[i]);
		}
	}

	free(Q->Nodes);
	free(Q);
}

BOOL Enqueue(Queue* Q, BufferNode* NewNode){
	if(IsFull(Q)){return FALSE;}

	int Position = 0;

	if(Q->Rear == Q->Capacity){
		Position = Q->Rear;
		Q->Rear = 0;
	}else{
		Position = Q->Rear++;
	}

	Q->Nodes[Position] = NewNode;
}

BufferNode* Dequeue(Queue* Q){
	if(IsEmpty(Q)){return NULL;}

	int Position = Q->Front;

	if(Q->Front == Q->Capacity){
		Q->Front = 0;
	}else{
		Q->Front++;
	}

	return Q->Nodes[Position];
}

int GetSize(Queue* Q){
	if(Q->Front <= Q->Rear){
		return Q->Rear - Q->Front;
	}else{
		return Q->Rear + (Q->Capacity - Q->Front) + 1;
	}
}

BOOL IsEmpty(Queue* Q){
	return (Q->Front == Q->Rear);
}

BOOL IsFull(Queue* Q){
	if(Q->Front < Q->Rear){
		return ((Q->Rear - Q->Front) == Q->Capacity);
	}else{
		return (Q->Rear+1 == Q->Front);
	}
}

/* SocketInfo & SocketNode */
SocketInfo* CreateSocketInfo(SOCKET ClientSocket, EVENT_TYPE EventType){
	SocketInfo* NewSocketInfo = (SocketInfo*)malloc(sizeof(SocketInfo));

	memset(&NewSocketInfo->ov, 0, sizeof(NewSocketInfo->ov));
	NewSocketInfo->send = NewSocketInfo->recv = 0;
	NewSocketInfo->sock = ClientSocket;
	NewSocketInfo->EventType = EventType;
	NewSocketInfo->Connected = FALSE;

	return NewSocketInfo;
}

void DestroySocketInfo(SocketInfo* Target){
	closesocket(Target->sock);
	free(Target);
}

SocketNode* CreateSocketNode(SocketInfo* NewInfo){
	SocketNode* NewNode = (SocketNode*)malloc(sizeof(SocketNode));

	NewNode->Info = NewInfo;
	NewNode->Prev = NewNode->Next = NULL;

	return NewNode;
}

void DestroySocketNode(SocketNode* Target){
	DestroySocketInfo(Target->Info);
	free(Target);
}

void AppendSocketNode(SocketNode** Head, SocketNode* NewNode){
	if((*Head) == NULL){
		(*Head) = NewNode;
	}else{
		SocketNode* Tail = (*Head);
		while(Tail->Next != NULL){
			Tail = Tail->Next;
		}

		Tail->Next = NewNode;
		NewNode->Prev = Tail;
	}
}

void InsertAfter(SocketNode* Current, SocketNode* NewNode){
	NewNode->Next = Current->Next;
	NewNode->Prev = Current;

	if(Current->Next != NULL){
		Current->Next->Prev = NewNode;
	}

	Current->Next = NewNode;
}

void RemoveSocketNode(SocketNode** Head, SocketNode* Remove){
	if((*Head) == Remove){
		(*Head) = Remove->Next;
		if((*Head) != NULL){
			(*Head)->Prev = NULL;
		}

		Remove->Prev = Remove->Next = NULL;
	}else{
		SocketNode* temp = Remove;

		Remove->Prev->Next = temp->Next;
		if(Remove->Next != NULL){
			Remove->Next->Prev = temp->Prev;
		}
		
		Remove->Prev = Remove->Next = NULL;
	}
}

SocketNode* GetNodeAtLocation(SocketNode* Head, int Location){
	SocketNode* Current = Head;

	while(Current != NULL && (--Location) >= 0){
		Current = Current->Next;
	}

	return Current;
}

SocketNode* GetNodeAtSocketInfo(SocketNode* Head, SocketInfo* ClientSession){
	SocketNode* Current = Head;
	
	/* 필요시 memcmp로 전체 비교 가능 */
	while(Current != NULL && Current->Info != ClientSession){
		Current = Current->Next;
	}
	
	return Current;
}

int GetNodeCount(SocketNode* Head){
	int cnt = 0;
	SocketNode* Current = Head;

	while(Current != NULL){
		Current = Current->Next;
		cnt++;
	}

	return cnt;
}
