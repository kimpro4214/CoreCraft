#include "pch.h"
#include "Game.h"

Game::Game()
{

}

Game::~Game()
{

}

void Game::Init(HWND hwnd)
{
	_hwnd = hwnd;

	_graphics = make_shared<Graphics>(hwnd);
	_pipeline = make_shared<Pipeline>(_graphics->GetDeviceContext());
	_gameObject = make_shared<GameObject>(_graphics->GetDevice(), _graphics->GetDeviceContext());
}

void Game::Update()
{
	_gameObject->Update();
}

void Game::Render()
{
	_graphics->RenderBegin();

	{
		_gameObject->Render(_pipeline);
	}

	_graphics->RenderEnd();
}