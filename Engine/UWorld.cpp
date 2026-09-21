#include "pch.h"
#include "UWorld.h"
#include "UPrimitiveComponent.h"

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

	for (const shared_ptr<AActor>& child : actor->GetAttachedActors())
		child->DetachFromActor();

	actor->EndPlay();
	_actors.erase(remove(_actors.begin(), _actors.end(), actor), _actors.end());
}

void UWorld::Tick(float deltaTime)
{
	for (const shared_ptr<AActor>& actor : _actors)
		actor->Tick(deltaTime);
}

void UWorld::Render()
{
	for (const shared_ptr<AActor>& actor : _actors)
	{
		if (!actor->IsActive())
			continue;

		for (const shared_ptr<UActorComponent>& component : actor->GetComponents())
		{
			shared_ptr<UPrimitiveComponent> primitive = dynamic_pointer_cast<UPrimitiveComponent>(component);
			if (primitive && primitive->IsActive() && primitive->IsVisible())
				primitive->Render();
		}
	}
}

vector<shared_ptr<AActor>> UWorld::GetRootActors() const
{
	vector<shared_ptr<AActor>> result;
	for (const shared_ptr<AActor>& actor : _actors)
	{
		if (!actor->GetParentActor())
			result.push_back(actor);
	}
	return result;
}

shared_ptr<AActor> UWorld::FindActor(UObjectId id) const
{
	for (const shared_ptr<AActor>& actor : _actors)
	{
		if (actor->GetUniqueId() == id)
			return actor;
	}
	return nullptr;
}
