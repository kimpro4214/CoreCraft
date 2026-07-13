#include "pch.h"
#include "GameObject.h"
#include "MonoBehaviour.h"
#include "Transform.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "ModelRenderer.h"
#include "ModelAnimator.h"
#include "Blueprint.h"
#include "Light.h"
#include "AnimatedModelRenderer.h"
#include "PlayerController.h"

namespace
{
	atomic<ObjectId> GNextObjectId = 1;
}

GameObject::GameObject()
{
	_id = GNextObjectId.fetch_add(1);
}

GameObject::~GameObject()
{

}

void GameObject::Awake()
{
	if (!_active)
		return;

	for (shared_ptr<Component>& component : _components)
	{
		if (component)
			component->Awake();
	}

	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->Awake();
	}
}

void GameObject::Start()
{
	if (!_active)
		return;

	for (shared_ptr<Component>& component : _components)
	{
		if (component)
			component->Start();
	}

	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->Start();
	}
}

void GameObject::Update()
{
	if (!_active)
		return;

	for (shared_ptr<Component>& component : _components)
	{
		if (component)
			component->Update();
	}

	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->Update();
	}

	// Scene 이전의 독립 실행 데모는 Update에서 바로 그리던 흐름을 유지한다.
	if (_renderDuringUpdate)
		Render();
}

void GameObject::LateUpdate()
{
	if (!_active)
		return;

	for (shared_ptr<Component>& component : _components)
	{
		if (component)
			component->LateUpdate();
	}

	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->LateUpdate();
	}
}

void GameObject::FixedUpdate()
{
	if (!_active)
		return;

	for (shared_ptr<Component>& component : _components)
	{
		if (component)
			component->FixedUpdate();
	}

	for (shared_ptr<MonoBehaviour>& script : _scripts)
	{
		script->FixedUpdate();
	}
}

std::shared_ptr<Component> GameObject::GetFixedComponent(ComponentType type)
{
	uint8 index = static_cast<uint8>(type);
	assert(index < FIXED_COMPONENT_COUNT);
	return _components[index];
}

std::shared_ptr<Transform> GameObject::GetTransform()
{
	shared_ptr<Component> component = GetFixedComponent(ComponentType::Transform);
	return static_pointer_cast<Transform>(component);
}

std::shared_ptr<Camera> GameObject::GetCamera()
{
	shared_ptr<Component> component = GetFixedComponent(ComponentType::Camera);
	return static_pointer_cast<Camera>(component);
}

std::shared_ptr<MeshRenderer> GameObject::GetMeshRenderer()
{
	shared_ptr<Component> component = GetFixedComponent(ComponentType::MeshRenderer);
	return static_pointer_cast<MeshRenderer>(component);
}

std::shared_ptr<ModelRenderer> GameObject::GetModelRenderer()
{
	shared_ptr<Component> component = GetFixedComponent(ComponentType::ModelRenderer);
	return static_pointer_cast<ModelRenderer>(component);
}

std::shared_ptr<ModelAnimator> GameObject::GetModelAnimator()
{
	shared_ptr<Component> component = GetFixedComponent(ComponentType::Animator);
	return static_pointer_cast<ModelAnimator>(component);
}

std::shared_ptr<Transform> GameObject::GetOrAddTransform()
{
	if (GetTransform() == nullptr)
	{
		shared_ptr<Transform> transform = make_shared<Transform>();
		AddComponent(transform);
	}

	return GetTransform();
}

void GameObject::AddComponent(shared_ptr<Component> component)
{
	if (component == nullptr)
		return;

	component->SetGameObject(shared_from_this());

	uint8 index = static_cast<uint8>(component->GetType());
	if (index < FIXED_COMPONENT_COUNT)
	{
		_components[index] = component;
	}
	else
	{
		_scripts.push_back(dynamic_pointer_cast<MonoBehaviour>(component));
	}
}

std::shared_ptr<BlueprintComponent> GameObject::GetBlueprint()
{
	shared_ptr<Component> component = GetFixedComponent(ComponentType::Blueprint);
	return static_pointer_cast<BlueprintComponent>(component);
}

std::shared_ptr<Light> GameObject::GetLight()
{
	shared_ptr<Component> component = GetFixedComponent(ComponentType::Light);
	return static_pointer_cast<Light>(component);
}

std::shared_ptr<AnimatedModelRenderer> GameObject::GetAnimatedModelRenderer()
{
	shared_ptr<Component> component = GetFixedComponent(ComponentType::AnimatedRenderer);
	return static_pointer_cast<AnimatedModelRenderer>(component);
}

std::shared_ptr<PlayerController> GameObject::GetPlayerController()
{
	for (const shared_ptr<MonoBehaviour>& script : _scripts)
	{
		shared_ptr<PlayerController> player = dynamic_pointer_cast<PlayerController>(script);
		if (player)
			return player;
	}
	return nullptr;
}

void GameObject::Render()
{
	if (!_active)
		return;

	for (shared_ptr<Component>& component : _components)
	{
		if (component)
			component->Render();
	}

	for (shared_ptr<MonoBehaviour>& script : _scripts)
		script->Render();
}

void GameObject::SetId(ObjectId id)
{
	_id = id;
	ObjectId expected = GNextObjectId.load();
	while (expected <= id && !GNextObjectId.compare_exchange_weak(expected, id + 1))
	{
	}
}
