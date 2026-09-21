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
		RegisterComponent(component);
		return component;
	}

	shared_ptr<UActorComponent> AddComponent(const shared_ptr<UActorComponent>& component)
	{
		RegisterComponent(component);
		return component;
	}

	bool RemoveComponent(const shared_ptr<UActorComponent>& component);

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

	bool AttachToActor(const shared_ptr<AActor>& parent);
	void DetachFromActor();
	shared_ptr<AActor> GetParentActor() const;
	vector<shared_ptr<AActor>> GetAttachedActors() const;

	shared_ptr<UWorld> GetWorld() const { return _world.lock(); }
	void SetWorld(const shared_ptr<UWorld>& world) { _world = world; }

	bool IsActive() const { return _active; }
	void SetActive(bool active) { _active = active; }

private:
	void RegisterComponent(const shared_ptr<UActorComponent>& component);

private:
	shared_ptr<USceneComponent> _rootComponent;
	vector<shared_ptr<UActorComponent>> _components;
	weak_ptr<UWorld> _world;
	bool _active = true;
};
