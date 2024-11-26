#include <windows.h>
#define CLASS_NAME TEXT("Minesweeper")
#define IDW_BUTTON 100

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ButtonSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

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

	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		NULL,
		CLASS_NAME,
	};

	RegisterClass(&wc);

	HWND hWnd = CreateWindow(
				CLASS_NAME,
				CLASS_NAME,
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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

static WNDPROC OldButtonProc;
void CustomClassRegister();

LRESULT CALLBACK ButtonSubProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	/* 문제는 윈도우 NonClient 영역의 버튼도 서브클래싱 되어 제대로 동작하지 않음 */
	switch(iMessage){
		case WM_CREATE:
			SetWindowLongPtr(hWnd, 0, 0);
			SetWindowLongPtr(hWnd, 4, FALSE);
			break;

		case WM_LBUTTONDOWN:
			{
				TCHAR buf[256];
				wsprintf(buf, TEXT("%d"), GetWindowLongPtr(hWnd, 0));
				MessageBox(hWnd, buf, TEXT("Alarm"), MB_OK);
			}
			break;
	}

	return CallWindowProc(OldButtonProc, hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	static HWND** hBtns;
	static int** GameBoard;
	static int Row, Column;

	static struct tag_Button{
		int Width;
		int Height;
	}ButtonLayout;

	static BOOL bInit = FALSE;
	static RECT srt, rt;
	static int ID;

	LPMINMAXINFO pmmi;

	switch(iMessage){
		case WM_CREATE:
			srand(GetTickCount());
			ButtonLayout.Width = 15;
			ButtonLayout.Height = 15;
			Row = Column = 20;
			CustomClassRegister();

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
					if(GameBoard == NULL){ GameBoard = CreateGameBoard(Row, Column); }
					if(GameBoard){ 
						if(Settings(&GameBoard, Row, Column)){
							for(int i=0; i<Row; i++){
								for(int j=0; j<Column; j++){
									SetWindowLongPtr(hBtns[i][j], 0, GameBoard[i][j]);
								}
							}
						}
					}
					break;
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
				if(bInit){
					SetRect(&srt, 0,0, 300, 300);
					AdjustWindowRect(&srt, GetWindowLongPtr(hWnd, GWL_STYLE), FALSE);
					SetWindowPos(hWnd, NULL, srt.left, srt.top, srt.right - srt.left, srt.bottom - srt.top, SWP_NOZORDER | SWP_NOMOVE);
				}
			}
			return 0;

		case WM_DESTROY:
			DestroyGameBoard(GameBoard, Row);
			DestroyButtons(hBtns, Row);
			PostQuitMessage(0);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void CustomClassRegister(){
	WNDCLASS btnwc;

	GetClassInfo(NULL, TEXT("button"), &btnwc);
	// btnwc.style &= ~CS_DBLCLKS;
	btnwc.cbWndExtra = 8;
	btnwc.hInstance = GetModuleHandle(NULL);
	btnwc.lpszClassName = TEXT("CustomButton");
	btnwc.hCursor = LoadCursor(NULL, IDC_ARROW);
	OldButtonProc = btnwc.lpfnWndProc;
	btnwc.lpfnWndProc = ButtonSubProc;
	RegisterClass(&btnwc);
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
						TEXT("CustomButton"),
						NULL,
						WS_BORDER | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
						j * Width, i * Height, Width, Height,
						hParent,
						(HMENU)IDW_BUTTON + i * Column + j,
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
