#define UNICODE
#define DEBUG
#include "..\\Headers\\resource.h"
#include "..\\Headers\\Color.h"
#include "..\\Headers\\Button.h"
#include "..\\Headers\\Queue.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32")

#define TILE_SIZE 16

BOOL bLeak = FALSE;
int StretchMode;
OSVERSIONINFO osv;

HWND hStatusWnd;
RECT g_crt, g_wrt, g_trt;
HBITMAP hClientBitmap = NULL;
HMENU hPopupSettings, hPopupSize;

typedef struct tag_MapSize{
	INT x,y;
}MapSize;

MapSize Table[3] = {
	{25, 25},
	{30, 20},
	{40, 30}
};

typedef enum { GAME_BEGIN, GAME_PAUSE, GAME_END } GAME_STATE;
UINT_PTR Index;
UINT_PTR g_Time;

int MINECNT;
GAME_STATE g_GameState;
HBITMAP g_hBitmapData[DATA_LAST_COUNT];
HBITMAP g_hBitmapState[STATE_LAST_COUNT];

Button** Btns = NULL;

/* TODO : Create a Queue Structure */
Queue* Q = NULL;
Queue* CQ = NULL;

LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam){
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osv);

	if(osv.dwPlatformId >= VER_PLATFORM_WIN32_NT)
	{
		StretchMode = HALFTONE;
	}else{
		StretchMode = COLORONCOLOR;
	}

	HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
	AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hSysMenu, MF_STRING, ID_SYS_ABOUT, TEXT("프로그램 소개"));

	hPopupSettings = CreatePopupMenu();
	AppendMenu(hSysMenu, MF_STRING | MF_POPUP, (UINT_PTR)hPopupSettings, TEXT("설정"));
	AppendMenu(hPopupSettings, MF_STRING, ID_SYS_NEWGAME, TEXT("새 게임"));

	hPopupSize = CreatePopupMenu();
	AppendMenu(hPopupSettings, MF_STRING | MF_POPUP, (UINT_PTR)hPopupSize, TEXT("크기 변경"));
	AppendMenu(hPopupSize, MF_STRING, ID_SYS_RESIZE1, TEXT("25 X 25"));
	AppendMenu(hPopupSize, MF_STRING, ID_SYS_RESIZE2, TEXT("30 X 20"));
	AppendMenu(hPopupSize, MF_STRING, ID_SYS_RESIZE3, TEXT("40 X 30"));
	CheckMenuRadioItem(hPopupSize, ID_SYS_RESIZE1, ID_SYS_RESIZE2, ID_SYS_RESIZE1, MF_BYCOMMAND);

	for(INT_PTR i=0; i<DATA_LAST_COUNT; i++){
		g_hBitmapData[i] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_EMPTY + i));
		/*
		TCHAR buf[1024];
		wsprintf(buf, TEXT("LoadBitmap Failed: IDB_NORMAL = %d, i = %d "), IDB_NORMAL, i);
		if(g_hBitmapData[i]==NULL){MessageBox(hWnd, buf, TEXT(""), MB_OK);}
		*/
	}

	for(INT_PTR j=0; j<STATE_LAST_COUNT; j++){
		g_hBitmapState[j] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_NORMAL + j));
		/*
		if(g_hBitmapState[j]==NULL){MessageBox(hWnd, TEXT("State Image Is NULL"), TEXT(""), MB_OK);}
		*/
	}

	// InitCommonControls();
	// hStatusWnd = CreateStatusWindow(WS_VISIBLE | WS_CHILD, TEXT(""), hWnd, IDW_STATUS);
	// hStatusWnd = CreateWindowEx(0, STATUSCLASSNAME, NULL, /*SBARS_SIZEGRIP | SBARS_TOOLTIPS */ WS_CHILD | WS_VISIBLE, 0,0,0,0, hWnd, (HMENU)IDW_STATUS, GetModuleHandle(NULL), NULL);

	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	SetWindowLong(hWnd, GWL_STYLE, dwStyle & ~WS_THICKFRAME);
	SendMessage(hWnd, WM_NCPAINT, 1, 0);

	Index = 0;
	MINECNT = Table[Index].x;
	g_GameState = GAME_PAUSE;

	SetClientRect(hWnd, TILE_SIZE * Table[Index].x, TILE_SIZE * Table[Index].y + (g_trt.bottom - g_trt.top));
	GetClientRect(hWnd, &g_crt);

	Btns = CreateButtons(Table[Index].x, Table[Index].y);
	InitButtons(hWnd, Btns, Table[Index].x, Table[Index].y);

	if(Q == NULL){ Q = CreateQueue(); }
	RandomizeSet();

	return 0;
}

LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam){
	if(hClientBitmap){DeleteObject(hClientBitmap);}
	if(g_hBitmapData){
		for(int i=0; i<DATA_LAST_COUNT; i++){
			if(g_hBitmapData[i]){
				DeleteObject(g_hBitmapData[i]);
			}
		}
	}
	if(g_hBitmapState){
		for(int i=0; i<STATE_LAST_COUNT; i++){
			if(g_hBitmapState[i]){
				DeleteObject(g_hBitmapState[i]);
			}
		}
	}
	if(Btns){DestroyButtons(Btns, Table[Index].x, Table[Index].y);}
	if(Q){DestroyQueue(Q);}
	if(CQ){DestroyQueue(CQ);}

	KillTimer(hWnd, 0);
	KillTimer(hWnd, 1);
	KillTimer(hWnd, 1234);
	PostQuitMessage(0);
	return 0;
}

UINT g_ix, g_iy;
LRESULT OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){
	GetIndex(lParam, &g_ix, &g_iy);
	Btns[g_iy][g_ix].OnLPressed();
	return 0;
}

LRESULT OnRButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam){
	GetIndex(lParam, &g_ix, &g_iy);
	Btns[g_iy][g_ix].OnRPressed();
	return 0;
}

LRESULT OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam){
	Btns[g_iy][g_ix].OnReleased();

	if(Btns[g_iy][g_ix].GetState() == PRESS){
		if(CQ == NULL){
			CQ = CreateQueue();
		}

		Enqueue(CQ, CreateNode(g_ix, g_iy));
		ExploreAround();
	}
	return 0;
}

LRESULT OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam){
	/* 
	   핫 상태를 굳이 표현할 필요가 있는가?
	*/
	return 0;
}

LRESULT OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam){
	UINT_PTR ID;
	static WPARAM PrevCommand = 0;

	switch(LOWORD(wParam)){
		case ID_SYS_ABOUT:
			MessageBox(hWnd, TEXT("이 프로그램은 고전 게임 '지뢰찾기'를 모작한 것이며 마우스 조작을 통해 게임을 진행하실 수 있습니다.\r\n\r\n시스템적 변화는 일절 없으며 메뉴에서 게임 옵션을 조정할 수 있습니다.\r\n\r\n"), TEXT("프로그램 소개"), MB_OK);
			return 0;

		case ID_SYS_NEWGAME:
			return 0;

		case ID_SYS_RESIZE1:
		case ID_SYS_RESIZE2:
		case ID_SYS_RESIZE3:
			if(LOWORD(PrevCommand) != LOWORD(wParam)){
				PrevCommand = wParam;
				CheckMenuRadioItem(hPopupSize, ID_SYS_RESIZE1, ID_SYS_RESIZE3, LOWORD(wParam), MF_BYCOMMAND);
				// TODO : Replace Map
			}
			return 0;

		default:
			return DefWindowProc(hWnd, WM_SYSCOMMAND, wParam, lParam);
	}
}

LRESULT OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam){
	MSG msg = {0};
	INT Part[3];

	if(wParam != SIZE_MINIMIZED) {
		if(hClientBitmap != NULL){
			DeleteObject(hClientBitmap);
			hClientBitmap = NULL;
		}

		/*
		SendMessage(hStatusWnd, WM_SIZE, wParam, lParam);
		for(int i=0; i<3; i++){
			Part[i] = LOWORD(lParam) / 3 * (i + 1);
		}
		SendMessage(hStatusWnd, SB_SETPARTS, 3, (LPARAM)Part);
		SendMessage(hStatusWnd, SB_GETRECT, 0, (LPARAM)&g_trt);
		*/
	}
	return 0;
}

LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	OnDrawButtons(hdc, Btns, Table[Index].x, Table[Index].y);
	static BOOL bFirst = TRUE;
	if(bFirst){IncreaseDataSet(); bFirst = FALSE;}
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam){
	RECT srt;

	switch(wParam){
		case 0:
			{
				/* 상태값 폴링 - 핫 상태 사용시에만 적용 */
				KillTimer(hWnd, 0);
			}
			break;
		case 1:
			{
				if(g_GameState == GAME_BEGIN){
					g_Time++;
				}else{
					g_Time = 0;
					KillTimer(hWnd, 1);
				}
			}
			break;
	}

	/*
	SendMessage(hStatusWnd, SB_GETRECT, 2, (LPARAM)&g_trt);
	InvalidateRect(hWnd, &g_trt, TRUE);
	*/
	return 0;
}


























/* Utility */
void DrawBitmap(HDC hDC, LONG X, LONG Y, HBITMAP hBitmap){
	if(hBitmap == NULL){return;}

	HDC hMemDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	BitBlt(hDC, X,Y, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
}

void SetClientRect(HWND hWnd, int Width, int Height){
	RECT crt;
	DWORD Style, ExStyle;

	SetRect(&crt, 0,0, Width, Height);
	Style = GetWindowLong(hWnd, GWL_STYLE);
	ExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);

	AdjustWindowRectEx(&crt, Style, GetMenu(hWnd) != NULL, ExStyle);
	if(Style & WS_VSCROLL){crt.right += GetSystemMetrics(SM_CXVSCROLL);}
	if(Style & WS_HSCROLL){crt.bottom += GetSystemMetrics(SM_CYHSCROLL);}

	SetWindowPos(hWnd, NULL, 0,0, crt.right - crt.left, crt.bottom - crt.top, SWP_NOZORDER);
}

void SetStatusText(HWND hWnd){
	TCHAR Info[MAX_PATH];

	wsprintf(Info, TEXT("Hidden Mines : %d"), ((g_GameState != GAME_BEGIN) ? 0 : MINECNT));
	SendMessage(hStatusWnd, SB_SETTEXT, 0, (LPARAM)Info);
	wsprintf(Info, TEXT("Map Size : %dx%d"), Table[Index].x, Table[Index].y);
	SendMessage(hStatusWnd, SB_SETTEXT, 1, (LPARAM)Info);
	wsprintf(Info, TEXT("경과 시간 : %d(s)"), g_Time);
	SendMessage(hStatusWnd, SB_SETTEXT, 2, (LPARAM)Info);
}

Button** CreateButtons(int W, int H){
	Button **Btns = new Button*[H];
	for(int i=0; i<H; i++){
		Btns[i] = new Button[W];
	}

	return Btns;
}

