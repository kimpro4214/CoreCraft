#include "pch.h"
#include "SceneManager.h"
#include "Scene.h"
#include "GameObject.h"
#include "Camera.h"
#include "Transform.h"
#include "MeshRenderer.h"

SceneManager::SceneManager(shared_ptr<Graphics> graphics)
	: _graphics(graphics)
{

}

void SceneManager::Init()
{
	if (_activeScene == nullptr)
		return;

	_activeScene->Awake();
	_activeScene->Start();
}

void SceneManager::Update()
{
	if (_activeScene == nullptr)
		return;

	_activeScene->Update();
	_activeScene->LateUpdate(); 
	
	_activeScene->FixedUpdate();	
}

void SceneManager::LoadScene(wstring sceneName)
{
	// Resource

	_activeScene = LoadTestScene();
	Init();
}

std::shared_ptr<Scene> SceneManager::LoadTestScene()
{
	shared_ptr<Scene> scene = make_shared<Scene>();

	
	// Camera
	{
		shared_ptr<GameObject> camera = make_shared<GameObject>(_graphics->GetDevice(), _graphics->GetDeviceContext());
		{
			camera->GetOrAddTransform();
			camera->AddComponent(make_shared<Camera>());
			scene->AddGameObject(camera);
		}
	}
	
	// Monster
	{
		shared_ptr<GameObject> monster = make_shared<GameObject>(_graphics->GetDevice(), _graphics->GetDeviceContext());
		{
			monster->GetOrAddTransform();
			monster->AddComponent(make_shared<MeshRenderer>(_graphics->GetDevice(), _graphics->GetDeviceContext()));
			//_monster->GetTransform()->SetScale(Vec3(100.f, 100.f, 1.f));
			// ..
			scene->AddGameObject(monster);
		}
	}

	return scene;
}
