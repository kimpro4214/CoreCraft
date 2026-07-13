#include "pch.h"
#include "Scene.h"
#include "Transform.h"
#include "Light.h"
#include "RenderManager.h"

Scene::Scene(string name)
	: _name(move(name))
{
}

shared_ptr<GameObject> Scene::CreateGameObject(const string& name)
{
	shared_ptr<GameObject> gameObject = make_shared<GameObject>();
	gameObject->SetName(name);
	gameObject->GetOrAddTransform();
	AddGameObject(gameObject);
	return gameObject;
}

void Scene::AddGameObject(const shared_ptr<GameObject>& gameObject)
{
	if (gameObject == nullptr || FindGameObject(gameObject->GetId()))
		return;

	gameObject->SetRenderDuringUpdate(false);
	_gameObjects.push_back(gameObject);
	_dirty = true;
}

bool Scene::DestroyGameObject(ObjectId id)
{
	auto found = find_if(_gameObjects.begin(), _gameObjects.end(), [id](const shared_ptr<GameObject>& object)
	{
		return object->GetId() == id;
	});
	if (found == _gameObjects.end())
		return false;

	shared_ptr<Transform> transform = (*found)->GetTransform();
	if (transform)
	{
		vector<shared_ptr<Transform>> children = transform->GetChildren();
		for (const shared_ptr<Transform>& child : children)
			child->SetParent(nullptr);
		transform->SetParent(nullptr);
	}

	_gameObjects.erase(found);
	_dirty = true;
	return true;
}

void Scene::Clear()
{
	for (const shared_ptr<GameObject>& object : _gameObjects)
	{
		if (object->GetTransform())
			object->GetTransform()->SetParent(nullptr);
	}
	_gameObjects.clear();
	_awakened = false;
	_started = false;
	_dirty = false;
}

shared_ptr<GameObject> Scene::FindGameObject(ObjectId id) const
{
	auto found = find_if(_gameObjects.begin(), _gameObjects.end(), [id](const shared_ptr<GameObject>& object)
	{
		return object->GetId() == id;
	});
	return found == _gameObjects.end() ? nullptr : *found;
}

vector<shared_ptr<GameObject>> Scene::GetRootObjects() const
{
	vector<shared_ptr<GameObject>> roots;
	for (const shared_ptr<GameObject>& object : _gameObjects)
	{
		if (object->GetTransform() == nullptr || !object->GetTransform()->HasParent())
			roots.push_back(object);
	}
	return roots;
}

void Scene::Awake()
{
	if (_awakened)
		return;
	for (const shared_ptr<GameObject>& object : _gameObjects)
		object->Awake();
	_awakened = true;
}

void Scene::Start()
{
	if (_started)
		return;
	Awake();
	for (const shared_ptr<GameObject>& object : _gameObjects)
		object->Start();
	_started = true;
}

void Scene::Update()
{
	Start();
	for (const shared_ptr<GameObject>& object : _gameObjects)
		object->Update();
}

void Scene::LateUpdate()
{
	for (const shared_ptr<GameObject>& object : _gameObjects)
		object->LateUpdate();
}

void Scene::Render()
{
	for (const shared_ptr<GameObject>& object : _gameObjects)
		object->Render();
}

void Scene::BuildLightData(LightDesc& data) const
{
	data = LightDesc{};
	data.ambient = Color(0.28f, 0.3f, 0.34f, 1.f);
	data.diffuse = Color(0.8f, 0.82f, 0.85f, 1.f);
	data.specular = Color(0.65f, 0.65f, 0.65f, 1.f);
	data.direction = Vec3(0.3f, -1.f, 0.25f);
	bool hasDirectional = false;

	for (const shared_ptr<GameObject>& object : _gameObjects)
	{
		if (!object->IsActive())
			continue;
		shared_ptr<Light> light = object->GetLight();
		shared_ptr<Transform> transform = object->GetTransform();
		if (light == nullptr || transform == nullptr)
			continue;

		if (light->GetLightType() == LightType::Directional)
		{
			if (hasDirectional)
				continue;
			data.direction = transform->GetLook();
			data.direction.Normalize();
			const Color& color = light->GetColor();
			float intensity = light->GetIntensity();
			data.diffuse = Color(color.x * intensity, color.y * intensity, color.z * intensity, color.w);
			data.specular = data.diffuse;
			hasDirectional = true;
			continue;
		}

		if (data.pointCount >= data.points.size())
			continue;
		LightDesc::Point& point = data.points[data.pointCount++];
		point.color = light->GetColor();
		point.position = transform->GetPosition();
		point.range = light->GetRange();
		point.intensity = light->GetIntensity();
	}
}

shared_ptr<Scene> SceneManager::CreateScene(const string& name)
{
	_activeScene = make_shared<Scene>(name);
	return _activeScene;
}
