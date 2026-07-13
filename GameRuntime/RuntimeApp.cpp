#include "pch.h"
#include "RuntimeApp.h"
#include "Scene.h"
#include "SceneSerializer.h"
#include "GameObject.h"
#include "Camera.h"
#include "Transform.h"

RuntimeApp::RuntimeApp(filesystem::path scenePath)
	: _scenePath(move(scenePath))
{
}

void RuntimeApp::Init()
{
	RESOURCES->Init();
	_shader = make_shared<Shader>(L"20. CoreCraft.fx");
	RENDER->Init(_shader);

	string error;
	_scene = SceneSerializer::Load(_scenePath, _shader, &error);
	if (_scene == nullptr)
	{
		MessageBoxA(nullptr, error.c_str(), "Core Craft Runtime", MB_OK | MB_ICONERROR);
		PostQuitMessage(1);
		return;
	}
	SCENE->SetActiveScene(_scene);

	_fallbackCamera = make_shared<GameObject>();
	_fallbackCamera->GetOrAddTransform()->SetPosition({ 0.f, 3.f, -8.f });
	_fallbackCamera->GetTransform()->SetRotation({ 0.2f, 0.f, 0.f });
	_fallbackCamera->AddComponent(make_shared<Camera>());
}

void RuntimeApp::Update()
{
	if (_scene == nullptr)
		return;

	_scene->Update();
	_scene->LateUpdate();

	shared_ptr<Camera> sceneCamera;
	for (const shared_ptr<GameObject>& object : _scene->GetGameObjects())
	{
		if (object->GetCamera())
		{
			sceneCamera = object->GetCamera();
			break;
		}
	}
	if (sceneCamera) sceneCamera->Update(); else _fallbackCamera->Update();
	RENDER->Update();

	LightDesc light;
	_scene->BuildLightData(light);
	RENDER->PushLightData(light);

}

void RuntimeApp::Render()
{
	if (_scene)
		_scene->Render();
}

void RuntimeApp::Shutdown()
{
	SCENE->SetActiveScene(nullptr);
	_scene.reset();
}
