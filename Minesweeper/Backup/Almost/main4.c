#include "resource.h"
#include <windows.h>
#define CLASS_NAME TEXT("Minesweeper")
#define IDW_BUTTON 100
#define MAX_OPEN 15

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
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
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		NULL,
		NULL,
		CLASS_NAME,
	};
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

void DrawBoard(HWND hWnd, HDC hdc, HBITMAP hBitmap, int** GameBoard, int Row, int Column, int Width, int Height);
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBitmap);

typedef struct tag_Node{
	int x,y;
}Node;

typedef struct tag_Queue{
	int Capacity;
	int Front;
	int Rear;
	struct tag_Node* Nodes;
}Queue;

void CreateQueue(Queue** Q, int Capacity);
void DestroyQueue(Queue* Q);
void Enqueue(Queue* Q, int x, int y);
struct tag_Node Dequeue(Queue* Q);
int GetSize(Queue* Q);
BOOL IsEmpty(Queue* Q);

enum tag_GameStatus {GAME_WAIT, GAME_RUNNING, GAME_OVER};
enum tag_ButtonData {BTN_ZERO, BTN_NUMBER, BTN_BOMB, BTN_FLAG};

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

#define RBTN_CLICKED WM_USER+1
static HWND hParent;
static WNDPROC OldProc;
LRESULT CALLBACK SubButtonProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	switch(iMessage){
		case WM_RBUTTONDOWN:
			SendMessage(hParent, WM_COMMAND, (WPARAM)MAKEWPARAM(GetWindowLongPtr(hWnd, GWLP_ID), RBTN_CLICKED),(LPARAM)hWnd);
			break;
	}

	return CallWindowProc(OldProc, hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static HWND **hBtns;
	static int Row, Column, cnt;
	static int** GameBoard;
	static int** CopyBoard;

	static struct tag_Button{
		int Width;
		int Height;
	}ButtonLayout;

	static BOOL bInit, bSettings, bReady, bClicked, bLock;
	static RECT srt, rt;
	static int ID;

	LPMINMAXINFO pmmi;
	PAINTSTRUCT ps;
	HDC hdc, hMemDC;
	static HBITMAP hBitmap, hButtonBitmap, hFlagBitmap;

	static Queue* Q;
	int x, y, xx, yy;
	static int dx[] = {-1, 0, 1, 0}, dy[] = {0, -1, 0, 1};

	DWORD Style;
	HWND SelectWnd;
	RECT SelectWndRect;
	Node CurrentNode;
	static HWND hGameOverChild, hTempButton;
	static enum tag_GameStatus GameStatus;
	int BitsPerPel;
	static BYTE Bits[] = {
		0xff, 0xff,
		0xdf, 0xff,
		0xcf, 0xff,
		0xc7, 0xff,
		0xc3, 0xff,
		0xc1, 0xff,
		0xd0, 0xff,
		0xd0, 0xff,
		0xd0, 0xff,
		0x8f, 0xff,
		0xff, 0xff
	};

	switch(iMessage){
		case WM_CREATE:
			hParent = hWnd;
			ButtonLayout.Width = 16;
			ButtonLayout.Height = 16;
			Row = Column = 20;
			hTempButton = CreateWindow(TEXT("button"), NULL, WS_CHILD, 0,0,0,0, hWnd, (HMENU)NULL, GetModuleHandle(NULL), NULL);
			OldProc = (WNDPROC)SetClassLongPtr(hTempButton, GCLP_WNDPROC, (LONG_PTR)SubButtonProc);
			if(hBtns == NULL){hBtns = CreateButtons(Row, Column);}
			if(hBtns){bInit = InitButtons(hWnd, hBtns, ButtonLayout.Width, ButtonLayout.Height, Row, Column);}
			if(!bInit){return -1;}
			hGameOverChild = CreateWindow(TEXT("GameOverChild"), NULL, WS_CHILD | WS_BORDER, 0,0,0,0, hWnd, (HMENU)0, GetModuleHandle(NULL), NULL);
			SetTimer(hWnd, 2, 1000, NULL);
			return 0;

		case WM_TIMER:
			switch(wParam){
				case 2:
					KillTimer(hWnd, 2);
					if(!bInit){return -1;}
					if(GameBoard == NULL){GameBoard = CreateGameBoard(Row, Column);}
					if(GameBoard){ bSettings = Settings(&GameBoard, Row, Column); }
					if(bSettings){
						CopyBoard = CreateGameBoard(Row, Column);
						CreateQueue(&Q, 100);
						// hFlagBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_FLAGBITMAP));
						hdc = GetDC(hWnd);
						BitsPerPel = GetDeviceCaps(hdc, BITSPIXEL);
						// hFlagBitmap = (HBITMAP)CreateBitmap(8, 8, 1, 1, Bits);
						hFlagBitmap = (HBITMAP)CreateBitmap(8, 8, 1, 1, Bits);
						ReleaseDC(hWnd, hdc);
						GameStatus = (enum tag_GameStatus)(CopyBoard && Q && hFlagBitmap);
					}
					break;

				case 3:
					if(IsEmpty(Q) || cnt == MAX_OPEN){
						KillTimer(hWnd, 3);
						if(bLock){LockWindowUpdate(NULL); bLock = FALSE;} 
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
							SetWindowPos(hGameOverChild, NULL, rt.left, (rt.bottom - rt.top) / 5, (rt.right - rt.left), (rt.bottom - rt.top) / 5 * 2, SWP_NOZORDER);
							SetTimer(hWnd, 4, 1000, NULL);
						}
						break;
					}

					if(GameBoard[x][y] == 0 && CopyBoard[x][y] == 0){
						if(bLock == FALSE){ bLock = LockWindowUpdate(hWnd);}
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

			if(HIWORD(wParam) == RBTN_CLICKED){
				/* 우측 마우스 버튼 */
				HWND FlagWnd = hBtns[ID / Row][ID % Column];
				if(GameBoard[ID / Row][ID % Column] == 0){
					BOOL Prop = (BOOL)GetProp(FlagWnd, TEXT("Flag"));
					if(Prop){
						SetWindowText(hWnd, TEXT("TRUE"));
						RemoveProp(FlagWnd, TEXT("Flag"));
						hFlagBitmap = (HBITMAP)SendMessage(FlagWnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hButtonBitmap);
						SetWindowPos(FlagWnd, NULL, 0,0,0,0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
					}else{
						SetWindowText(hWnd, TEXT("FALSE"));
						SetProp(FlagWnd, TEXT("Flag"), (HANDLE)TRUE);
						hButtonBitmap = (HBITMAP)SendMessage(FlagWnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hFlagBitmap);
						SetWindowPos(FlagWnd, NULL, 0,0,0,0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					}

					GetWindowRect(FlagWnd, &SelectWndRect);
					ScreenToClient(hWnd, (LPPOINT)&SelectWndRect);
					ScreenToClient(hWnd, (LPPOINT)&SelectWndRect+1);
					InvalidateRect(hWnd, &SelectWndRect, TRUE);
				}
			}else{
				/* 좌측 마우스 버튼 */
				ID = LOWORD(wParam) - IDW_BUTTON;
				bClicked = TRUE;
				Enqueue(Q, ID / Row, ID % Column);
				SetTimer(hWnd, 3, 15, NULL);
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
					SetRect(&srt, 0,0, ButtonLayout.Width * Column, ButtonLayout.Height * Row);
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
			if(Q){DestroyQueue(Q);}
			if(CopyBoard){DestroyGameBoard(CopyBoard, Row);}
			if(GameBoard){DestroyGameBoard(GameBoard, Row);}
			if(hBtns){DestroyButtons(hBtns, Row);}
			if(hBitmap){DeleteObject(hBitmap);}
			if(hFlagBitmap){DeleteObject(hFlagBitmap);}
			if(OldProc){SetClassLongPtr(hTempButton, GCLP_WNDPROC, (LONG_PTR)OldProc);}
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

COLORREF ColorList[] = {
	RGB(0, 0, 0),
	RGB(0, 0, 255),
	RGB(0, 255, 0),
	RGB(255, 0, 0)
};

void DrawBoard(HWND hWnd, HDC hdc, HBITMAP hBitmap, int** GameBoard, int Row, int Column, int Width, int Height){
	if(hBitmap == NULL){return;}

	RECT rt;
	GetClientRect(hWnd, &rt);

	HDC hMemDC = CreateCompatibleDC(hdc);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);
	FillRect(hMemDC, &rt, GetSysColorBrush(COLOR_BTNFACE));

	#define GET_RED(V)		((BYTE)(((DWORD_PTR)(V)) & 0xff))
	#define GET_GREEN(V)	((BYTE)(((DWORD_PTR)(((WORD)(V))) >> 8) & 0xff))
	#define GET_BLUE(V)		((BYTE)(((DWORD_PTR)(V >> 16)) & 0xff))
	#define SET_RGB(R,G,B)	((COLORREF)(((BYTE)(R) | ((WORD)((BYTE)(G)) << 8)) | (((DWORD)(BYTE)(B)) << 16)))

	DWORD TextBackgroundColor = GetSysColor(COLOR_BTNFACE);
	COLORREF BtnColor = SET_RGB(GET_RED(TextBackgroundColor), GET_GREEN(TextBackgroundColor), GET_BLUE(TextBackgroundColor));
	COLORREF OldColor = SetBkColor(hMemDC, BtnColor);

	int Number;
	TCHAR buf[256];
	for(int i=0; i<Row; i++){
		for(int j=0; j<Column; j++){
			Number = GameBoard[i][j];
			if(GameBoard[i][j] == -1){
				wsprintf(buf, TEXT("B"));
			}else if(GameBoard[i][j] == 0){
				wsprintf(buf, TEXT(" "), Number);
			}else{
				wsprintf(buf, TEXT("%d"), Number);
			}

			SetTextColor(hMemDC, ColorList[(Number == -1) ? 0 : Number]);
			TextOut(hMemDC, Width * j, Height * i, buf, lstrlen(buf));
		}
	}

	SetBkColor(hMemDC, OldColor);
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

