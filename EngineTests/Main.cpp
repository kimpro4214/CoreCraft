#include "EnginePch.h"
#include "Scene.h"
#include "SceneSerializer.h"
#include "Blueprint.h"
#include "Transform.h"
#include "Light.h"
#include "PlayerController.h"

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
