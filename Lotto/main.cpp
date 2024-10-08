#define _WIN32_WINNT 0x0A00
#include <windows.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#define CLASS_NAME TEXT("Lotto")

#define QUIT -1
#define FAILURE 0
#define SUCCESS 1
#define _TEXTOUT(DC, X, Y, TEXT) TextOut(DC, X, Y, TEXT, lstrlen(TEXT));

#include "Stack.h"
Stack* S = NULL;
Node* Popped = NULL;

const int LookupTable[] = {
	1, 2, 3, 4, 5,
	6, 7, 8, 9, 10,
	11, 12, 13, 14, 15,
	16, 17, 18, 19, 20,
	21, 22, 23, 24, 25,
	26, 27, 28, 29, 30,
	31, 32, 33, 34, 35,
	36, 37, 38, 39, 40,
	41, 42, 43, 44, 45
};
const int Pcs = sizeof(LookupTable)/sizeof(LookupTable[0]);

HWND g_hWnd;
HANDLE g_hTimer;
HBITMAP g_hBitmap,
		g_hWallPaper;

BOOL Init();
void Wait();
void Clean();
void GetClientSize(HWND hWnd, PLONG Width, PLONG Height);
void SetClientRect(HWND hWnd, LONG Width, LONG Height);
void CenterWindow(HWND hWnd);
void DrawBitmap(HDC hdc, LONG x, LONG y, HBITMAP hBit);
HBITMAP loadbmp(HDC hdc, LPCTSTR Path);

void DrawBall(HDC hdc, COLORREF Color);
void DrawCircle(HDC hdc, int x, int y, int iRadius);
void Sort(int* arr, int left, int right);
inline BOOL Choose();
void InitialSetUp();

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
	WNDCLASSEX wcex = {
		sizeof(wcex),
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		NULL,
		NULL,
		CLASS_NAME,
		NULL
	};

	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				CLASS_NAME,
				CLASS_NAME,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				NULL,
				(HMENU)NULL,
				hInst,
				NULL
			);

	SetClientRect(hWnd, 800, 600);
	CenterWindow(hWnd);

	ShowWindow(hWnd, nCmdShow);
	srand(GetTickCount64());

    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }else{
			// IDLE : 

			// Wait();
			WaitMessage();
        }
    }

    return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	static LONG cWidth,
				cHeight;

	static int	StretchMode;

	OSVERSIONINFO osv;

	switch(iMessage)
	{
		case WM_CREATE:
			if(Init() == FAILURE){ return QUIT; }
			CreateStack(&S);
			InitialSetUp();

			osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			if(osv.dwPlatformId >= VER_PLATFORM_WIN32_NT){
				StretchMode = HALFTONE;
			}else{
				StretchMode = COLORONCOLOR;
			}

			SetTimer(hWnd, 1, 25, NULL);
			return 0;
			
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);

				if(g_hBitmap)
				{
					DrawBitmap(hdc, 0,0, g_hBitmap);
				}
				
				EndPaint(hWnd, &ps);
			}
			return 0;

		case WM_SIZE:
			if(wParam != SIZE_MINIMIZED){
				if(g_hBitmap != NULL){
					DeleteObject(g_hBitmap);
					g_hBitmap = NULL;
				}
			}
			return 0;

		case WM_TIMER:
			{
				HDC hdc = GetDC(hWnd);
				HDC hBkDC = CreateCompatibleDC(hdc);
				HDC hMemDC = CreateCompatibleDC(hdc);
				
				RECT crt;
				GetClientRect(hWnd, &crt);
				if(g_hBitmap == NULL){
					g_hBitmap = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
				}
				HBITMAP hMemOld = (HBITMAP)SelectObject(hMemDC, g_hBitmap);
				// 배경 : 비트맵 or 단색

				if(g_hWallPaper == NULL){
					g_hWallPaper = loadbmp(hdc, TEXT("WallPaper.bmp"));
				}
				HBITMAP hBkOld = (HBITMAP)SelectObject(hBkDC, g_hWallPaper);

				BITMAP bmp;
				GetObject(g_hWallPaper, sizeof(BITMAP), &bmp);

				SetStretchBltMode(hMemDC, StretchMode);
				StretchBlt(hMemDC, 0,0, crt.right - crt.left, crt.bottom - crt.top, hBkDC, 0,0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

				if(GetAsyncKeyState(VK_LBUTTON) & 0x8000){ 
					TCHAR Debug[256];
					wsprintf(Debug, TEXT("Pcs = %d"), Pcs);
					_TEXTOUT(hMemDC, 10, 10, Debug);
				}

				SelectObject(hBkDC, hBkOld);
				SelectObject(hMemDC, hMemOld);
				DeleteDC(hBkDC);
				DeleteDC(hMemDC);
				ReleaseDC(hWnd, hdc);
			}

			InvalidateRect(hWnd, NULL, FALSE);
			return 0;

		case WM_DESTROY:
			Clean();
			PostQuitMessage(0);
			return 0;
	}
	
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

