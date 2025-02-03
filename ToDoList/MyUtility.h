#ifndef __MY_UTILITY_H_
#define __MY_UTILITY_H_

namespace MyUtility{
	static void CenterWindow(HWND hWnd) {
		RECT wrt, srt;
		LONG lWidth, lHeight;
		POINT NewPosition;

		GetWindowRect(hWnd, &wrt);
		GetWindowRect(GetDesktopWindow(), &srt);

		lWidth = wrt.right - wrt.left;
		lHeight = wrt.bottom - wrt.top;
		NewPosition.x = (srt.right - lWidth) / 2;
		NewPosition.y = (srt.bottom - lHeight) / 2;

		SetWindowPos(hWnd, NULL, NewPosition.x, NewPosition.y, lWidth, lHeight, SWP_NOZORDER);
	}
}
#endif
