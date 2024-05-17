#include "resource.h"

UINT Control::LocalID = 0;

Control::Control(LONG _x, LONG _y, LONG _Width, LONG _Height, UINT _ID, HWND _hParent)
	: x(_x), y(_y), Width(_Width), Height(_Height), ID(_ID), hParent(_hParent)
{
	
}

Control::~Control(){

}

Button::Button(LONG _x, LONG _y, LONG _Width, LONG _Height, UINT _ID, HWND _hParent) : Control(_x, _y, _Width, _Height, _ID, _hParent){

}

Button::~Button(){
	
}