BOOL Init(){
	g_hTimer = CreateWaitableTimer(NULL, FALSE, NULL);

	if(g_hTimer == NULL){return FALSE;}

	LARGE_INTEGER li = {0};
	if(!SetWaitableTimer(g_hTimer, &li, (1000.f / 60.f), NULL, NULL, FALSE)){
		CloseHandle(g_hTimer);
		g_hTimer = NULL;
		return FALSE;
	}

	return TRUE;
}

void Wait(){
	if(MsgWaitForMultipleObjects(1, &g_hTimer, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0){
		InvalidateRect(g_hWnd, NULL, FALSE);
	}
}

void Clean(){
	KillTimer(g_hWnd, 1);
	if(g_hTimer != NULL){ CloseHandle(g_hTimer); }
	if(g_hBitmap != NULL){ DeleteObject(g_hBitmap); }
	if(g_hWallPaper != NULL){ DeleteObject(g_hWallPaper); }
}

void GetClientSize(HWND hWnd, PLONG Width, PLONG Height){
	RECT crt;

	GetClientRect(hWnd, &crt);
	*Width = crt.right - crt.left;
	*Height = crt.bottom - crt.top;
}

void DrawBitmap(HDC hDC, LONG X, LONG Y, HBITMAP hBitmap){
	if(hBitmap == NULL){return;}

	HDC hMemDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	BitBlt(hDC, X, Y, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
}

void SetClientRect(HWND hWnd, LONG Width, LONG Height){
	RECT crt;
	DWORD Style, ExStyle;

	SetRect(&crt, 0,0, Width, Height);
	Style = GetWindowLong(hWnd, GWL_STYLE);
	ExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);

	AdjustWindowRectEx(&crt, Style, GetMenu(hWnd) != NULL, ExStyle);
	if(Style & WS_VSCROLL){crt.right += GetSystemMetrics(SM_CXVSCROLL);}
	if(Style & WS_HSCROLL){crt.bottom += GetSystemMetrics(SM_CYHSCROLL);}

	SetWindowPos(hWnd, NULL, 0,0, crt.right - crt.left, crt.bottom - crt.top, SWP_NOZORDER);
}

void DrawCircle(HDC hdc, int x, int y, int iRadius) {
	Ellipse(hdc, x - iRadius, y - iRadius, x + iRadius, y + iRadius);
}

void DrawBall(HDC hdc, COLORREF Color){
	/* Edit :  */
	LONG Width,
		 Height,
		 OriginX,
		 OriginY,
		 MaxRadius,
		 Range;

	GetClientSize(g_hWnd, &Width, &Height);
	if(Width == 0 || Height == 0){return;}

	OriginX = Width >> 1;
	OriginY = Height >> 1;

	#define MIN(a,b) (((a) < (b)) ? (a) : (b))
	#define CLAMP(Min, Max, Value) (((Value) < (Min)) ? (Min) : ((Value) > (Max)) ? (Max) : (Value))

	MaxRadius = MIN(Width, Height);
	Range = MaxRadius >> 2;

	HBRUSH hBrush,
		   hOldBrush;

	hBrush = CreateSolidBrush(RGB(255, 0,0));
	hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	
	for(int i=50; i<Range; i++){
		DrawCircle(hdc, OriginX, OriginY, i);
	}

	DeleteObject(SelectObject(hdc, hOldBrush));
}

HBITMAP loadbmp(HDC hdc, LPCTSTR Path){
	HANDLE hFile = CreateFile(Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE){return NULL;}

	DWORD dwFileSize = GetFileSize(hFile, NULL);

	BITMAPFILEHEADER *fh = (BITMAPFILEHEADER*)malloc(dwFileSize);

	DWORD dwRead;
	ReadFile(hFile, fh, dwFileSize, &dwRead, NULL);
	CloseHandle(hFile);

	PBYTE ih = ((PBYTE)fh + sizeof(BITMAPFILEHEADER));
	HBITMAP ret = CreateDIBitmap(hdc, (BITMAPINFOHEADER*)ih, CBM_INIT, (PBYTE)fh + fh->bfOffBits, (BITMAPINFO*)ih, DIB_RGB_COLORS);

	free(fh);
	return ret;
}

void CenterWindow(HWND hWnd) {
	RECT Desktop,
		 Client;

	LONG X, Y,
		 Width,
		 Height;

	GetWindowRect(GetDesktopWindow(), &Desktop);
	GetWindowRect(hWnd, &Client);

	Width = Client.right - Client.left;
	Height = Client.bottom - Client.top;
	X = LONG((Desktop.right - Width) / 2);
	Y = LONG((Desktop.bottom - Height) / 2);

	MoveWindow(hWnd, X, Y, Width, Height, TRUE) ;
}

/* 
	TODO : Logic Design
	
	X = { x | 0 <= x <= 45 }
	n(X) = 46

	f(x) = return type : boolean
	<수정> 추가/삭제 속도 우선, 순차 탐색 

	Lookup Table [0, 45];
*/

void Sort(int* arr, int left, int right){
	int i = left,
		j = right,
		key = arr[ (left+right) / 2 ];

	while(i<=j){
		while(arr[i] < key){ i++; }
		while(arr[j] > key){ j--; }
		#define swap(a,b) {int n; n=a, a=b, b=n;}
		if(i <= j){ swap(arr[i],arr[j]); i++; j--; }
	}

	if(i < right){ Sort(arr, i, right); }
	if(j > left){ Sort(arr, left, j); }
}

BOOL Choose(){ return (BOOL)(rand() % 2); }

void InitialSetUp(){
	if(S == NULL){return;}

	for(int i=0; i<Pcs; i++){
		Push(S, CreateNode(i));
	}
}

/* 
	매우 간단하게 계산해보면 45개중 6개를 선택하면 되므로
	45C6 => C(45, 6) : 1 / 8,145,060

	현재 방식인 Boolean 타입 곧, 참/거짓 절반 확률의 문제로 계산해보면
	2^23 = 8,388,608로 약 23회 반복 필요

	-> 조합식 만들고 메모이제이션 활용해서 구조 단순히 작성해볼 것
*/

void Raffle(){
	/*
	   번호를 6개씩 한 쌍으로 묶은 후 버퍼에 담아 추첨하는 방식으로 진행한다.
	   필요에 따라 버퍼 자체도 임의 선택이 가능하게끔 한다.

	   데이터에 대한 접근이 빨라야하므로 기본 구조는 배열을 활용하며
	   큐 또는 스택을 이용해 데이터(또는 첨자)를 저장한다.
	*/
}
