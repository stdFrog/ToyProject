#include "..\\Resource\\Maze.h"
#include "..\\Resource\\Tile.h"

static int size;
int Tile::Row = 0;
int Tile::Column = 0;
const int Tile::_width = 10;
const int Tile::_height = 10;

Tile::Tile(TileType type) : _type(type), _color(RGB(0,0,0))
{
	if(size == 0)
	{
		size = Maze::GetSize();
	}

	SetRect(&_square, Align + Column * _width, Align + Row * _height, Align + Column * _width + _width, Align + Row * _height + _height);

	Column++;
	if(Column >= size/**/)
	{
		Column = 0;
		Row++;
		if(Row >= size/**/)
		{
			Row = 0;
		}
	}
}

Tile::~Tile()
{
	Column--;
	if(Column < 0 && Row > 0)
	{
		Row--;
	}
}

TileType Tile::GetType()
{
	return _type;
}

void Tile::Init()
{
}

void Tile::Update()
{
	switch(GetType())
	{
		case TileType::None:
			_color = RGB(0,0,0);
			break;
		case TileType::Wall:
			_color = RGB(0,0,0);
			break;
		case TileType::Empty:
			_color = RGB(255,255,255);
			break;
		case TileType::Start:
			_color = RGB(255,0,0);
			break;
		case TileType::Goal:
			_color = RGB(0,0,255);
			break;
		case TileType::Player:
			_color = RGB(255,0,0);
			break;
		case TileType::Marked:
			_color = RGB(0, 255, 0);
			break;
	}

}

void Tile::Render(HDC hdc)
{
	RECT srt;

	Rectangle(hdc, _square.left-1, _square.top-1, _square.right+1, _square.bottom+1);
	SetRect(&srt, _square.left+1, _square.top+1, _square.right-1, _square.bottom-1);
	
	HBRUSH hBrush = CreateSolidBrush(_color);
	FillRect(hdc, &srt, hBrush);
	DeleteObject(hBrush);
}
