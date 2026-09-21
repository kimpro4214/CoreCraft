#include "EnginePch.h"
#include "Scene.h"
#include "SceneSerializer.h"
#include "Blueprint.h"
#include "Transform.h"
#include "Light.h"
#include "PlayerController.h"
#include "AActor.h"
#include "USceneComponent.h"
#include "UWorld.h"
#include "UComponentRegistry.h"
#include "Console.h"

namespace
{
	bool NearlyEqual(float a, float b)
	{
		return abs(a - b) < 0.0001f;
	}

	bool TestTransformHierarchy()
	{
		shared_ptr<Scene> scene = make_shared<Scene>("Hierarchy");
		shared_ptr<GameObject> parent = scene->CreateGameObject("Parent");
		shared_ptr<GameObject> child = scene->CreateGameObject("Child");
		if (!child->GetTransform()->SetParent(parent->GetTransform())) return false;
		if (parent->GetTransform()->SetParent(child->GetTransform())) return false;
		return child->GetTransform()->GetParent() == parent->GetTransform();
	}

	bool TestSceneRoundTrip()
	{
		shared_ptr<Scene> scene = make_shared<Scene>("RoundTrip");
		shared_ptr<GameObject> parent = scene->CreateGameObject("Parent");
		parent->GetTransform()->SetLocalPosition({ 1.f, 2.f, 3.f });
		shared_ptr<GameObject> child = scene->CreateGameObject("Child");
		child->GetTransform()->SetParent(parent->GetTransform());

		filesystem::path path = filesystem::temp_directory_path() / L"CoreCraftSceneTest.scene.xml";
		string error;
		if (!SceneSerializer::Save(scene, path, &error)) return false;
		shared_ptr<Scene> loaded = SceneSerializer::Load(path, nullptr, &error);
		filesystem::remove(path);
		if (!loaded || loaded->GetGameObjects().size() != 2) return false;
		shared_ptr<GameObject> loadedParent = loaded->FindGameObject(parent->GetId());
		shared_ptr<GameObject> loadedChild = loaded->FindGameObject(child->GetId());
		if (!loadedParent || !loadedChild) return false;
		Vec3 position = loadedParent->GetTransform()->GetLocalPosition();
		return NearlyEqual(position.x, 1.f) && NearlyEqual(position.y, 2.f) && NearlyEqual(position.z, 3.f)
			&& loadedChild->GetTransform()->GetParent() == loadedParent->GetTransform();
	}

	bool TestBlueprintCompile()
	{
		BlueprintAsset asset;
		BlueprintNode& tick = asset.AddNode(BlueprintNodeType::Tick);
		uint64 tickOutput = tick.outputPin;
		BlueprintNode& rotate = asset.AddNode(BlueprintNodeType::AddRotation);
		rotate.vectorValue = { 0.f, 1.f, 0.f };
		if (!asset.AddLink(tickOutput, rotate.inputPin)) return false;
		CompiledBlueprint compiled;
		vector<string> diagnostics;
		return asset.Compile(compiled, diagnostics) && compiled.tick.size() == 1
			&& compiled.tick[0].operation == BlueprintOperation::AddRotation;
	}

	bool TestNoHierarchyLeak()
	{
		weak_ptr<GameObject> parentWeak;
		weak_ptr<GameObject> childWeak;
		{
			shared_ptr<Scene> scene = make_shared<Scene>("Lifetime");
			shared_ptr<GameObject> parent = scene->CreateGameObject("Parent");
			shared_ptr<GameObject> child = scene->CreateGameObject("Child");
			child->GetTransform()->SetParent(parent->GetTransform());
			parentWeak = parent;
			childWeak = child;
			scene->Clear();
		}
		return parentWeak.expired() && childWeak.expired();
	}

	bool TestLightAndPlayerRoundTrip()
	{
		shared_ptr<Scene> scene = make_shared<Scene>("Gameplay");
		shared_ptr<GameObject> lightObject = scene->CreateGameObject("Point Light");
		lightObject->GetTransform()->SetPosition({ 1.f, 3.f, 2.f });
		shared_ptr<Light> light = make_shared<Light>();
		light->SetLightType(LightType::Point);
		light->SetIntensity(2.5f);
		light->SetRange(7.f);
		lightObject->AddComponent(light);

		shared_ptr<GameObject> camera = scene->CreateGameObject("Camera");
		shared_ptr<GameObject> playerObject = scene->CreateGameObject("Player");
		shared_ptr<PlayerController> player = make_shared<PlayerController>();
		player->SetCamera(camera);
		playerObject->AddComponent(player);

		filesystem::path path = filesystem::temp_directory_path() / L"CoreCraftGameplayTest.scene.xml";
		string error;
		if (!SceneSerializer::Save(scene, path, &error)) return false;
		shared_ptr<Scene> loaded = SceneSerializer::Load(path, nullptr, &error);
		filesystem::remove(path);
		if (!loaded) return false;
		shared_ptr<GameObject> loadedLight = loaded->FindGameObject(lightObject->GetId());
		shared_ptr<GameObject> loadedPlayer = loaded->FindGameObject(playerObject->GetId());
		if (!loadedLight || !loadedLight->GetLight() || !loadedPlayer || !loadedPlayer->GetPlayerController()) return false;
		return loadedLight->GetLight()->GetLightType() == LightType::Point
			&& NearlyEqual(loadedLight->GetLight()->GetIntensity(), 2.5f)
			&& NearlyEqual(loadedLight->GetLight()->GetRange(), 7.f)
			&& loadedPlayer->GetPlayerController()->GetCamera()
			&& loadedPlayer->GetPlayerController()->GetCamera()->GetId() == camera->GetId();
	}

