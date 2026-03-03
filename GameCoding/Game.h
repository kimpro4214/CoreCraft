#pragma once
class Game
{

public:
	Game();
	~Game();

public:
	void Init(HWND hwnd);
	void Update();
	void Render();

private:
	HWND _hwnd;
	uint32 _width = 0;
	uint32 _height = 0;
};