void RandomizeSet(){
	MSG msg = {0};
	/* TODO : 현재 맵 사이즈 확인 */
	int Row = Table[Index].x;
	int Col = Table[Index].y;

	while(MINECNT > 0){
		while(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE)){
			if(msg.message == WM_QUIT){
				PostQuitMessage(0);
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		int r = rand() % Row;
		int c = rand() % Col;

		if(Btns[c][r].GetData() == MINE){
			MINECNT++;
		}else{
			Btns[c][r].SetData(MINE);

			MINECNT--;
			Enqueue(Q, CreateNode(r,c));
		}
	}
}

void InitButtons(HWND hWnd, Button** Btns, int W, int H){
	MSG msg = {0};

	for(int i=0; i<H; i++){
		for(int j=0; j<W; j++){
			while(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE)){
				if(msg.message == WM_QUIT){
					PostQuitMessage(0);
				}

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			Btns[i][j].SetParent(hWnd);
			Btns[i][j].SetX(j * TILE_SIZE);
			Btns[i][j].SetY(i * TILE_SIZE);
			Btns[i][j].SetBitmap(g_hBitmapData, g_hBitmapState);
		}
	}
}

void DestroyButtons(Button** Target, int W, int H){
	for(int i=0; i<H; i++){
		delete [] Target[i];
	}
	delete [] Target;
}

/*
	버튼을 그리는 도중 발생하는 일련의 메시지에 대하여
	교착 상태가 발생할 수 있으므로 메시지 펌프를 활용한다.
*/
void OnDrawButtons(HDC hdc, Button** Btns, int W, int H){
	MSG msg = {0};

	for(int i=0; i<H; i++){
		for(int j=0; j<W; j++){
			while(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE)){
				if(msg.message == WM_QUIT){
					PostQuitMessage(0);
				}

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			Btns[i][j].OnPaint(hdc);
		}
	}
}

void GetIndex(LPARAM lParam, UINT* ix, UINT* iy){
	LONG x = (LONG)(WORD)LOWORD(lParam);
	LONG y = (LONG)(WORD)HIWORD(lParam);

	*ix = x / TILE_SIZE;
	*iy = y / TILE_SIZE;
}

/*
	기존의 지뢰찾기 게임을 볼 때 BFS보단 DFS로의 구현이 더 정확해 보인다.
	종료 시점과 메시지 루프를 수정해서 반응성을 높여보자.

void ExploreBottomUp(int x , int y){
	if(x < 0 || x >= Table[Index].x || y < 0 || y >= Table[Index].y){return;}
	if(Btns[y][x].GetState() != NORMAL){return;}
	if(Btns[y][x].GetData() != EMPTY){return;}

	Btns[y][x].ChangeState(PRESS);
	ExploreBottomUp(x+1, y);
	ExploreBottomUp(x, y+1);
	ExploreBottomUp(x-1, y);
	ExploreBottomUp(x, y-1);
}
*/

void ExploreAround(){
	static int dx[] = {0, 1, 0, -1},
			   dy[] = {-1, 0, 1, 0};

	MSG msg;
	while(!IsEmpty(CQ)){
		Node* Popped = Dequeue(CQ);

		while(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE)){
			if(msg.message == WM_QUIT){
				PostQuitMessage(0);
			}

			if(msg.message != WM_LBUTTONDOWN && msg.message != WM_RBUTTONDOWN){
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		for(int i=0; i<4; i++){
			int xx = Popped->x + dx[i];
			int yy = Popped->y + dy[i];

			if(yy < 0 || yy >= Table[Index].y || xx < 0 || xx >= Table[Index].x){ continue; }
			if(Btns[yy][xx].GetState() != NORMAL){ continue; }
			if(Btns[yy][xx].GetData() != EMPTY){ continue; }

			Btns[yy][xx].ChangeState(PRESS);
			Enqueue(CQ, CreateNode(xx,yy));
		}

		DestroyNode(Popped);
	}

	while(!IsEmpty(CQ)){
		while(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE)){
			if(msg.message == WM_QUIT){
				PostQuitMessage(0);
			}

			if(msg.message != WM_LBUTTONDOWN && msg.message != WM_RBUTTONDOWN){
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		DestroyNode(Dequeue(CQ));
	}
}

/*
	프로그램이 살해당하는 이유를 찾을 수 없다.

	디버깅을 위한 도구가 부족해서 어림 짐작할 수 밖에 없는데,
	당장 떠오르는 이유로는 배열의 범위를 넘어서는 첨자 또는 메시지 데드락이다.

	메시지 펌프도 소용없으므로 메모리를 다루는 부분에서 오류가 발생하는 것으로 보인다.

	당장 급한 일이 있기 때문에 명일 아침 수정하도록 하자.
*/
void IncreaseDataSet(){
	static int dx[] = {0, 1, 0, -1},
			   dy[] = {-1, 0, 1, 0};

	static int diagx[] = {1, 1, -1, -1},
			   diagy[] = {-1, 1, 1, -1};

	MSG msg;
	while(!IsEmpty(Q)){
		Node* Popped = Dequeue(Q);

		while(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE)){

			if(msg.message == WM_QUIT){
				PostQuitMessage(0);
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		/*
			TCHAR Debug[256];
			wsprintf(Debug, TEXT("Popped = %d, %d"), Popped->x, Popped->y);
			MessageBox(HWND_DESKTOP, Debug, TEXT(""), MB_OK);

			약 10번 정도 시도해본 결과 범위를 벗어나는 배열 첨자는 없는 것으로 보인다.

			후자의 경우 곧, 인라인 호출이 아니여서 메시지 데드락 현상이 발생하는 것이 거의 확실하다.
			이를 위해 알고리즘을 세분화 할 필요가 있다.
		*/
	
		/*
		for(int i=0; i<4; i++){
			int xx = Popped->x + dx[i];
			int yy = Popped->y + dy[i];

			if(Btns[yy][xx].GetData() == MINE){continue;}
			Btns[yy][xx].SetData((DATA)(Btns[Popped->y][Popped->x].GetData() + MINE));
		}
		*/

		DestroyNode(Popped);
	}

}
