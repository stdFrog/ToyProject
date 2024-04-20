#ifndef __MAZE_H_
#define __MAZE_H_

#include "resource.h"
#define Align 25
#define SUCCEED 100
#define FAIL -1

class Tile;
enum class TileType;

enum DIRECTION {NORTH, WEST, SOUTH, EAST};

class Maze
{
	static int _size;
	static const int Minimum, Maximum;

	private:
		Tile** MazeMap;
		TileType** CopyMap;
		HWND _hWnd;
		RECT _crt;
		HDC _hdc, _hMemDC;
		HBITMAP _hBit;
		POINT _Player;
		int input;

	public:
		Maze(POINT Player = {1,1}, int size = Maze::Minimum);
		~Maze();

		void Init(HWND hWnd);
		void Update();
		void Render();

		static int GetSize() {return _size;}
		HBITMAP GetBitmap(){return _hBit;}

		POINT GetPosition() {return _Player;}
		void SetPosition(int y, int x){_Player.x = x; _Player.y = y;}

		TileType GetTileType(int y, int x);
		TileType GetPrevType(int y, int x);

		void GetInput();

		bool MoveTo(int y, int x);
		int Solve(POINT Current);

		void Reset();
};

#endif
