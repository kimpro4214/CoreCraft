#include "pch.h"
#include "SceneSerializer.h"
#include "Scene.h"
#include "Transform.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Texture.h"
#include "Utils.h"
#include "tinyxml2.h"
#include "Blueprint.h"
#include "Light.h"
#include "AnimatedModelRenderer.h"
#include "PlayerController.h"
#include "Shader.h"

using namespace tinyxml2;

namespace
{
	void WriteVec3(XMLElement* element, const char* prefix, const Vec3& value)
	{
		string x = string(prefix) + "X";
		string y = string(prefix) + "Y";
		string z = string(prefix) + "Z";
		element->SetAttribute(x.c_str(), value.x);
		element->SetAttribute(y.c_str(), value.y);
		element->SetAttribute(z.c_str(), value.z);
	}

	Vec3 ReadVec3(const XMLElement* element, const char* prefix, const Vec3& fallback)
	{
		Vec3 value = fallback;
		string x = string(prefix) + "X";
		string y = string(prefix) + "Y";
		string z = string(prefix) + "Z";
		element->QueryFloatAttribute(x.c_str(), &value.x);
		element->QueryFloatAttribute(y.c_str(), &value.y);
		element->QueryFloatAttribute(z.c_str(), &value.z);
		return value;
	}

	void SetError(string* error, const string& message)
	{
		if (error)
			*error = message;
	}
}

