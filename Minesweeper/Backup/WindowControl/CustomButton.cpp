#include "CustomButton.h"

enum CustomButtonState { CB_NORMAL, CB_DOWN, CB_CHECK };

struct CustomButtonData{
	// Click(Mine, Number: 1~9), Check
	HBITMAP hStateBitmap[3];
	HBITMAP hNumberBitmap[9];
	CustomButtonState State;
	int Number;
};

static void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBitmap);
static void DeleteAll(struct CustomButtonData* Data);
static void ChangeState(HWND hWnd, CustomButtonState NewState);

LRESULT CALLBACK CustomButtonProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

class CustomButtonRegister{
	public:
		CustomButtonRegister(){
			WNDCLASS wc;

			wc.style = 0;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = sizeof(INT_PTR);
			wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.hIcon = NULL;
			wc.hInstance = GetModuleHandle(NULL);
			wc.lpszClassName = TEXT("CustomButton");
			wc.lpfnWndProc = CustomButtonProc;
			wc.lpszMenuName = NULL;

			RegisterClass(&wc);
		}
} CustomButtonRegisterObject;


LRESULT CALLBACK CustomButtonProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc, hMemDC;
	struct CustomButtonData* pData = NULL;

	switch(iMessage){
		case WM_CREATE:
			pData = (struct CustomButtonData*)calloc(sizeof(struct CustomButtonData), 1);
			SetWindowLongPtr(hWnd, 0, (INT_PTR)pData);
			return 0;

		case CBM_SETNUMBER:
			pData->Number = (int)wParam;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;

		case CBM_SETIMAGE:
			DeleteAll(pData);
			/* 이미지 추가 */

			return 0;

		case WM_LBUTTONDOWN:
			ChangeState(hWnd, CB_DOWN);
			SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetWindowLongPtr(hWnd, GWLP_ID), CBN_LCLICKED), (LPARAM)hWnd);
			return 0;

		case WM_RBUTTONDOWN:
			if(pData->State == CB_NORMAL){
				ChangeState(hWnd, CB_CHECK);
				SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetWindowLongPtr(hWnd, GWLP_ID), CBN_RCLICKED), (LPARAM)hWnd);
			}
			return 0;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			if(pData->State == CB_DOWN){
				DrawBitmap(hdc, 0,0, pData->hNumberBitmap[pData->Number + 1]);
			}else if(pData->State == CB_CHECK){
				DrawBitmap(hdc, 0,0, pData->hStateBitmap[pData->State]);
			}else{
				DrawBitmap(hdc, 0,0, pData->hStateBitmap[pData->State]);
			}
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			DeleteAll(pData);
			free(pData);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBitmap) {
	if(hBitmap == NULL){return;}

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	HDC hMemDC = CreateCompatibleDC(hdc);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	BitBlt(hdc, x,y, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
}

void DeleteAll(struct CustomButtonData* pData) {
	for(int i=0; i<3; i++){
		if(pData->hStateBitmap[i]){
			DeleteObject(pData->hStateBitmap[i]);
		}
	}

	for(int i=0; i<9; i++) {
		if(pData->hNumberBitmap[i]) {
			DeleteObject(pData->hNumberBitmap[i]);
		}
	}
}

void ChangeState(HWND hWnd, CustomButtonState NewState) {
	struct CustomButtonData* pData;

	pData = (CustomButtonData*)GetWindowLongPtr(hWnd, 0);
	if(pData->State == NewState){return;}

	pData->State = NewState;
}
