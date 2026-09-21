#pragma once
#include "UObject.h"
#include "UActorComponent.h"
#include "USceneComponent.h"

class UWorld;

class AActor : public UObject
{
	DECLARE_UCLASS(AActor, UObject)
public:
	AActor();
	virtual ~AActor();

	virtual void BeginPlay();
	virtual void Tick(float deltaTime);
	virtual void EndPlay();

	template<typename T>
	shared_ptr<T> AddComponent()
	{
		static_assert(is_base_of_v<UActorComponent, T>, "T must derive from UActorComponent");

		shared_ptr<T> component = make_shared<T>();
		component->SetOwner(static_pointer_cast<AActor>(shared_from_this()));
		_components.push_back(component);

		if (!_rootComponent)
		{
			shared_ptr<USceneComponent> sceneComponent = dynamic_pointer_cast<USceneComponent>(component);
			if (sceneComponent)
				_rootComponent = sceneComponent;
		}

		component->OnRegister();
		return component;
	}

	template<typename T>
	shared_ptr<T> GetComponentByClass() const
	{
		for (const shared_ptr<UActorComponent>& component : _components)
		{
			shared_ptr<T> casted = dynamic_pointer_cast<T>(component);
			if (casted)
				return casted;
		}
		return nullptr;
	}

	shared_ptr<USceneComponent> GetRootComponent() const { return _rootComponent; }
	void SetRootComponent(const shared_ptr<USceneComponent>& root) { _rootComponent = root; }

	const vector<shared_ptr<UActorComponent>>& GetComponents() const { return _components; }

	shared_ptr<UWorld> GetWorld() const { return _world.lock(); }
	void SetWorld(const shared_ptr<UWorld>& world) { _world = world; }

	bool IsActive() const { return _active; }
	void SetActive(bool active) { _active = active; }

private:
	shared_ptr<USceneComponent> _rootComponent;
	vector<shared_ptr<UActorComponent>> _components;
	weak_ptr<UWorld> _world;
	bool _active = true;
};
