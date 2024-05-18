#include "resource.h"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow){
	HANDLE hMutex;
	hMutex = CreateMutex(NULL, FALSE, TEXT("MinesweeperToyProjectExample_01"));
	if(GetLastError() == ERROR_ALREADY_EXISTS){
		CloseHandle(hMutex);
		HWND hOnce = FindWindow(NULL, CLASS_NAME);
		if(hOnce){
			ShowWindowAsync(hOnce, SW_SHOWNORMAL);
			SetForegroundWindow(hOnce);
		}
		return 0;
	}

	WNDCLASSEX wcex = {
		sizeof(wcex),
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		MAKEINTRESOURCE(IDR_MENU),
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

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	while(GetMessage(&msg, NULL, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

MSGMAP mainmsg[] = {
	{WM_TIMER, OnTimer},
	{WM_PAINT, OnPaint},
	{WM_SIZE, OnSize},
	{WM_MOUSEMOVE, OnMouseMove},
	{WM_LBUTTONDOWN, OnLButtonDown},
	{WM_LBUTTONUP, OnLButtonUp},
	{WM_RBUTTONDOWN, OnRButtonDown},
	{WM_RBUTTONUP, OnRButtonUp},
	{WM_CREATE, OnCreate},
	{WM_DESTROY, OnDestroy},
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	for(DWORD_PTR i=0; i<sizeof(mainmsg)/sizeof(mainmsg[0]); i++){
		if(mainmsg[i].iMessage == iMessage){
			return (*mainmsg[i].lpfnWndProc)(hWnd, wParam, lParam);
		}
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
