#include <windows.h>
#define CLASS_NAME TEXT("Minesweeper")
#define IDW_BUTTON 100

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow){
	HANDLE hMutex;
	hMutex = CreateMutex(NULL, FALSE, TEXT("MinesweeperToyProject"));

	if(GetLastError() == ERROR_ALREADY_EXISTS){
		CloseHandle(hMutex);
		HWND hOnce = FindWindow(NULL, CLASS_NAME);
		if(hOnce){
			ShowWindowAsync(hOnce, SW_SHOWNORMAL);
			SetForegroundWindow(hOnce);
		}
		return 0;
	}
	srand(GetTickCount());

	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		NULL,
		NULL,
		CLASS_NAME,
	};

	RegisterClass(&wc);

	HWND hWnd = CreateWindow(
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

HWND** CreateButtons(int Row, int Column);
void DestroyButtons(HWND** Btns, int Row);
BOOL InitButtons(HWND hParent, HWND** Btns, int Width, int Height, int Row, int Column);

int** CreateGameBoard(int Row, int Column);
void DestroyGameBoard(int** Board, int Row);
BOOL Settings(int*** Board, int Row, int Column);
int* Randomization(int Scope, int MinesCount);

void DrawBoard(HWND hWnd, HDC hdc, HBITMAP hBitmap, int** GameBoard, int Row, int Column, int Width, int Height);
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBitmap);

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static HWND **hBtns;
	static int Row, Column;
	static int ** GameBoard;

	static struct tag_Button{
		int Width;
		int Height;
	}ButtonLayout;

	static BOOL bInit, bSettings;
	static RECT srt, rt;
	static int ID;

	LPMINMAXINFO pmmi;
	PAINTSTRUCT ps;
	HDC hdc, hMemDC;
	static HBITMAP hBitmap;

	switch(iMessage){
		case WM_CREATE:
			ButtonLayout.Width = 15;
			ButtonLayout.Height = 15;
			Row = Column = 20;
			if(hBtns == NULL){hBtns = CreateButtons(Row, Column);}
			if(hBtns){bInit = InitButtons(hWnd, hBtns, ButtonLayout.Width, ButtonLayout.Height, Row, Column);}
			if(!bInit){return -1;}
			SetTimer(hWnd, 2, 1000, NULL);
			return 0;

		case WM_TIMER:
			switch(wParam){
				case 2:
					KillTimer(hWnd, 2);
					if(!bInit){return -1;}
					if(GameBoard == NULL){GameBoard = CreateGameBoard(Row, Column);}
					if(GameBoard){bSettings = Settings(&GameBoard, Row, Column);}
					break;
			}
			return 0;

		case WM_COMMAND:
			ID = LOWORD(wParam) - IDW_BUTTON;
			{
				/*
				TCHAR buf[256];
				wsprintf(buf, TEXT("ID = %d, Data = %d"), ID, (GameBoard[ID / Row][ID % Column] == -1) ? 1000 : GameBoard[ID / Row][ID % Column]);
				SetWindowText(hWnd, buf);
				*/

				RECT ChildRect;
				GetWindowRect((HWND)lParam, &ChildRect);
				ScreenToClient(hWnd, (LPPOINT)&ChildRect);
				ScreenToClient(hWnd, (LPPOINT)&ChildRect+1);
				SetWindowLongPtr((HWND)lParam, GWL_STYLE, GetWindowLongPtr((HWND)lParam, GWL_STYLE) & ~WS_VISIBLE);
				InvalidateRect(hWnd, &ChildRect, TRUE);
			}
			return 0;

		case WM_GETMINMAXINFO:
			pmmi = (LPMINMAXINFO)lParam;

			pmmi->ptMinTrackSize.x = srt.right - srt.left;
			pmmi->ptMinTrackSize.y = srt.bottom - srt.top;
			pmmi->ptMaxTrackSize.x = srt.right - srt.left;
			pmmi->ptMaxTrackSize.y = srt.bottom - srt.top;
			return 0;

		case WM_SIZE:
			if(wParam != SIZE_MINIMIZED){
				if(hBitmap != NULL){
					DeleteObject(hBitmap);
					hBitmap = NULL;
				}

				if(bInit){
					SetRect(&srt, 0,0, 300, 300);
					AdjustWindowRect(&srt, GetWindowLongPtr(hWnd, GWL_STYLE), FALSE);
					SetWindowPos(hWnd, NULL, srt.left, srt.top, srt.right - srt.left, srt.bottom - srt.top, SWP_NOZORDER | SWP_NOMOVE);
				}
			}
			return 0;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			if(hBitmap == NULL){
				GetClientRect(hWnd, &rt);
				hBitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);
			}else{
				if(bSettings){
					DrawBoard(hWnd, hdc, hBitmap, GameBoard, Row, Column, ButtonLayout.Width, ButtonLayout.Height);
				}

				DrawBitmap(hdc, 0, 0, hBitmap);
			}

			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			if(hBitmap){DeleteObject(hBitmap);}
			DestroyGameBoard(GameBoard, Row);
			DestroyButtons(hBtns, Row);
			PostQuitMessage(0);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

HWND** CreateButtons(int Row, int Column){
	HWND** Btns = (HWND**)calloc(Row, sizeof(HWND*));

	for(int i=0; i<Column; i++){
		Btns[i] = (HWND*)calloc(Column, sizeof(HWND));
	}

	return Btns;
}

void DestroyButtons(HWND** Btns, int Row){
	if(Btns == NULL){return;}

	for(int i=0; i<Row; i++){
		free(Btns[i]);
	}

	free(Btns);
}

