#include "resource.h"

/*
	프로그램에 대한 아무런 설명이 추가되어 있지 않아 주석 남깁니다.

	해당 프로그램은 작성자의 컴퓨터 환경에 맞춰 제작된 도구입니다.
	게임 및 디자인 관련 작업을 하면서 필요한 색상값이나 프로세스 ID 등을 조회하는 용도로 사용되며
	초기 버전이라 Color.h 곧, HSV 색상 모델로의 변환은 추가되어 있지 않습니다.

	혹, HSV로의 변환이 필요한 경우 Minesweeper 프로젝트의 Headers\Color.h 파일을 참고하시기 바라며,
	이외의 CMYK 등의 Hex 변환은 관련 내용을 검색하면 간단한 산술 변환만으로도 가능하므로 따로 추가하지 않겠습니다.

	해당 프로그램의 용도를 분류해보면 크게 두 가지가 있으며 이는 다음과 같습니다.
	
	1. 프로그램 기본 정보 조회(프로그램 이름 및 클래스 이름, Process ID 등)
	2. 이미지 캡쳐 및 마우스 포인터가 가리키는 지점의 색상값 조회

	Windows 운영체제가 지원하는 API로만 제작되었으며 Linux 등의 Posix 환경에선 작동하지 않을 수 있습니다.
	또한, 이미지 캡쳐 기능 역시 제작자의 편의에 맞게 설계되었으므로 이에 대한 수정은 본인이 직접하여야 합니다.

	추후 시스템 보안과 관련된 기능이 추가될 수 있습니다.

	마지막 수정 : 주석 추가(2024.06.30)
*/

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

	wcex.lpfnWndProc = CaptureProc;
	wcex.lpszClassName = TEXT("CapturePopup");
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName = NULL;
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
	{WM_CREATE, OnCreate},
	{WM_DESTROY, OnDestroy},
};

MSGMAP submsg[] = {
	{WM_PAINT, OnChildPaint},
	{WM_CREATE, OnChildCreate},
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