	class CountingSceneComponent : public USceneComponent
	{
	public:
		int beginPlayCount = 0;
		int tickCount = 0;
		int endPlayCount = 0;

		virtual void BeginPlay() override { beginPlayCount++; }
		virtual void TickComponent(float deltaTime) override { tickCount++; }
		virtual void EndPlay() override { endPlayCount++; }
	};

	class CountingActor : public AActor
	{
	public:
		int tickCount = 0;

		virtual void Tick(float deltaTime) override
		{
			AActor::Tick(deltaTime);
			tickCount++;
		}
	};

	bool TestUObjectRTTI()
	{
		shared_ptr<AActor> actor = make_shared<AActor>();
		if (!IsA<UObject>(actor.get())) return false;
		if (!IsA<AActor>(actor.get())) return false;
		if (IsA<USceneComponent>(actor.get())) return false;

		shared_ptr<UObject> asUObject = actor;
		if (!Cast<AActor>(asUObject)) return false;
		if (Cast<USceneComponent>(asUObject)) return false;
		return true;
	}

	bool TestActorComponentOwnerAndLifecycle()
	{
		shared_ptr<AActor> actor = make_shared<AActor>();
		shared_ptr<CountingSceneComponent> component = actor->AddComponent<CountingSceneComponent>();
		if (component->GetOwner() != actor) return false;
		if (actor->GetRootComponent() != component) return false;

		actor->BeginPlay();
		actor->Tick(0.016f);
		actor->EndPlay();

		return component->beginPlayCount == 1 && component->tickCount == 1 && component->endPlayCount == 1;
	}

	bool TestSceneComponentHierarchyWorldMatrix()
	{
		shared_ptr<AActor> actor = make_shared<AActor>();
		shared_ptr<USceneComponent> parent = actor->AddComponent<USceneComponent>();
		parent->SetRelativeLocation({ 1.f, 2.f, 3.f });

		shared_ptr<USceneComponent> child = make_shared<USceneComponent>();
		if (!child->AttachToComponent(parent)) return false;
		child->SetRelativeLocation({ 0.f, 1.f, 0.f });

		Vec3 worldLocation = child->GetWorldLocation();
		return NearlyEqual(worldLocation.x, 1.f) && NearlyEqual(worldLocation.y, 3.f) && NearlyEqual(worldLocation.z, 3.f);
	}

	bool TestWorldSpawnTickDestroy()
	{
		shared_ptr<UWorld> world = make_shared<UWorld>();
		shared_ptr<CountingActor> actor = world->SpawnActor<CountingActor>();
		if (world->GetActors().size() != 1) return false;

		world->Tick(0.016f);
		if (actor->tickCount != 1) return false;

		world->DestroyActor(actor);
		return world->GetActors().empty();
	}

	bool TestActorAttachDetach()
	{
		shared_ptr<UWorld> world = make_shared<UWorld>();
		shared_ptr<AActor> a = world->SpawnActor<AActor>();
		shared_ptr<AActor> b = world->SpawnActor<AActor>();
		shared_ptr<AActor> c = world->SpawnActor<AActor>();
		a->AddComponent<USceneComponent>();
		b->AddComponent<USceneComponent>();
		c->AddComponent<USceneComponent>();

		if (!b->AttachToActor(a)) return false;
		if (!c->AttachToActor(b)) return false;

		if (b->GetParentActor() != a) return false;
		if (c->GetParentActor() != b) return false;
		vector<shared_ptr<AActor>> aChildren = a->GetAttachedActors();
		if (aChildren.size() != 1 || aChildren[0] != b) return false;

		vector<shared_ptr<AActor>> roots = world->GetRootActors();
		if (roots.size() != 1 || roots[0] != a) return false;

		b->DetachFromActor();
		if (b->GetParentActor() != nullptr) return false;
		if (!a->GetAttachedActors().empty()) return false;

		roots = world->GetRootActors();
		return roots.size() == 2
			&& find(roots.begin(), roots.end(), a) != roots.end()
			&& find(roots.begin(), roots.end(), b) != roots.end();
	}

