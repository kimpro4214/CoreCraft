#pragma once
#include "UObject.h"

class AActor;

class UActorComponent : public UObject
{
	DECLARE_UCLASS(UActorComponent, UObject)
public:
	UActorComponent();
	virtual ~UActorComponent();

	virtual void OnRegister() { }
	virtual void BeginPlay() { }
	virtual void TickComponent(float deltaTime) { }
	virtual void EndPlay() { }

	shared_ptr<AActor> GetOwner() const { return _owner.lock(); }
	void SetOwner(const shared_ptr<AActor>& owner) { _owner = owner; }

	bool IsActive() const { return _active; }
	void SetActive(bool active) { _active = active; }

	bool CanEverTick() const { return _canEverTick; }
	void SetCanEverTick(bool canEverTick) { _canEverTick = canEverTick; }

protected:
	weak_ptr<AActor> _owner;
	bool _active = true;
	bool _canEverTick = true;
};
