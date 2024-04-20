#include "..\\Resource\\Maze.h"
#include "..\\Resource\\Tile.h"

int Maze::_size = 0;
const int Maze::Minimum = 25;
const int Maze::Maximum = 50;

Maze::Maze(POINT Player, int size) 
	: _Player(Player), input(size), MazeMap(nullptr), CopyMap(nullptr),_hBit(nullptr)
{
	if(size < Minimum)
	{
		input = Maximum + Minimum >> 1;
	}

	if(size > Maximum)
	{
		input = Maximum;
	}

	Maze::_size = input;
}

Maze::~Maze()
{
	if(_hBit){ DeleteObject(_hBit); }

	if(_hMemDC) {DeleteDC(_hMemDC);}

	if(_hdc){ReleaseDC(_hWnd, _hdc);}

	if(MazeMap)
	{
		for(int i=0; i<_size; i++)
		{
			delete MazeMap[i];
		}
		delete [] MazeMap;
	}

	if(CopyMap)
	{
		for(int i=0; i<_size; i++)
		{
			delete CopyMap[i];
		}
		delete [] CopyMap;
	}
}


void Maze::Init(HWND hWnd)
{
	_hWnd = hWnd;

	if(MazeMap == nullptr)
	{
		MazeMap = new Tile* [_size];

		for(int i=0; i<_size; i++)
		{
			MazeMap[i] = new Tile [_size];	
		}
	}

	if(CopyMap == nullptr)
	{
		CopyMap = new TileType* [_size];

		for(int i=0; i<_size; i++)
		{
			CopyMap[i] = new TileType [_size];	
			memset(CopyMap[i], static_cast<int>(TileType::None), sizeof(int) * _size);
		}
	}

	if(_hBit == nullptr)
	{
		_hdc = GetDC(_hWnd);
		GetClientRect(_hWnd, &_crt);
		_hBit = CreateCompatibleBitmap(
					_hdc,
					_crt.right - _crt.left,
					_crt.bottom - _crt.top
				);
	}

	if(MazeMap)
	{
		for(int i=0; i<_size; i++)
		{
			for(int j=0; j<_size; j++)
			{
				if((i % 2 == 0) || (j % 2 == 0))
				{
					MazeMap[i][j].SetType(TileType::Wall);
				}else{
					MazeMap[i][j].SetType(TileType::Empty);
				}
			}
		}

		for(int i=0; i<_size; i++)
		{
			for(int j=0; j<_size; j++)
			{
				if((i%2 == 0) || (j%2 == 0))
				{
					continue;
				}

				if((i == _size - 2) && (j == _size - 2))
				{
					MazeMap[i][j].SetType(TileType::Goal);
					continue;
				}

				if(i == _size - 2)
				{
					MazeMap[i][j+1].SetType(TileType::Empty);
					continue;
				}
				
				if(j == _size - 2)
				{
					MazeMap[i+1][j].SetType(TileType::Empty);
					continue;
				}

				const int randValue = ::rand() % 2;
				if(randValue == 0)
				{
					MazeMap[i][j+1].SetType(TileType::Empty);
				}else{
					MazeMap[i+1][j].SetType(TileType::Empty);
				}
			}
		}
	}

	if(CopyMap)
	{
		for(int i=0; i<_size; i++)
		{
			for(int j=0; j<_size; j++)
			{
				CopyMap[i][j] = MazeMap[i][j].GetType();
			}
		}
	}

	if(MazeMap)
	{
		MazeMap[GetPosition().y][GetPosition().x].SetType(TileType::Player);
		MazeMap[GetPosition().y][GetPosition().x].Init();
	}
}

void Maze::Update()
{
	GetInput();
	MazeMap[GetPosition().y][GetPosition().x].SetType(TileType::Player);

	for(int i=0; i<_size; i++)
	{
		for(int j=0; j<_size; j++)
		{
			MazeMap[i][j].Update();
		}
	}
}

