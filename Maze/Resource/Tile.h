#ifndef __TILE_H_
#define __TILE_H_

#include "resource.h"

enum class TileType
{
	None,
	Wall,
	Empty,
	Start,
	Goal,
	Player,
	Marked
};

class Tile
{
	static int Row, Column;
	static const int _width, _height;

	private:
		TileType _type;
		
		RECT _square;
		COLORREF _color;

	public:
		Tile(TileType type = TileType::None);
		~Tile();

		TileType GetType();
		void SetType(TileType type){_type = type;};
		void Init();
		void Update();
		void Render(HDC hdc);
};

#endif
