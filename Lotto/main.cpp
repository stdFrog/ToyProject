#define _WIN32_WINNT 0x0A00
#include <windows.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#define CLASS_NAME TEXT("Lotto")

#define QUIT -1
#define FAILURE 0
#define SUCCESS 1
#define _TEXTOUT(DC, X, Y, TEXT) TextOut(DC, X, Y, TEXT, lstrlen(TEXT));

#include "PriorityQueue.h"
PriorityQueue* PQ = CreateQueue(100);
PriorityQueue* Lottery = CreateQueue(100);
PQNode Popped;

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
BOOL Choose();
void InitialSetUp();
void Raffle();

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
			osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			if(osv.dwPlatformId >= VER_PLATFORM_WIN32_NT){
				StretchMode = HALFTONE;
			}else{
				StretchMode = COLORONCOLOR;
			}
			InitialSetUp();
			SetTimer(hWnd, 1, 10, NULL);
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

				if(GetAsyncKeyState(VK_LBUTTON) & 0x8000){ Raffle(); }

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
	if(PQ){DestroyQueue(PQ);}
	if(Lottery){DestroyQueue(Lottery);}
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
	정렬 및 탐색 속도 우선: 배열 활용

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

BOOL Choose(){
	static BOOL Init = FALSE;
	if(Init == FALSE){srand(GetTickCount64());}

	return (BOOL)(rand() % 2);
}

const int LookupTable[] = {
	0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35,
	36, 37, 38, 39, 40, 41,
	42, 43, 44, 45
};

const int Pcs = sizeof(LookupTable)/sizeof(LookupTable[0]);

void InitialSetUp(){
	while(!IsEmpty(PQ)){ Dequeue(PQ, &Popped); }

	for(int i=0; i<Pcs; i++){ 
		PQNode NewNode = {LookupTable[i], NULL};
		Enqueue(PQ, NewNode);
	}
}

/* 버튼이나 레버 따위를 만든 후 애니메이션 추가해도 좋음 */
void Raffle(){
	if(PQ->UsedSize == 0){return;}

	while(!Once(PQ)){ if(Choose()){ Dequeue(PQ, &Popped); } }
	Dequeue(PQ, &Popped);
	Enqueue(Lottery, Popped);
}