BOOL InitButtons(HWND hParent, HWND** Btns, int Width, int Height, int Row, int Column){
	if(Btns == NULL){return FALSE;}
	for(int i=0; i<Row; i++){
		for(int j=0; j<Column; j++){
			Btns[i][j] = CreateWindow(
						TEXT("button"),
						NULL,
						WS_BORDER | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
						j * Width, i * Height, Width, Height,
						hParent,
						(HMENU)(INT_PTR)(IDW_BUTTON + j + (i * Column)),
						GetModuleHandle(NULL),
						NULL
					);

			if(Btns[i][j] == NULL){return FALSE;}
		}
	}

	return TRUE;
}

int** CreateGameBoard(int Row, int Column){
	int **Board = (int**)calloc(Row, sizeof(int*));

	for(int i=0; i<Row; i++){
		Board[i] = (int*)calloc(Column, sizeof(int));
	}

	return Board;
}

void DestroyGameBoard(int** Board, int Row){
	if(Board == NULL){return;}

	for(int i=0; i<Row; i++){
		free(Board[i]);
	}

	free(Board);
}

struct tag_SelectNumber{
	int SelectRow, SelectColumn;
};

int* Randomization(int Scope, int MinesCount){
	int* CheckBox = (int*)calloc(Scope, sizeof(int));
	int* NumberList = (int*)calloc(MinesCount, sizeof(int));

	int i = 0, Number = -1;
	while(MinesCount > 1){
		Number = rand() % Scope;
		if(CheckBox[Number] != 0){continue;}

		--MinesCount;
		CheckBox[Number] = 1;
		NumberList[i++] = Number; 
	}

	free(CheckBox);
	return NumberList;
}

BOOL Settings(int*** Board, int Row, int Column){
	if(Board == NULL){return FALSE;}

	int MinesCount = Row + Column;
	int Scope = Row * Column;

	struct tag_SelectNumber* S  = (struct tag_SelectNumber*)calloc(MinesCount, sizeof(struct tag_SelectNumber));

	int SelectRow = 0, SelectColumn = 0;
	int Number = 2147483647;
	int MoveRow = 0;
	int MoveColumn = 0;

	int *NumberList = Randomization(Scope, MinesCount);

	for(int i=0; i<MinesCount; i++){
		Number = NumberList[i];
		SelectRow = Number / Row;
		SelectColumn = Number % Column;

		MoveRow = ((SelectRow == 0) ? 1 : (SelectRow == (Row-1)) ? -1 : 0);
		MoveColumn = ((SelectColumn == 0) ? 1 : (SelectColumn == (Column-1)) ? -1 : 0);

		if(MoveRow && MoveColumn){
			(*Board)[SelectRow][SelectColumn + MoveColumn] += 1;
			(*Board)[SelectRow + MoveRow][SelectColumn] += 1;
			(*Board)[SelectRow + MoveRow][SelectColumn + MoveColumn] += 1;
		}else if(MoveRow || MoveColumn){
			if(MoveRow){
				(*Board)[SelectRow][SelectColumn + 1] += 1;
				(*Board)[SelectRow][SelectColumn - 1] += 1;
				(*Board)[SelectRow + MoveRow][SelectColumn] += 1;
				(*Board)[SelectRow + MoveRow][SelectColumn + 1] += 1;
				(*Board)[SelectRow + MoveRow][SelectColumn - 1] += 1;
			}else{
				(*Board)[SelectRow + 1][SelectColumn] += 1;
				(*Board)[SelectRow - 1][SelectColumn] += 1;
				(*Board)[SelectRow][SelectColumn + MoveColumn] += 1;
				(*Board)[SelectRow + 1][SelectColumn + MoveColumn] += 1;
				(*Board)[SelectRow - 1][SelectColumn + MoveColumn] += 1;
			}
		}else{
			(*Board)[SelectRow][SelectColumn - 1] += 1;
			(*Board)[SelectRow][SelectColumn + 1] += 1;
			(*Board)[SelectRow - 1][SelectColumn] += 1;
			(*Board)[SelectRow - 1][SelectColumn - 1] += 1;
			(*Board)[SelectRow - 1][SelectColumn + 1] += 1;
			(*Board)[SelectRow + 1][SelectColumn] += 1;
			(*Board)[SelectRow + 1][SelectColumn - 1] += 1;
			(*Board)[SelectRow + 1][SelectColumn + 1] += 1;
		}

		S[i].SelectRow = SelectRow;
		S[i].SelectColumn = SelectColumn;
	}

	for(int i=0; i<MinesCount; i++){
		(*Board)[S[i].SelectRow][S[i].SelectColumn] = -1;
	}

	free(NumberList);
	free(S);

	return TRUE;
}

void DrawBoard(HWND hWnd, HDC hdc, HBITMAP hBitmap, int** GameBoard, int Row, int Column, int Width, int Height){
	if(hBitmap == NULL){return;}

	RECT rt;
	GetClientRect(hWnd, &rt);

	HDC hMemDC = CreateCompatibleDC(hdc);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	FillRect(hMemDC, &rt, GetSysColorBrush(COLOR_BTNFACE));
	for(int i=0; i<Row; i++){
		for(int j=0; j<Column; j++){
			TCHAR buf[256];
			if(GameBoard[i][j] == -1){
				wsprintf(buf, TEXT("B"));
			}else{
				wsprintf(buf, TEXT("%d"), GameBoard[i][j]);
			}
			TextOut(hMemDC, Width * j, Height * i, buf, lstrlen(buf));
		}
	}

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
}

void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBitmap){
	if(hBitmap == NULL){return;}

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	HDC hMemDC = CreateCompatibleDC(hdc);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);
	
	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
}
