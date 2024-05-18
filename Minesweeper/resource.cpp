#define UNICODE
#define DEBUG
#include "resource.h"

RECT crt;
HPEN hWhitePen, hNullPen;
HBRUSH hBkBrush, hGrayBrush;
HBITMAP hClientBitmap, hOriginImage;

HDC hOriginImageDC;
BYTE* pData = NULL;
BITMAPINFOHEADER sih;

int StretchMode;
OSVERSIONINFO osv;

Control::Control(LONG _x, LONG _y, LONG _Width, LONG _Height, UINT _ID, HWND _hParent)
	: x(_x), y(_y), Width(_Width), Height(_Height), ID(_ID), hParent(_hParent)
{
	
}

Control::~Control(){

}

Button::Button(LONG _x, LONG _y, LONG _Width, LONG _Height, UINT _ID, HWND _hParent) : Control(_x, _y, _Width, _Height, _ID, _hParent)
{
	Edge = {};
	State = NORMAL;

	for(int i=0; i<4; i++){
		hBitmap[i] = NULL;
	}
}

Button::~Button(){
	
}

LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam){
	hBkBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	hGrayBrush = CreateSolidBrush(RGB(190, 190, 190));
	hWhitePen = CreatePen(PS_SOLID, 2, RGB(235,235,235));
	hNullPen = (HPEN)GetStockObject(NULL_PEN);

	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osv);

	if(osv.dwPlatformId >= VER_PLATFORM_WIN32_NT)
	{
		StretchMode = HALFTONE;
	}else{
		StretchMode = COLORONCOLOR;
	}


	SetTimer(hWnd, 1, 10, NULL);
	SendMessage(hWnd, WM_TIMER, 1, 0);
	return 0;
}

LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam){
	KillTimer(hWnd, 1);
	if(hBkBrush){ DeleteObject(hBkBrush); }
	if(hGrayBrush){DeleteObject(hGrayBrush);}
	if(hWhitePen){DeleteObject(hWhitePen);}
	if(hNullPen){DeleteObject(hNullPen);}
	if(hClientBitmap){DeleteObject(hClientBitmap);}
	if(hOriginImage){DeleteObject(hClientBitmap);}
	if(pData){free(pData);}
	if(hOriginImageDC){ DeleteDC(hOriginImageDC); }
	PostQuitMessage(0);
	return 0;
}

BITMAPINFOHEADER ih;
LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){

	#ifdef DEBUG
	pData = (BYTE*)loadbmp(&sih);

	if(pData != NULL) {
		HDC hdc = GetDC(hWnd);
		hOriginImageDC = CreateCompatibleDC(hdc);
		hOriginImage = CreateCompatibleBitmap(hdc, sih.biWidth, sih.biHeight);
		HGDIOBJ hOld = SelectObject(hOriginImageDC, hOriginImage);
		SetDIBitsToDevice(hOriginImageDC, 0,0, sih.biWidth, sih.biHeight, 0,0,0, sih.biHeight, pData, (BITMAPINFO*)&sih, DIB_RGB_COLORS);
		DeleteObject(hOld);
		ReleaseDC(hWnd, hdc);
	}
	#endif

	return 0;
}

LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam){

	return 0;
}

LRESULT OnRButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){

	return 0;
}

LRESULT OnRButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam){

	return 0;
}

LRESULT OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam){

	return 0;
}

LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam){
	if(wParam != SIZE_MINIMIZED) {
		if(hClientBitmap) {
			DeleteObject(hClientBitmap);
			hClientBitmap = NULL;
		}
	}
	return 0;
}

LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	if(hClientBitmap){
		HDC hMemDC = CreateCompatibleDC(hdc);
		HGDIOBJ hOld = SelectObject(hMemDC, hClientBitmap);

		BITMAP bmp;
		GetObject(hClientBitmap, sizeof(BITMAP), &bmp);

		BitBlt(hdc, 0,0, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

		SelectObject(hMemDC, hOld);
		DeleteDC(hMemDC);
	}
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam){
	static HWND hWndPoint;
	static TCHAR Caption[MAX_PATH], ClassName[MAX_PATH];
	static COLORREF Color;

	switch(wParam){
		case 1:
			{
				HDC hDC = GetDC(hWnd);
				HDC hMemDC = CreateCompatibleDC(hDC);

				if(hClientBitmap == NULL){
					GetClientRect(hWnd, &crt);
					hClientBitmap = CreateCompatibleBitmap(hDC, crt.right, crt.bottom);
				}

				HGDIOBJ hOld = SelectObject(hMemDC, hClientBitmap);
				FillRect(hMemDC, &crt, hBkBrush);

				POINT pt;
				GetCursorPos(&pt);

				SetBkMode(hMemDC, TRANSPARENT);
				SetStretchBltMode(hMemDC, StretchMode);

				#ifdef DEBUG
				if(hOriginImage != NULL){
					StretchBlt(hMemDC, 0,0, crt.right - crt.left, crt.bottom - crt.top, hOriginImageDC, 0,0, sih.biWidth, sih.biHeight, SRCCOPY);
				}

				hWndPoint = WindowFromPoint(pt);
				GetWindowText(hWndPoint, Caption, MAX_PATH);
				GetClassName(hWndPoint, ClassName, MAX_PATH);
				TextOut(hMemDC, crt.right - lstrlen(Caption) * 10, crt.top + 16, Caption, lstrlen(Caption));
				TextOut(hMemDC, crt.right - lstrlen(ClassName) * 10, crt.top + 32, ClassName, lstrlen(ClassName));
				if(hWndPoint == hWnd){
					ScreenToClient(hWnd, &pt);
				}
				Color = GetPixel(GetDC(hWndPoint), pt.x, pt.y);
				#else
				ScreenToClient(hWnd, &pt);
				Color = GetPixel(hDC, pt.x, pt.y);
				#endif

				INT R = (BYTE)(Color);
				INT G = ((BYTE)(((WORD)(Color)) >> 8));
				INT B = ((BYTE)((Color)>>16));

				TCHAR Cursor[MAX_PATH];
				wsprintf(Cursor, TEXT("Cursor Pos(%d, %d), Color = (R: %d, G: %d, B: %d)"), pt.x, pt.y, R,G,B);
				TextOut(hMemDC, crt.right - lstrlen(Cursor) * 10, crt.top + 48, Cursor, lstrlen(Cursor));

				HPEN hOldPen = (HPEN)SelectObject(hMemDC, hWhitePen);
				Rectangle(hMemDC, 10-1, 10-1, 26, 26);
				SelectObject(hMemDC, hOldPen);
				hOldPen = (HPEN)SelectObject(hMemDC, hNullPen);
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hGrayBrush);
				Rectangle(hMemDC, 10, 10, 26, 26);
				SelectObject(hMemDC, hOldPen);
				SelectObject(hMemDC, hOldBrush);

				SelectObject(hMemDC, hOld);
				DeleteDC(hMemDC);
				ReleaseDC(hWnd, hDC);
			}
			break;
	}

	InvalidateRect(hWnd, NULL, FALSE);
	return 0;
}

void* loadbmp(BITMAPINFOHEADER* ih)
{
	void *buf = NULL;
	TCHAR lpstrFile[MAX_PATH] = TEXT("");
	TCHAR FileName[MAX_PATH];
	TCHAR InitDir[MAX_PATH];
	TCHAR *path[MAX_PATH];
	TCHAR *pt = NULL;
	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = lpstrFile;
	ofn.lpstrFilter = TEXT("모든 파일(*.*)\0*.*\0비트맵 파일(*.bmp)\0*.bmp\0\0");
	ofn.lpstrTitle= TEXT("비트맵 파일을 선택하세요");
	ofn.lpstrDefExt = TEXT("txt");
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.hwndOwner = NULL;

	GetWindowsDirectory(InitDir, MAX_PATH);
	ofn.lpstrInitialDir = InitDir;

	if(GetOpenFileName(&ofn) != 0)
	{
		if(wcscmp(lpstrFile + ofn.nFileExtension, TEXT("bmp")) == 0)
		{
			HANDLE hFile = CreateFile(lpstrFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if(hFile != INVALID_HANDLE_VALUE)
			{
				DWORD dwRead;
				SetFilePointer(hFile, sizeof(BITMAPFILEHEADER), NULL, FILE_BEGIN);
				if(ReadFile(hFile, ih, sizeof(BITMAPINFOHEADER), &dwRead, NULL))
				{
					if(ih->biSizeImage == 0)
					{
						ih->biSizeImage = (((ih->biBitCount * ih->biWidth + 31) & ~31) >> 3) * ih->biHeight;
					}

					buf = malloc(ih->biSizeImage);
					if(ReadFile(hFile, buf, ih->biSizeImage, &dwRead, NULL))
					{
						CloseHandle(hFile);
						return buf;
					}else{
						MessageBox(NULL, TEXT("Failed to read bmp file data"), TEXT("Error"), MB_OK);
					}
				}else{
					MessageBox(NULL, TEXT("Failed to ReadFile"), TEXT("Error"), MB_OK);
				}

				CloseHandle(hFile);
			}else{
				MessageBox(NULL, TEXT("cannot open this file"), TEXT("Error"), MB_OK);
			}
		}else{
			MessageBox(NULL, TEXT("those not bmp file"), TEXT("Warning"), MB_OK);
		}
	}

	return NULL;
}
