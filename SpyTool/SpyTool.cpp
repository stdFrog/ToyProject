#include "resource.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK CaptureProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow){
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

	wcex = {
		sizeof(wcex),
		CS_HREDRAW | CS_VREDRAW,
		CaptureProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		NULL,
		CHILD_CLASS_NAME,
		NULL
	};
	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				CLASS_NAME,
				CLASS_NAME,
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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
	{WM_MOVE, OnMove},
	{WM_COMMAND, OnCommand},
	{WM_LBUTTONDOWN, OnLButtonDown},
	{WM_LBUTTONUP, OnLButtonUp},
	{WM_CREATE, OnCreate},
	{WM_DESTROY, OnDestroy},
};

MSGMAP submsg[] = {
	{WM_TIMER, OnChildTimer},
	{WM_PAINT, OnChildPaint},
	{WM_LBUTTONDOWN, OnChildLButtonDown},
	{WM_LBUTTONUP, OnChildLButtonUp},
	{WM_CREATE, OnChildCreate},
	{WM_DESTROY, OnChildDestroy},
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	for(DWORD_PTR i=0; i<sizeof(mainmsg)/sizeof(mainmsg[0]); i++){
		if(mainmsg[i].iMessage == iMessage){
			return (*mainmsg[i].lpfnWndProc)(hWnd, wParam, lParam);
		}
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT CALLBACK CaptureProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	for(DWORD_PTR i=0; i<sizeof(submsg)/sizeof(submsg[0]); i++){
		if(submsg[i].iMessage == iMessage){
			return (*submsg[i].lpfnWndProc)(hWnd, wParam, lParam);
		}
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
