#define _WIN32_WINNT 0x0A00
#define _UNICODE
#include "..\\Resource\\Maze.h"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#define CLASS_NAME TEXT("Maze Solve")

static HBITMAP hBit;
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex = {
		sizeof(wcex),
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		NULL,
		CLASS_NAME,
		NULL
	};

	RegisterClassEx(&wcex);

	RECT crt;
	SetRect(&crt, 0,0, 300, 300);
	AdjustWindowRect(&crt, WS_POPUPWINDOW, FALSE);

	HWND hWnd = CreateWindow(
				CLASS_NAME,
				CLASS_NAME,
				WS_POPUPWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT, crt.right - crt.left, crt.bottom - crt.top,
				NULL,
				(HMENU)NULL,
				hInst,
				NULL
			);

	ShowWindow(hWnd, nCmdShow);

	Maze m;
	m.Init(hWnd);
	hBit = m.GetBitmap();

	srand(GetTickCount64());

	INT_PTR prev;

	MSG msg;
	while(1)
	{
		if(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT){return 0;}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			INT_PTR cur = GetTickCount64();
			if(cur - prev >= 40)
			{
				m.Update();
				m.Render();

				prev = cur;
			}
		}
	}

	return (int)msg.wParam;
}

bool CheckRect(int cx, int cy, int x, int y, int Alpha)
{
	if(x < Alpha)
	{
		return true;
	}

	if(x > cx - Alpha)
	{
		return true;
	}

	if(y < Alpha)
	{
		return true;
	}

	if(y > cy - Alpha)
	{
		return true;
	}

	return false;	
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch(iMessage)
	{
		case WM_NCHITTEST:
			{
				UINT nHit;
				POINT pt;
				RECT rt;

				nHit=DefWindowProc(hWnd,iMessage,wParam,lParam);

				pt.x=LOWORD(lParam);
				pt.y=HIWORD(lParam);

				GetClientRect(hWnd, &rt);

				ScreenToClient(hWnd,&pt);
				if ((nHit==HTCLIENT) && CheckRect(rt.right - rt.left, rt.bottom - rt.top, pt.x, pt.y, Align))
					nHit=HTCAPTION;
				return nHit;
			}

			/*
		case WM_NCHITTEST:
			nHit = DefWindowProc(hWnd, iMessage, wParam, lParam);
			if(nHit == HTCLIENT){
				nHit = HTCAPTION;
			}
			return nHit;
			*/

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				if(hBit)
				{
					HDC hMemDC = CreateCompatibleDC(hdc);
					HGDIOBJ hOld = SelectObject(hMemDC, hBit);

					BITMAP bmp;
					GetObject(hBit, sizeof(bmp), &bmp);
					BitBlt(hdc, 0,0, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

					SelectObject(hMemDC, hOld);
					DeleteDC(hMemDC);
				}
				EndPaint(hWnd, &ps);
			}
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
