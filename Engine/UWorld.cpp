#include "pch.h"
#include "UWorld.h"

UWorld::UWorld()
{
}

UWorld::~UWorld()
{
}

void UWorld::DestroyActor(const shared_ptr<AActor>& actor)
{
	if (!actor)
		return;

	actor->EndPlay();
	_actors.erase(remove(_actors.begin(), _actors.end(), actor), _actors.end());
}

void UWorld::Tick(float deltaTime)
{
	for (const shared_ptr<AActor>& actor : _actors)
		actor->Tick(deltaTime);
}
