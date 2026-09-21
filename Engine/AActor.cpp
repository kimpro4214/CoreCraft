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
