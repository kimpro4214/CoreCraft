#include "pch.h"
#include "Game.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "InputManager.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "RenderManager.h"

unique_ptr<Game> GGame = make_unique<Game>();

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

	_input = make_shared<InputManager>();
	_input->Init(hwnd);
	_time = make_shared<TimeManager>();
	_time->Init();
	_scene = make_shared<SceneManager>(_graphics);
	_scene->Init();
	_resource = make_shared<ResourceManager>(_graphics->GetDevice());
	_resource->Init();
	_render = make_shared<RenderManager>(_graphics->GetDevice(), _graphics->GetDeviceContext());
	_render->Init();

	SCENE->LoadScene(L"Test");
}

void Game::Update()
{
	TIME->Update();
	INPUT->Update();
	SCENE->Update();
}

void Game::Render()
{
	RENDER->Update(_graphics);
}