bool SceneSerializer::Save(const shared_ptr<Scene>& scene, const filesystem::path& path, string* error)
{
	if (scene == nullptr || path.empty())
	{
		SetError(error, "저장할 Scene 또는 경로가 없습니다.");
		return false;
	}

	tinyxml2::XMLDocument document;
	document.InsertEndChild(document.NewDeclaration());
	XMLElement* root = document.NewElement("Scene");
	root->SetAttribute("version", 1);
	root->SetAttribute("name", scene->GetName().c_str());
	document.InsertEndChild(root);

	for (const shared_ptr<GameObject>& object : scene->GetGameObjects())
	{
		XMLElement* objectElement = document.NewElement("GameObject");
		objectElement->SetAttribute("id", static_cast<int64_t>(object->GetId()));
		objectElement->SetAttribute("name", object->GetName().c_str());
		objectElement->SetAttribute("active", object->IsActive());

		shared_ptr<Transform> transform = object->GetTransform();
		if (transform && transform->GetParent())
			objectElement->SetAttribute("parent", static_cast<int64_t>(transform->GetParent()->GetGameObject()->GetId()));

		if (transform)
		{
			XMLElement* transformElement = document.NewElement("Transform");
			WriteVec3(transformElement, "position", transform->GetLocalPosition());
			WriteVec3(transformElement, "rotation", transform->GetLocalRotation());
			WriteVec3(transformElement, "scale", transform->GetLocalScale());
			objectElement->InsertEndChild(transformElement);
		}

		if (shared_ptr<Camera> camera = object->GetCamera())
			objectElement->InsertEndChild(document.NewElement("Camera"));

		if (shared_ptr<Light> light = object->GetLight())
		{
			XMLElement* lightElement = document.NewElement("Light");
			lightElement->SetAttribute("type", light->GetLightType() == LightType::Directional ? "Directional" : "Point");
			lightElement->SetAttribute("colorR", light->GetColor().x);
			lightElement->SetAttribute("colorG", light->GetColor().y);
			lightElement->SetAttribute("colorB", light->GetColor().z);
			lightElement->SetAttribute("intensity", light->GetIntensity());
			lightElement->SetAttribute("range", light->GetRange());
			objectElement->InsertEndChild(lightElement);
		}

		if (shared_ptr<MeshRenderer> renderer = object->GetMeshRenderer())
		{
			XMLElement* rendererElement = document.NewElement("MeshRenderer");
			if (renderer->GetMesh())
				rendererElement->SetAttribute("mesh", Utils::ToString(renderer->GetMesh()->GetName()).c_str());
			if (shared_ptr<Material> material = renderer->GetMaterial())
			{
				if (material->GetShader())
					rendererElement->SetAttribute("shader", Utils::ToString(material->GetShader()->GetFile()).c_str());
				if (material->GetDiffuseMap())
					rendererElement->SetAttribute("texture", Utils::ToString(material->GetDiffuseMap()->GetPath()).c_str());
				const MaterialDesc& desc = material->GetMaterialDesc();
				rendererElement->SetAttribute("ambient", desc.ambient.x);
				rendererElement->SetAttribute("specular", desc.specular.x);
			}
			objectElement->InsertEndChild(rendererElement);
		}

		if (shared_ptr<AnimatedModelRenderer> renderer = object->GetAnimatedModelRenderer())
		{
			XMLElement* animatedElement = document.NewElement("AnimatedModel");
			animatedElement->SetAttribute("model", Utils::ToString(renderer->GetModelPath()).c_str());
			animatedElement->SetAttribute("material", Utils::ToString(renderer->GetMaterialPath()).c_str());
			for (const wstring& animationPath : renderer->GetAnimationPaths())
			{
				XMLElement* animationElement = document.NewElement("Animation");
				animationElement->SetAttribute("path", Utils::ToString(animationPath).c_str());
				animatedElement->InsertEndChild(animationElement);
			}
			objectElement->InsertEndChild(animatedElement);
		}

		if (shared_ptr<PlayerController> player = object->GetPlayerController())
		{
			XMLElement* playerElement = document.NewElement("PlayerController");
			if (shared_ptr<GameObject> camera = player->GetCamera())
				playerElement->SetAttribute("camera", static_cast<int64_t>(camera->GetId()));
			playerElement->SetAttribute("moveSpeed", player->GetMoveSpeed());
			playerElement->SetAttribute("jumpSpeed", player->GetJumpSpeed());
			playerElement->SetAttribute("gravity", player->GetGravity());
			playerElement->SetAttribute("cameraDistance", player->GetCameraDistance());
			playerElement->SetAttribute("cameraHeight", player->GetCameraHeight());
			objectElement->InsertEndChild(playerElement);
		}

		if (shared_ptr<BlueprintComponent> blueprint = object->GetBlueprint())
		{
			if (blueprint->GetAsset() && !blueprint->GetAsset()->GetPath().empty())
			{
				XMLElement* blueprintElement = document.NewElement("Blueprint");
				blueprintElement->SetAttribute("asset", blueprint->GetAsset()->GetPath().generic_string().c_str());
				objectElement->InsertEndChild(blueprintElement);
			}
		}

		root->InsertEndChild(objectElement);
	}

	filesystem::create_directories(path.parent_path());
	filesystem::path temporaryPath = path;
	temporaryPath += L".tmp";
	if (document.SaveFile(temporaryPath.string().c_str()) != XML_SUCCESS)
	{
		SetError(error, "임시 Scene 파일을 저장하지 못했습니다.");
		return false;
	}

	if (!MoveFileExW(temporaryPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		SetError(error, "Scene 파일을 교체하지 못했습니다.");
		return false;
	}

	scene->SetPath(path);
	scene->SetDirty(false);
	return true;
}

shared_ptr<Scene> SceneSerializer::Load(const filesystem::path& path, const shared_ptr<Shader>& defaultShader, string* error)
{
	tinyxml2::XMLDocument document;
	if (document.LoadFile(path.string().c_str()) != XML_SUCCESS)
	{
		SetError(error, "Scene 파일을 열지 못했습니다.");
		return nullptr;
	}

	XMLElement* root = document.FirstChildElement("Scene");
	if (root == nullptr || root->IntAttribute("version", 0) != 1)
	{
		SetError(error, "지원하지 않는 Scene 형식입니다.");
		return nullptr;
	}

	shared_ptr<Scene> scene = make_shared<Scene>(root->Attribute("name") ? root->Attribute("name") : path.stem().string());
	unordered_map<ObjectId, ObjectId> parents;
	unordered_map<ObjectId, ObjectId> playerCameras;

	for (XMLElement* objectElement = root->FirstChildElement("GameObject"); objectElement; objectElement = objectElement->NextSiblingElement("GameObject"))
	{
		uint64_t id = static_cast<uint64_t>(objectElement->Int64Attribute("id", 0));
		shared_ptr<GameObject> object = scene->CreateGameObject(objectElement->Attribute("name") ? objectElement->Attribute("name") : "GameObject");
		object->SetId(id);
		object->SetActive(objectElement->BoolAttribute("active", true));

		if (XMLElement* transformElement = objectElement->FirstChildElement("Transform"))
		{
			shared_ptr<Transform> transform = object->GetTransform();
			transform->SetLocalPosition(ReadVec3(transformElement, "position", Vec3::Zero));
			transform->SetLocalRotation(ReadVec3(transformElement, "rotation", Vec3::Zero));
			transform->SetLocalScale(ReadVec3(transformElement, "scale", Vec3::One));
		}

		uint64_t parentId = static_cast<uint64_t>(objectElement->Int64Attribute("parent", 0));
		if (parentId != 0)
			parents[id] = parentId;

		if (objectElement->FirstChildElement("Camera"))
			object->AddComponent(make_shared<Camera>());

		if (XMLElement* lightElement = objectElement->FirstChildElement("Light"))
		{
			shared_ptr<Light> light = make_shared<Light>();
			const char* type = lightElement->Attribute("type");
			light->SetLightType(type && strcmp(type, "Point") == 0 ? LightType::Point : LightType::Directional);
			Color color(1.f, 1.f, 1.f, 1.f);
			lightElement->QueryFloatAttribute("colorR", &color.x);
			lightElement->QueryFloatAttribute("colorG", &color.y);
			lightElement->QueryFloatAttribute("colorB", &color.z);
			light->SetColor(color);
			light->SetIntensity(lightElement->FloatAttribute("intensity", 1.f));
			light->SetRange(lightElement->FloatAttribute("range", 10.f));
			object->AddComponent(light);
		}

		if (XMLElement* rendererElement = objectElement->FirstChildElement("MeshRenderer"))
		{
			const char* meshName = rendererElement->Attribute("mesh");
			shared_ptr<Mesh> mesh = meshName ? RESOURCES->Get<Mesh>(Utils::ToWString(meshName)) : nullptr;
			if (mesh && defaultShader)
			{
				shared_ptr<MeshRenderer> renderer = make_shared<MeshRenderer>();
				renderer->SetMesh(mesh);
				shared_ptr<Material> material = make_shared<Material>();
				material->SetShader(defaultShader);
				material->GetMaterialDesc().ambient = Vec4(rendererElement->FloatAttribute("ambient", 0.2f));
				material->GetMaterialDesc().diffuse = Vec4(1.f);
				material->GetMaterialDesc().specular = Vec4(rendererElement->FloatAttribute("specular", 1.f));
				if (const char* texturePath = rendererElement->Attribute("texture"))
				{
					wstring pathValue = Utils::ToWString(texturePath);
					material->SetDiffuseMap(RESOURCES->GetOrAddTexture(pathValue, pathValue));
				}
				renderer->SetMaterial(material);
				object->AddComponent(renderer);
			}
		}

		if (XMLElement* animatedElement = objectElement->FirstChildElement("AnimatedModel"))
		{
			const char* modelPath = animatedElement->Attribute("model");
			const char* materialPath = animatedElement->Attribute("material");
			if (modelPath && materialPath)
			{
				vector<wstring> animationPaths;
				for (XMLElement* animationElement = animatedElement->FirstChildElement("Animation"); animationElement; animationElement = animationElement->NextSiblingElement("Animation"))
				{
					if (const char* animationPath = animationElement->Attribute("path"))
						animationPaths.push_back(Utils::ToWString(animationPath));
				}
				shared_ptr<Shader> animationShader = make_shared<Shader>(L"21. CoreCraftAnimation.fx");
				shared_ptr<AnimatedModelRenderer> renderer = make_shared<AnimatedModelRenderer>(animationShader);
				renderer->LoadModel(Utils::ToWString(modelPath), Utils::ToWString(materialPath), animationPaths);
				object->AddComponent(renderer);
			}
		}

		if (XMLElement* playerElement = objectElement->FirstChildElement("PlayerController"))
		{
			shared_ptr<PlayerController> player = make_shared<PlayerController>();
			player->SetMoveSpeed(playerElement->FloatAttribute("moveSpeed", 4.f));
			player->SetJumpSpeed(playerElement->FloatAttribute("jumpSpeed", 7.f));
			player->SetGravity(playerElement->FloatAttribute("gravity", -18.f));
			player->SetCameraDistance(playerElement->FloatAttribute("cameraDistance", 5.5f));
			player->SetCameraHeight(playerElement->FloatAttribute("cameraHeight", 1.8f));
			object->AddComponent(player);
			ObjectId cameraId = static_cast<ObjectId>(playerElement->Int64Attribute("camera", 0));
			if (cameraId != 0)
				playerCameras[id] = cameraId;
		}

		if (XMLElement* blueprintElement = objectElement->FirstChildElement("Blueprint"))
		{
			if (const char* assetPath = blueprintElement->Attribute("asset"))
			{
				shared_ptr<BlueprintAsset> asset = BlueprintAsset::Load(assetPath);
				if (asset)
				{
					shared_ptr<BlueprintComponent> blueprint = make_shared<BlueprintComponent>();
					blueprint->SetAsset(asset);
					object->AddComponent(blueprint);
				}
			}
		}
	}

	for (const auto& [childId, parentId] : parents)
	{
		shared_ptr<GameObject> child = scene->FindGameObject(childId);
		shared_ptr<GameObject> parent = scene->FindGameObject(parentId);
		if (child && parent)
			child->GetTransform()->SetParent(parent->GetTransform());
	}
	for (const auto& [playerId, cameraId] : playerCameras)
	{
		shared_ptr<GameObject> playerObject = scene->FindGameObject(playerId);
		shared_ptr<GameObject> cameraObject = scene->FindGameObject(cameraId);
		if (playerObject && cameraObject && playerObject->GetPlayerController())
			playerObject->GetPlayerController()->SetCamera(cameraObject);
	}

	scene->SetPath(path);
	scene->SetDirty(false);
	return scene;
}