	bool TestActorRemoveComponent()
	{
		shared_ptr<AActor> actor = make_shared<AActor>();
		shared_ptr<USceneComponent> root = actor->AddComponent<USceneComponent>();
		shared_ptr<USceneComponent> child = actor->AddComponent<USceneComponent>();
		if (!child->AttachToComponent(root)) return false;

		if (!actor->RemoveComponent(root)) return false;
		if (actor->GetRootComponent() != nullptr) return false;
		if (child->GetAttachParent() != nullptr) return false;

		return actor->RemoveComponent(child);
	}

	bool TestComponentRegistryCreate()
	{
		COMPONENT_REGISTRY->Register<USceneComponent>();

		shared_ptr<UActorComponent> created = COMPONENT_REGISTRY->Create(FName("USceneComponent"));
		if (!created || !IsA<USceneComponent>(created.get())) return false;

		return COMPONENT_REGISTRY->Create(FName("NoSuchComponentXYZ")) == nullptr;
	}

	bool TestWorldDestroyActorDetachesChildren()
	{
		shared_ptr<UWorld> world = make_shared<UWorld>();
		shared_ptr<AActor> parent = world->SpawnActor<AActor>();
		shared_ptr<AActor> child = world->SpawnActor<AActor>();
		parent->AddComponent<USceneComponent>();
		child->AddComponent<USceneComponent>();
		if (!child->AttachToActor(parent)) return false;

		world->DestroyActor(parent);

		if (child->GetParentActor() != nullptr) return false;
		return world->GetActors().size() == 1 && world->GetActors()[0] == child;
	}

	size_t CountObjects()
	{
		size_t count = 0;
		for (FObjectIterator<UObject> it; it; ++it)
			count++;
		return count;
	}

	size_t CountActors()
	{
		size_t count = 0;
		for (FObjectIterator<AActor> it; it; ++it)
			count++;
		return count;
	}

	bool TestObjectIterator()
	{
		size_t objectsBefore = CountObjects();
		size_t actorsBefore = CountActors();

		{
			shared_ptr<AActor> a = make_shared<AActor>();
			shared_ptr<AActor> b = make_shared<AActor>();
			shared_ptr<USceneComponent> c = make_shared<USceneComponent>();

			if (CountActors() != actorsBefore + 2) return false;
			if (CountObjects() != objectsBefore + 3) return false;

			bool foundA = false;
			bool foundB = false;
			for (FObjectIterator<AActor> it; it; ++it)
			{
				if (*it == a.get()) foundA = true;
				if (*it == b.get()) foundB = true;
			}
			if (!foundA || !foundB) return false;
		}

		return CountObjects() == objectsBefore && CountActors() == actorsBefore;
	}

	bool TestConsoleCommands()
	{
		CONSOLE->Init();
		CONSOLE->Execute("clear");

		vector<string> echoedArgs;
		CONSOLE->RegisterCommand("echo", [&echoedArgs](const vector<string>& args)
		{
			echoedArgs = args;
		});

		CONSOLE->Execute("echo hello world");
		if (echoedArgs.size() != 2 || echoedArgs[0] != "hello" || echoedArgs[1] != "world") return false;

		CONSOLE->Execute("nosuchcommand");
		bool foundUnknown = false;
		for (const string& line : CONSOLE->GetHistory())
		{
			if (line == "Unknown command: nosuchcommand")
				foundUnknown = true;
		}
		if (!foundUnknown) return false;

		CONSOLE->Execute("clear");
		return CONSOLE->GetHistory().empty();
	}
}

int main()
{
	const pair<const char*, bool(*)()> tests[] =
	{
		{ "Transform hierarchy", TestTransformHierarchy },
		{ "Scene round trip", TestSceneRoundTrip },
		{ "Blueprint compile", TestBlueprintCompile },
		{ "Hierarchy lifetime", TestNoHierarchyLeak },
		{ "Light and player round trip", TestLightAndPlayerRoundTrip },
		{ "UObject RTTI", TestUObjectRTTI },
		{ "ActorComponent owner and lifecycle", TestActorComponentOwnerAndLifecycle },
		{ "SceneComponent hierarchy world matrix", TestSceneComponentHierarchyWorldMatrix },
		{ "World spawn/tick/destroy", TestWorldSpawnTickDestroy },
		{ "Actor attach/detach", TestActorAttachDetach },
		{ "Actor RemoveComponent", TestActorRemoveComponent },
		{ "Component registry create", TestComponentRegistryCreate },
		{ "World DestroyActor detaches children", TestWorldDestroyActorDetachesChildren },
		{ "FObjectIterator", TestObjectIterator },
		{ "Console commands", TestConsoleCommands },
	};

	for (const auto& [name, test] : tests)
	{
		if (!test())
		{
			cout << "[FAIL] " << name << endl;
			return 1;
		}
		cout << "[PASS] " << name << endl;
	}
	return 0;
}