void Maze::Render()
{
	if(_hBit != nullptr)
	{
		_hMemDC = CreateCompatibleDC(_hdc);
		HGDIOBJ hOld = SelectObject(_hMemDC, _hBit);
		FillRect(_hMemDC, &_crt, GetSysColorBrush(COLOR_WINDOW));

		SetBkMode(_hMemDC, TRANSPARENT);

		TCHAR lpszText[128];
		wsprintf(lpszText, TEXT("Q = Solve, R = Reset"));
		TextOut(_hMemDC, Align , _crt.bottom - _crt.top - Align, lpszText, lstrlen(lpszText));

		for(int i=0; i<_size; i++)
		{
			for(int j=0; j<_size; j++)
			{
				MazeMap[i][j].Render(_hMemDC);
			}
		}

		SelectObject(_hMemDC, hOld);
		DeleteDC(_hMemDC);
	}

	InvalidateRect(_hWnd, NULL, FALSE);
}

TileType Maze::GetTileType(int y, int x)
{
	if(MazeMap)
	{
		return MazeMap[y][x].GetType();
	}
}

TileType Maze::GetPrevType(int y, int x)
{
	if(CopyMap)
	{
		return CopyMap[y][x];
	}
}

bool Maze::MoveTo(int y, int x)
{
	if((y >= 0 && y < _size) && (x >= 0 && x < _size))
	{
		if(MazeMap[y][x].GetType() == TileType::Empty 
				|| MazeMap[y][x].GetType() == TileType::Goal)
		{
			return true;
		}
	}
	return false;
}

void Maze::GetInput()
{
	int XAxis = 0, YAxis = 0;
	BYTE keyboard[256] = {};
	if(GetKeyboardState(keyboard))
	{
		for(int i=0; i<256; i++)
		{
			if(keyboard[i] & 0x80)
			{
				switch(i)
				{
					case VK_LEFT:
						XAxis = -1;
						break;

					case VK_RIGHT:
						XAxis = 1;
						break;

					case VK_UP:
						YAxis = -1;
						break;

					case VK_DOWN:
						YAxis = 1;
						break;

					case 'R':
						Reset();
						break;

					case 'Q':
						MazeMap[GetPosition().y][GetPosition().x].SetType(TileType::Start);
						Solve(GetPosition());
						break;

					case VK_ESCAPE:
						if(MessageBox(_hWnd, TEXT("You want to exit the Program?"), TEXT("Warning"), MB_YESNO) == IDYES)
						{
							DestroyWindow(_hWnd);
						}
						break;

					default:
						XAxis = YAxis = 0;
						break;
				}

				POINT Prev = GetPosition();
				POINT New = {Prev.x + XAxis, Prev.y + YAxis};

				if(MoveTo(New.y, New.x))
				{
					SetPosition(New.y, New.x);
					// MazeMap[New.y][New.x].SetType(TileType::Player);
					MazeMap[Prev.y][Prev.x].SetType(GetPrevType(Prev.y, Prev.x));
				}
			}
		}
	}
}

void Maze::Reset()
{
	for(int i=0; i<_size; i++)
	{
		for(int j=0; j<_size; j++)
		{
			MazeMap[i][j].SetType(GetPrevType(i,j));
		}
	}
}

int Maze::Solve(POINT Current)
{
	if(MazeMap[Current.y][Current.x].GetType() == TileType::Goal)
	{
		return SUCCEED;
	}

	MazeMap[Current.y][Current.x].SetType(TileType::Marked);

	int dx[] = {0, 1, 0, -1};
	int dy[] = {-1, 0, 1, 0};

	for(int i=0; i<sizeof(dx)/sizeof(dx[0]); i++)
	{
		int nx, ny;
		nx = Current.x + dx[i];
		ny = Current.y + dy[i];

		if(!MoveTo(ny, nx))
		{
			continue;
		}

		POINT Next = {nx, ny};
		if(Solve(Next) == SUCCEED)
		{
			return SUCCEED;
		}
	}

	MazeMap[Current.y][Current.x].SetType(GetPrevType(Current.y, Current.x));

	return FAIL;
}
