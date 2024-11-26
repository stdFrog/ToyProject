#include <windows.h>
#define CLASS_NAME TEXT("Minesweeper")
#define IDW_BUTTON 100
#define MAX_OPEN 15

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GameBoardWndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK GameStateWndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK SubButtonProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GameOverChildProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);

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
		MainWndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		NULL,
		NULL,
		CLASS_NAME,
	};
	RegisterClass(&wc);

	wc.lpfnWndProc = GameStateWndProc;
	wc.lpszClassName = TEXT("GameStateChild");
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName = NULL;
	RegisterClass(&wc);

	wc.lpfnWndProc = GameBoardWndProc;
	wc.lpszClassName = TEXT("GameBoardChild");
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName = NULL;
	RegisterClass(&wc);

	wc.lpfnWndProc = GameOverChildProc;
	wc.lpszClassName = TEXT("GameOverChild");
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName = NULL;
	wc.style = CS_PARENTDC;
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

typedef struct tag_Node{
	int x,y;
}Node;

typedef struct tag_Queue{
	int Capacity;
	int Front;
	int Rear;
	struct tag_Node* Nodes;
}Queue;

COLORREF ColorList[] = {
	RGB(0, 0, 0),
	RGB(0, 0, 255),
	RGB(0, 255, 0),
	RGB(255, 0, 0)
};

void CreateQueue(Queue** Q, int Capacity);
void DestroyQueue(Queue* Q);
void Enqueue(Queue* Q, int x, int y);
struct tag_Node Dequeue(Queue* Q);
int GetSize(Queue* Q);
BOOL IsEmpty(Queue* Q);

enum tag_GameStatus {GAME_WAIT, GAME_RUNNING, GAME_OVER};
enum tag_ButtonData {BTN_ZERO, BTN_NUMBER, BTN_MINE, BTN_FLAG};

#define DEFAULT_BTN_SIZE 16
#define DEFAULT_BOARD_SIZE 20
#define DIV 4

struct tag_Layout{
	int x, y, Width, Height;
};

#define WM_SETINFO WM_USER+1
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static struct tag_Layout Layout, ButtonLayout, GameBoardLayout, GameStateLayout;
	static int Row, Column;
	static HWND hGameBoardWnd, hGameStateWnd;
	static RECT srt;
	LPMINMAXINFO pmmi;

	switch(iMessage){
		case WM_CREATE:
			Row = Column = DEFAULT_BOARD_SIZE;
			ButtonLayout.y = Row;
			ButtonLayout.x = Column;
			ButtonLayout.Width = ButtonLayout.Height = DEFAULT_BTN_SIZE;
			GameStateLayout.x = GameStateLayout.y = 0;
			GameStateLayout.Width = ButtonLayout.Width * Column;
			GameStateLayout.Height = ButtonLayout.Height * Row / DIV;
			GameBoardLayout.x = 0;
			GameBoardLayout.y = GameStateLayout.Height;
			GameBoardLayout.Width = GameStateLayout.Width;
			GameBoardLayout.Height = GameStateLayout.Height * DIV;
			Layout.Width = GameBoardLayout.Width;
			Layout.Height = GameBoardLayout.Height + GameStateLayout.Height;

			hGameStateWnd = CreateWindow(TEXT("GameStateChild"), NULL, WS_VISIBLE | WS_CHILD | WS_BORDER, GameStateLayout.x, GameStateLayout.y, GameStateLayout.Width, GameStateLayout.Height, hWnd, (HMENU)NULL, GetModuleHandle(NULL), NULL);
			hGameBoardWnd = CreateWindow(TEXT("GameBoardChild"), NULL, WS_VISIBLE | WS_CHILD | WS_BORDER, GameBoardLayout.x, GameBoardLayout.y, GameBoardLayout.Width, GameBoardLayout.Height, hWnd, (HMENU)NULL, GetModuleHandle(NULL), NULL);

			SendMessage(hGameStateWnd, WM_SETINFO, (WPARAM)2, (LPARAM)&GameStateLayout);

			SendMessage(hGameBoardWnd, WM_SETINFO, (WPARAM)1, (LPARAM)&ButtonLayout);
			SendMessage(hGameBoardWnd, WM_SETINFO, (WPARAM)2, (LPARAM)&GameStateLayout);
			SendMessage(hGameBoardWnd, WM_SETINFO, (WPARAM)3, (LPARAM)&GameBoardLayout);
			SendMessage(hGameBoardWnd, WM_SETINFO, (WPARAM)4, (LPARAM)0);
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
				SetRect(&srt, 0,0, Layout.Width, Layout.Height);
				AdjustWindowRect(&srt, GetWindowLongPtr(hWnd, GWL_STYLE), FALSE);
				SetWindowPos(hWnd, NULL, srt.left, srt.top, srt.right - srt.left, srt.bottom - srt.top, SWP_NOZORDER | SWP_NOMOVE);
				SetWindowPos(hGameStateWnd, NULL, GameStateLayout.x, GameStateLayout.y, GameStateLayout.Width, GameStateLayout.Height, SWP_NOZORDER);
				SetWindowPos(hGameBoardWnd, NULL, GameBoardLayout.x, GameBoardLayout.y, GameBoardLayout.Width, GameBoardLayout.Height, SWP_NOZORDER);
			}
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

