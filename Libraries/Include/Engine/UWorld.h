#pragma once
#include "UObject.h"
#include "AActor.h"

class UWorld : public UObject
{
	DECLARE_UCLASS(UWorld, UObject)
public:
	UWorld();
	virtual ~UWorld();

	template<typename T>
	shared_ptr<T> SpawnActor()
	{
		static_assert(is_base_of_v<AActor, T>, "T must derive from AActor");

		shared_ptr<T> actor = make_shared<T>();
		actor->SetWorld(static_pointer_cast<UWorld>(shared_from_this()));
		_actors.push_back(actor);
		actor->BeginPlay();
		return actor;
	}

	void DestroyActor(const shared_ptr<AActor>& actor);
	void Tick(float deltaTime);
	void Render();

	const vector<shared_ptr<AActor>>& GetActors() const { return _actors; }
	vector<shared_ptr<AActor>> GetRootActors() const;
	shared_ptr<AActor> FindActor(UObjectId id) const;

private:
	vector<shared_ptr<AActor>> _actors;
};
