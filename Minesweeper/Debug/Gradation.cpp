#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include "Color.h"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#define CLASS_NAME TEXT("Test")

/*
	Color 클래스와 색상값 테스트를 위해 만들어진 예제입니다
	해당 파일과 Color.o 파일을 링크하여 컴파일하면 예시 프로그램이 만들어집니다.
*/

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
	SetRect(&crt, 0,0, 800, 600);
	AdjustWindowRectEx(&crt, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);

	HWND hWnd = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				CLASS_NAME,
				CLASS_NAME,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT, crt.right - crt.left, crt.bottom - crt.top,
				NULL,
				(HMENU)NULL,
				hInst,
				NULL
			);

	ShowWindow(hWnd, nCmdShow);

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
			// TODO : Update, Render, Time Tick
			WaitMessage();
		}
	}

	return (int)msg.wParam;
}


void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP hBit = NULL;
	static RECT crt;
	static float PI = 3.1415926535897932384626433832795L;
	static float TwoPI = PI * 2.f;

	switch(iMessage)
	{
		case WM_CREATE:
			SetTimer(hWnd, 1, 10, NULL);
			return 0;
			
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				
				if(hBit)
				{
					DrawBitmap(hdc, 0,0, hBit);
				}
				
				EndPaint(hWnd, &ps);
			}
			return 0;

		case WM_TIMER:
			switch(wParam){
				case 1:
					{
						
						HDC hdc = GetDC(hWnd);
						HDC hMemDC = CreateCompatibleDC(hdc);

						if(hBit == NULL){
							GetClientRect(hWnd, &crt);
							hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
						}
						HGDIOBJ hOld = SelectObject(hMemDC, hBit);
						FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));

						{
							HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, GetStockObject(NULL_BRUSH));

							Color HSVColor(0.f, 1.f, 0.89f, TRUE);

							// TwoPI 상수값은 색 공간 좌표를 변환하는 기본 공식에 사용되므로 이를 명확히 표현하기 위해 소거하지 않았다.
							static float increase = 1.f / (300.f * 200.f) * TwoPI;
							float rad = 0.f;

							for(int i=10; i<210; i++){
								for(int j=410; j<710; j++){
									HSVColor._H = rad / TwoPI;
									SetPixel(hMemDC, j, i, HSVColor.ToColor());
									rad += increase;
								}
							}

							typedef struct tag_Vertex{
								float x,y;

								tag_Vertex(float X = 0.f, float Y = 0.f) : x(X), y(Y) {}

								const struct tag_Vertex operator +(const struct tag_Vertex& Other) const{
									return tag_Vertex(x + Other.x, y + Other.y);
								}

								const struct tag_Vertex operator -(const struct tag_Vertex& Other) const{
									return tag_Vertex(x - Other.x, y - Other.y);
								}

								const struct tag_Vertex operator *(const struct tag_Vertex& Other) const{
									return tag_Vertex(x * Other.x, y * Other.y);
								}

								const struct tag_Vertex operator /(const struct tag_Vertex& Other) const{
									return tag_Vertex(x / Other.x, y / Other.y);
								}
							}Vertex;

							Vertex LT, RB;
							LT = {10.f, 10.f};
							RB = {310.f, 210.f};

							Vertex VertexRange = RB - LT;

							Color LTColor = Color::LightGray;
							Color RBColor = Color::WhiteGray;
							Color ColorRange = RBColor - LTColor;

							/* 가중치 없이 색을 흩뿌리기만 한다 */
							static float increase2 = 1.f / (VertexRange.x * VertexRange.y) * ColorRange.MaxColor();

							for(int i=LT.y; i<RB.y; i++){
								for(int j=LT.x; j<RB.x; j++){
									LTColor = LTColor + increase2;
									SetPixel(hMemDC, j, i, LTColor);
								}
							}

							SelectObject(hMemDC, hOldBrush);
						}

						SelectObject(hMemDC, hOld);
						DeleteDC(hMemDC);
						ReleaseDC(hWnd, hdc);
					}
					break;
			}

			InvalidateRect(hWnd, NULL, FALSE);
			return 0;

		case WM_DESTROY:
			if(hBit){DeleteObject(hBit);}
			KillTimer(hWnd, 1);
			PostQuitMessage(0);
			return 0;
	}
	
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
	BITMAP bmp;
	GetObject(hBit, sizeof(BITMAP), &bmp);

	HDC hMemDC = CreateCompatibleDC(hdc);
	HGDIOBJ hOld = SelectObject(hMemDC, hBit);

	BitBlt(hdc, x,y, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
}