#define RBTN_CLICKED WM_USER+2
static WNDPROC OldProc;
LRESULT CALLBACK GameBoardWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static struct tag_Layout ButtonLayout, GameStateLayout, GameBoardLayout;

	static HWND **hBtns;
	static int Row, Column, cnt;
	static int** GameBoard;
	static int** CopyBoard;

	static BOOL bInit, bSettings, bReady, bClicked, bLock;
	static RECT srt, rt;
	static int ID;

	static Queue* Q;
	int x, y, xx, yy;
	static int dx[] = {-1, 0, 1, 0}, dy[] = {0, -1, 0, 1};

	DWORD Style;
	HWND SelectWnd;
	RECT SelectWndRect;
	Node CurrentNode;
	static HWND hGameOverChild, hTempButton;
	static enum tag_GameStatus GameStatus;
	static BYTE FlagBits[] = {
		0xff, 0xff,
		0xdf, 0xff,
		0xcf, 0xff,
		0xc7, 0xff,
		0xc3, 0xff,
		0xc1, 0xff,
		0xdf, 0xff,
		0xdf, 0xff,
		0xdf, 0xff,
		0x8f, 0xff,
		0xff, 0xff,
	};
	INT_PTR Prop;
	HWND FlagWnd;

	PAINTSTRUCT ps;
	HDC hdc, hMemDC;
	static HBITMAP hButtonBitmap, hFlagBitmap;
	DWORD TextBackgroundColor;
	COLORREF BtnColor, OldColor;
	TCHAR TextFace[256], buf[256];
	HFONT hFont, hOldFont;
	TEXTMETRIC tm;
	SIZE szSize;
	BOOL bBit;
	int Number, BaseLine, PaddingLR, PaddingTB;
	static BYTE MineBits[] = {
		0xef, 0xff,
		0x6d, 0xff,
		0xc7, 0xff,
		0xb3, 0xff,
		0x30, 0xfe,
		0x83, 0xff,
		0xc7, 0xff,
		0x6d, 0xff,
		0xef, 0xff
	};
	HBITMAP hMineBitmap;
	HPEN hPen, hOldPen;

	switch(iMessage){
		case WM_SETINFO:
			switch(wParam){
				case 1:
					ButtonLayout = *(struct tag_Layout*)lParam;
					Row = ButtonLayout.y;
					Column = ButtonLayout.x;
					break;

				case 2:
					GameStateLayout = *(struct tag_Layout*)lParam;
					break;

				case 3:
					GameBoardLayout = *(struct tag_Layout*)lParam;
					break;

				case 4:
					hGameOverChild = CreateWindow(TEXT("GameOverChild"), NULL, WS_CHILD | WS_BORDER, 0,0,0,0, hWnd, (HMENU)0, GetModuleHandle(NULL), NULL);
					hTempButton = CreateWindow(TEXT("button"), NULL, WS_CHILD, 0,0,0,0, hWnd, (HMENU)NULL, GetModuleHandle(NULL), NULL);
					OldProc = (WNDPROC)SetClassLongPtr(hTempButton, GCLP_WNDPROC, (LONG_PTR)SubButtonProc);
					if(hBtns == NULL){hBtns = CreateButtons(Row, Column);}
					if(hBtns){bInit = InitButtons(hWnd, hBtns, ButtonLayout.Width, ButtonLayout.Height, Row, Column);}
					if(!bInit){ return -1; }
					if(GameBoard == NULL){GameBoard = CreateGameBoard(Row, Column);}
					if(GameBoard){ bSettings = Settings(&GameBoard, Row, Column); }
					if(bSettings){
						CopyBoard = CreateGameBoard(Row, Column);
						CreateQueue(&Q, 100);
						hdc = GetDC(hWnd);
						hFlagBitmap = (HBITMAP)CreateBitmap(8, 11, 1, 1, FlagBits);
						ReleaseDC(hWnd, hdc);
						GameStatus = (enum tag_GameStatus)(CopyBoard && Q && hFlagBitmap);
					}
					break;
			}
			return 0;


		case WM_TIMER:
			switch(wParam){
				case 3:
					if(IsEmpty(Q) || cnt == MAX_OPEN){
						KillTimer(hWnd, 3);
						while(!IsEmpty(Q)){Dequeue(Q);}
						// if(bLock){LockWindowUpdate(NULL); bLock = FALSE;} 
						cnt = 0;
						break;
					}

					/* 아래에서 사용하는 x는 행을 의미하고, y는 열을 의미한다. */
					CurrentNode = Dequeue(Q);
					x = CurrentNode.x;
					y = CurrentNode.y;
					if(x < 0 || y < 0 || x >= Row || y >= Column){break;}

					SelectWnd = hBtns[x][y];
					Style = GetWindowLongPtr(SelectWnd, GWL_STYLE);
					if(!(Style & WS_VISIBLE)){break;}
					if((INT_PTR)GetProp(SelectWnd, TEXT("Flag"))){break;}

					if(bClicked && GameBoard[x][y] != 0){
						SetWindowLongPtr(SelectWnd, GWL_STYLE, Style & ~WS_VISIBLE);
						GetWindowRect(SelectWnd, &SelectWndRect);
						ScreenToClient(hWnd, (LPPOINT)&SelectWndRect);
						ScreenToClient(hWnd, (LPPOINT)&SelectWndRect+1);
						InvalidateRect(hWnd, &SelectWndRect, TRUE);
						if(GameBoard[x][y] == -1){
							/* 게임 종료 */
							GameStatus = GAME_OVER;
							HDWP hdwp = BeginDeferWindowPos(Row * Column);
							for(int i=0; i<Row; i++){
								for(int j=0; j<Column; j++){
									hdwp = DeferWindowPos(hdwp, hBtns[i][j], NULL, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
								}
							}
							EndDeferWindowPos(hdwp);
							SetWindowPos(hGameOverChild, NULL, rt.left, rt.top + (rt.bottom - rt.top) / 4, (rt.right - rt.left), (rt.bottom - rt.top) / 2, SWP_NOZORDER);
							SetTimer(hWnd, 4, 1000, NULL);
						}
						break;
					}

					if(GameBoard[x][y] == 0 && CopyBoard[x][y] == 0){
						// if(bLock == FALSE){ bLock = LockWindowUpdate(hWnd);}
						bClicked = FALSE;
						CopyBoard[x][y] = 1;
						cnt++;
						SetWindowLongPtr(SelectWnd, GWL_STYLE, Style & ~WS_VISIBLE);
						GetWindowRect(SelectWnd, &SelectWndRect);
						ScreenToClient(hWnd, (LPPOINT)&SelectWndRect);
						ScreenToClient(hWnd, (LPPOINT)&SelectWndRect+1);
						InvalidateRect(hWnd, &SelectWndRect, TRUE);
						if(cnt == MAX_OPEN){break;}

						Enqueue(Q, x, y - 1);
						Enqueue(Q, x - 1, y);
						Enqueue(Q, x, y + 1);
						Enqueue(Q, x + 1, y);
					}
					break;

				case 4:
					if(IsWindowVisible(hGameOverChild)){
						ShowWindow(hGameOverChild, SW_HIDE);
					}else{
						ShowWindow(hGameOverChild, SW_SHOW);
					}
					break;
			}
			return 0;

		case WM_COMMAND:
			if(GameStatus != GAME_RUNNING){return 0;}
			ID = LOWORD(wParam) - IDW_BUTTON;

			if(HIWORD(wParam) == RBTN_CLICKED){
				/* 우측 마우스 버튼 */
				FlagWnd = hBtns[ID / Row][ID % Column];
				Prop = (INT_PTR)GetProp(FlagWnd, TEXT("Flag"));

				if(Prop){
					RemoveProp(FlagWnd, TEXT("Flag"));
					hFlagBitmap = (HBITMAP)SendMessage(FlagWnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hButtonBitmap);
					SetWindowPos(FlagWnd, NULL, 0,0,0,0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
				}else{
					SetProp(FlagWnd, TEXT("Flag"), (HANDLE)TRUE);
					hButtonBitmap = (HBITMAP)SendMessage(FlagWnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hFlagBitmap);
					SetWindowPos(FlagWnd, NULL, 0,0,0,0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				}

				GetWindowRect(FlagWnd, &SelectWndRect);
				ScreenToClient(hWnd, (LPPOINT)&SelectWndRect);
				ScreenToClient(hWnd, (LPPOINT)&SelectWndRect+1);
				InvalidateRect(hWnd, &SelectWndRect, TRUE);
			}else if(HIWORD(wParam) == BN_CLICKED){
				/* 좌측 마우스 버튼 */
				bClicked = TRUE;
				Enqueue(Q, ID / Row, ID % Column);
				SetTimer(hWnd, 3, 15, NULL);
			}
			return 0;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rt);
			FillRect(hdc, &rt, GetSysColorBrush(COLOR_BTNFACE));

			#define GET_RED(V)		((BYTE)(((DWORD_PTR)(V)) & 0xff))
			#define GET_GREEN(V)	((BYTE)(((DWORD_PTR)(((WORD)(V))) >> 8) & 0xff))
			#define GET_BLUE(V)		((BYTE)(((DWORD_PTR)(V >> 16)) & 0xff))
			#define SET_RGB(R,G,B)	((COLORREF)(((BYTE)(R) | ((WORD)((BYTE)(G)) << 8)) | (((DWORD)(BYTE)(B)) << 16)))

			TextBackgroundColor = GetSysColor(COLOR_BTNFACE);
			BtnColor = SET_RGB(GET_RED(TextBackgroundColor), GET_GREEN(TextBackgroundColor), GET_BLUE(TextBackgroundColor));
			OldColor = SetBkColor(hdc, BtnColor);

			GetTextFace(hMemDC, sizeof(TextFace), TextFace);
			hFont = CreateFont(ButtonLayout.Height / 4 * 3, ButtonLayout.Width / 4 * 3, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TextFace);
			hOldFont = (HFONT)SelectObject(hdc, hFont);

			GetTextMetrics(hdc, &tm);
			SetTextAlign(hdc, TA_CENTER);

			hMineBitmap = (HBITMAP)CreateBitmap(9, 9, 1, 1, MineBits);

			for(int i=0; i<Row; i++){
				for(int j=0; j<Column; j++){
					bBit = FALSE;
					Number = GameBoard[i][j];
					if(GameBoard[i][j] == -1){
						bBit = TRUE;
					}else if(GameBoard[i][j] == 0){
						wsprintf(buf, TEXT(" "), Number);
					}else{
						wsprintf(buf, TEXT("%d"), Number);
					}

					if(bBit){
						HDC hMineDC = CreateCompatibleDC(hdc);
						HBITMAP hMineBitmapOld = (HBITMAP)SelectObject(hMineDC, hMineBitmap);

						BITMAP bmp;
						GetObject(hMineBitmap, sizeof(BITMAP), &bmp);
						SetTextColor(hdc, ColorList[0]);
						BitBlt(hdc, ButtonLayout.Width * j + bmp.bmWidth / 3 + 2, ButtonLayout.Height * i + bmp.bmHeight / 3 + 2, bmp.bmWidth, bmp.bmHeight, hMineDC, 0,0, SRCCOPY);

						SelectObject(hMineDC, hMineBitmapOld);
						DeleteDC(hMineDC);
					}else{
						GetTextExtentPoint32(hdc, buf, lstrlen(buf), &szSize);
						SetTextColor(hdc, ColorList[Number]);

						BaseLine = szSize.cx / 2;
						PaddingLR = szSize.cx / 4;
						PaddingTB = tm.tmHeight / 4;
						TextOut(hdc, ButtonLayout.Width * j + BaseLine + PaddingLR, ButtonLayout.Height * i + PaddingTB, buf, lstrlen(buf));
					}
				}
			}

			hPen = CreatePen(PS_SOLID, 1, SET_RGB(GET_RED(BtnColor) - 30, GET_GREEN(BtnColor) - 30, GET_BLUE(BtnColor) - 30));
			hOldPen = (HPEN)SelectObject(hdc, hPen);

			for(int i=0; i<Row; i++){
				MoveToEx(hdc, rt.left, i * ButtonLayout.Height, NULL);
				LineTo(hdc, rt.right, i * ButtonLayout.Height);
			}

			for(int i=0; i<Column; i++){
				MoveToEx(hdc, i * ButtonLayout.Width, rt.top, NULL);
				LineTo(hdc, i * ButtonLayout.Width, rt.bottom);
			}

			SetBkColor(hdc, OldColor);
			DeleteObject(SelectObject(hdc, hOldFont));
			DeleteObject(SelectObject(hdc, hOldPen));
			DeleteObject(hMineBitmap);
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			if(Q){DestroyQueue(Q);}
			if(CopyBoard){DestroyGameBoard(CopyBoard, Row);}
			if(GameBoard){DestroyGameBoard(GameBoard, Row);}
			if(hBtns){DestroyButtons(hBtns, Row);}
			if(hFlagBitmap){DeleteObject(hFlagBitmap);}
			if(OldProc){SetClassLongPtr(hTempButton, GCLP_WNDPROC, (LONG_PTR)OldProc);}
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT CALLBACK GameStateWndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam){
	static struct tag_Layout ButtonLayout, GameStateLayout, GameBoardLayout;

	switch(iMessage){
		case WM_CREATE:
			return 0;

		case WM_SETINFO:
			switch(wParam){
				case 0:
					break;

				case 1:
					ButtonLayout = *(struct tag_Layout*)lParam;
					break;

				case 2:
					GameStateLayout = *(struct tag_Layout*)lParam;
					break;

				case 3:
					GameBoardLayout = *(struct tag_Layout*)lParam;
					break;
			}
			return 0;

	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}


LRESULT CALLBACK GameOverChildProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam){
	TEXTMETRIC tm;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rt;
	TCHAR buf[0xf] = TEXT("GAME OVER");

	switch(iMessage){
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rt);
			SetTextAlign(hdc, TA_CENTER);
			GetTextMetrics(hdc, &tm);
			TextOut(hdc, rt.right / 2, rt.bottom / 2 - tm.tmHeight / 2, buf, lstrlen(buf));
			EndPaint(hWnd, &ps);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT CALLBACK SubButtonProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	switch(iMessage){
		case WM_RBUTTONDOWN:
			SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)MAKEWPARAM(GetWindowLongPtr(hWnd, GWLP_ID), RBTN_CLICKED),(LPARAM)hWnd);
			break;
	}

	return CallWindowProc(OldProc, hWnd, iMessage, wParam, lParam);
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
						WS_BORDER | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_BITMAP,
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

void CreateQueue(Queue** Q, int Capacity){
	(*Q) = (Queue*)malloc(sizeof(Queue));
	(*Q)->Nodes = (struct tag_Node*)malloc(sizeof(Node) * (Capacity + 1));

	(*Q)->Capacity = Capacity;
	(*Q)->Front = (*Q)->Rear = 0;
}

void DestroyQueue(Queue* Q){
	free(Q->Nodes);
	free(Q);
}

void Enqueue(Queue* Q, int x, int y){
	int Position = 0;

	if(Q->Rear == Q->Capacity){
		Position = Q->Rear;
		Q->Rear = 0;
	}else{
		Position = Q->Rear++;
	}

	Q->Nodes[Position].x = x;
	Q->Nodes[Position].y = y;
}

struct tag_Node Dequeue(Queue* Q){
	int Position = Q->Front;
	
	if(Q->Front == Q->Capacity){
		Q->Front = 0;
	}else{
		Q->Front++;
	}

	return Q->Nodes[Position];
}

int GetSize(Queue* Q){
	if(Q->Front <= Q->Rear){
		return Q->Rear - Q->Front;
	}else{
		return Q->Rear + (Q->Capacity - Q->Front) + 1;
	}
}

BOOL IsEmpty(Queue* Q){
	return (BOOL)(Q->Front == Q->Rear);
}

