#include "pch.h"
#include "AActor.h"

AActor::AActor()
{
}

AActor::~AActor()
{
}

void AActor::BeginPlay()
{
	for (const shared_ptr<UActorComponent>& component : _components)
		component->BeginPlay();
}

void AActor::Tick(float deltaTime)
{
	if (!_active)
		return;

	for (const shared_ptr<UActorComponent>& component : _components)
	{
		if (component->IsActive() && component->CanEverTick())
			component->TickComponent(deltaTime);
	}
}

void AActor::EndPlay()
{
	for (const shared_ptr<UActorComponent>& component : _components)
		component->EndPlay();
}

void AActor::RegisterComponent(const shared_ptr<UActorComponent>& component)
{
	if (!component)
		return;

	component->SetOwner(static_pointer_cast<AActor>(shared_from_this()));
	_components.push_back(component);

	if (!_rootComponent)
	{
		shared_ptr<USceneComponent> sceneComponent = dynamic_pointer_cast<USceneComponent>(component);
		if (sceneComponent)
			_rootComponent = sceneComponent;
	}

	component->OnRegister();
}

bool AActor::RemoveComponent(const shared_ptr<UActorComponent>& component)
{
	if (!component)
		return false;

	auto found = find(_components.begin(), _components.end(), component);
	if (found == _components.end())
		return false;

	if (shared_ptr<USceneComponent> asScene = dynamic_pointer_cast<USceneComponent>(component))
	{
		for (const shared_ptr<USceneComponent>& child : vector<shared_ptr<USceneComponent>>(asScene->GetAttachChildren()))
			child->DetachFromParent();

		asScene->DetachFromParent();

		if (_rootComponent == asScene)
			_rootComponent = nullptr;
	}

	component->EndPlay();
	component->SetOwner(nullptr);
	_components.erase(found);
	return true;
}

bool AActor::AttachToActor(const shared_ptr<AActor>& parent)
{
	if (!parent || !_rootComponent)
		return false;

	shared_ptr<USceneComponent> parentRoot = parent->GetRootComponent();
	if (!parentRoot)
		return false;

	return _rootComponent->AttachToComponent(parentRoot);
}

void AActor::DetachFromActor()
{
	if (_rootComponent)
		_rootComponent->DetachFromParent();
}

shared_ptr<AActor> AActor::GetParentActor() const
{
	if (!_rootComponent)
		return nullptr;

	shared_ptr<USceneComponent> parentComponent = _rootComponent->GetAttachParent();
	return parentComponent ? parentComponent->GetOwner() : nullptr;
}

vector<shared_ptr<AActor>> AActor::GetAttachedActors() const
{
	vector<shared_ptr<AActor>> result;
	if (!_rootComponent)
		return result;

	for (const shared_ptr<USceneComponent>& child : _rootComponent->GetAttachChildren())
	{
		shared_ptr<AActor> owner = child->GetOwner();
		if (owner && owner->GetRootComponent() == child)
			result.push_back(owner);
	}
	return result;
}
