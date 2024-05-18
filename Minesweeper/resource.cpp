#define UNICODE
#include "resource.h"

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
	return 0;
}

LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam){
	PostQuitMessage(0);
	return 0;
}

LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){

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

LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam){

	return 0;
}
