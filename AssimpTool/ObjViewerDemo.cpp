#include "pch.h"
#include "ObjViewerDemo.h"
#include "Camera.h"
#include "CameraScript.h"
#include "GameObject.h"
#include "Converter.h"
#include "Model.h"
#include "ModelMesh.h"
#include "Material.h"
#include "AActor.h"
#include "UWorld.h"
#include "USceneComponent.h"
#include "UStaticMeshComponent.h"
#include "UComponentRegistry.h"
#include "Utils.h"

namespace
{
	constexpr float RadToDeg = 180.f / XM_PI;
	constexpr float DegToRad = XM_PI / 180.f;
}

void ObjViewerDemo::Init()
{
	RESOURCES->Init();
	_shader = make_shared<Shader>(L"15. ModelDemo.fx");

	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(make_shared<CameraScript>());

	_world = make_shared<UWorld>();

	COMPONENT_REGISTRY->Register<USceneComponent>();
	COMPONENT_REGISTRY->Register<UStaticMeshComponent>();

	ImportAsset(L"Tower/Tower.fbx");

	RENDER->Init(_shader);
}

void ObjViewerDemo::Update()
{
	_camera->Update();
	RENDER->Update();

	LightDesc lightDesc;
	lightDesc.ambient = Vec4(0.4f);
	lightDesc.diffuse = Vec4(1.f);
	lightDesc.specular = Vec4(0.f);
	lightDesc.direction = Vec3(1.f, 0.f, 1.f);
	RENDER->PushLightData(lightDesc);

	_world->Tick(TIME->GetDeltaTime());
}

void ObjViewerDemo::Render()
{
	_world->Render();
}

shared_ptr<AActor> ObjViewerDemo::ImportAsset(const wstring& assetFile)
{
	filesystem::path path(assetFile);
	wstring folder = path.parent_path().filename().wstring();
	wstring savePath = folder.empty() ? path.stem().wstring() : folder + L"/" + folder;

	shared_ptr<Converter> converter = make_shared<Converter>();
	converter->ReadAssetFile(assetFile);
	converter->ExportModelData(savePath);
	converter->ExportMaterialData(savePath);

	shared_ptr<Model> model = make_shared<Model>();
	model->ReadModel(savePath);
	model->ReadMaterial(savePath);

	_availableModels.push_back({ savePath, model });

	return CreateStaticMeshActor(model, Utils::ToString(savePath));
}

shared_ptr<AActor> ObjViewerDemo::CreateStaticMeshActor(const shared_ptr<Model>& model, const string& name)
{
	shared_ptr<AActor> actor = _world->SpawnActor<AActor>();
	actor->SetName(FName(name));

	shared_ptr<UStaticMeshComponent> meshComponent = actor->AddComponent<UStaticMeshComponent>();
	meshComponent->SetShader(_shader);
	meshComponent->SetStaticMesh(model);

	_selectedId = actor->GetUniqueId();
	return actor;
}

void ObjViewerDemo::RenderUI()
{
	DrawOutliner();
	DrawInspector();
	DrawImportPanel();
}

void ObjViewerDemo::DrawOutliner()
{
	ImGui::Begin("Outliner");

	if (ImGui::Button("+ Empty Actor"))
	{
		shared_ptr<AActor> actor = _world->SpawnActor<AActor>();
		actor->AddComponent<USceneComponent>();
		actor->SetName(FName("Actor"));
		_selectedId = actor->GetUniqueId();
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete Selected"))
	{
		shared_ptr<AActor> selected = _world->FindActor(_selectedId);
		if (selected)
		{
			_world->DestroyActor(selected);
			_selectedId = 0;
		}
	}
	ImGui::Separator();

	for (const shared_ptr<AActor>& root : _world->GetRootActors())
		DrawOutlinerNode(root);

	ImGui::End();
}

void ObjViewerDemo::DrawOutlinerNode(const shared_ptr<AActor>& actor)
{
	vector<shared_ptr<AActor>> children = actor->GetAttachedActors();
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (children.empty())
		flags |= ImGuiTreeNodeFlags_Leaf;
	if (_selectedId == actor->GetUniqueId())
		flags |= ImGuiTreeNodeFlags_Selected;

	bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(actor->GetUniqueId()), flags, "%s", actor->GetName().ToString().c_str());
	if (ImGui::IsItemClicked())
		_selectedId = actor->GetUniqueId();

	if (ImGui::BeginDragDropSource())
	{
		UObjectId payload = actor->GetUniqueId();
		ImGui::SetDragDropPayload("OUTLINER_ACTOR", &payload, sizeof(UObjectId));
		ImGui::Text("%s", actor->GetName().ToString().c_str());
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OUTLINER_ACTOR"))
		{
			UObjectId droppedId = *static_cast<const UObjectId*>(payload->Data);
			shared_ptr<AActor> dropped = _world->FindActor(droppedId);
			if (dropped && dropped != actor)
				dropped->AttachToActor(actor);
		}
		ImGui::EndDragDropTarget();
	}

	if (open)
	{
		for (const shared_ptr<AActor>& child : children)
			DrawOutlinerNode(child);
		ImGui::TreePop();
	}
}

void ObjViewerDemo::DrawInspector()
{
	ImGui::Begin("Inspector");

	shared_ptr<AActor> selected = _world->FindActor(_selectedId);
	if (!selected)
	{
		ImGui::TextDisabled("Select an Actor");
		ImGui::End();
		return;
	}

	char name[128] = {};
	strncpy_s(name, selected->GetName().ToString().c_str(), _TRUNCATE);
	if (ImGui::InputText("Name", name, sizeof(name)))
		selected->SetName(FName(name));

	bool active = selected->IsActive();
	if (ImGui::Checkbox("Active", &active))
		selected->SetActive(active);

	if (shared_ptr<USceneComponent> root = selected->GetRootComponent())
	{
		ImGui::SeparatorText("Transform");
		Vec3 location = root->GetRelativeLocation();
		if (ImGui::DragFloat3("Location", &location.x, 0.05f))
			root->SetRelativeLocation(location);

		Vec3 rotation = root->GetRelativeRotation() * RadToDeg;
		if (ImGui::DragFloat3("Rotation", &rotation.x, 0.25f))
			root->SetRelativeRotation(rotation * DegToRad);

		Vec3 scale = root->GetRelativeScale();
		if (ImGui::DragFloat3("Scale", &scale.x, 0.02f, 0.001f, 1000.f))
			root->SetRelativeScale(scale);
	}

	ImGui::SeparatorText("Components");
	shared_ptr<UActorComponent> pendingRemove;
	for (const shared_ptr<UActorComponent>& component : selected->GetComponents())
	{
		ImGui::PushID(component.get());

		bool isRoot = (component == selected->GetRootComponent());
		ImGui::BulletText("%s%s", component->GetClass()->GetName().ToString().c_str(), isRoot ? " (Root)" : "");
		if (!isRoot)
		{
			ImGui::SameLine();
			if (ImGui::SmallButton("Remove"))
				pendingRemove = component;
		}

		if (shared_ptr<UStaticMeshComponent> meshComponent = dynamic_pointer_cast<UStaticMeshComponent>(component))
		{
			int currentIndex = -1;
			for (size_t i = 0; i < _availableModels.size(); i++)
			{
				if (_availableModels[i].second == meshComponent->GetStaticMesh())
				{
					currentIndex = static_cast<int>(i);
					break;
				}
			}
			string preview = currentIndex >= 0 ? Utils::ToString(_availableModels[currentIndex].first) : "(None)";
			if (ImGui::BeginCombo("Static Mesh", preview.c_str()))
			{
				for (size_t i = 0; i < _availableModels.size(); i++)
				{
					bool isSelected = (static_cast<int>(i) == currentIndex);
					if (ImGui::Selectable(Utils::ToString(_availableModels[i].first).c_str(), isSelected))
					{
						meshComponent->SetStaticMesh(_availableModels[i].second);
						meshComponent->SetShader(_shader);
					}
				}
				ImGui::EndCombo();
			}
		}

		ImGui::PopID();
	}
	if (pendingRemove)
		selected->RemoveComponent(pendingRemove);

	ImGui::Separator();
	const vector<FName>& names = COMPONENT_REGISTRY->GetRegisteredNames();
	if (!names.empty())
	{
		static int selectedTypeIndex = 0;
		if (selectedTypeIndex >= static_cast<int>(names.size()))
			selectedTypeIndex = 0;

		if (ImGui::BeginCombo("##AddComponentType", names[selectedTypeIndex].ToString().c_str()))
		{
			for (int i = 0; i < static_cast<int>(names.size()); i++)
			{
				bool isSelected = (i == selectedTypeIndex);
				if (ImGui::Selectable(names[i].ToString().c_str(), isSelected))
					selectedTypeIndex = i;
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Component"))
		{
			shared_ptr<UActorComponent> created = COMPONENT_REGISTRY->Create(names[selectedTypeIndex]);
			if (created)
				selected->AddComponent(created);
		}
	}

	ImGui::End();
}

void ObjViewerDemo::DrawImportPanel()
{
	ImGui::Begin("Import");
	ImGui::TextWrapped("Resources/Assets/ 기준 상대 경로 (예: Tower/Tower.fbx, MyModel/model.obj)");
	ImGui::InputText("Asset Path", _importPathBuffer, sizeof(_importPathBuffer));
	if (ImGui::Button("Import"))
	{
		wstring assetFile = Utils::ToWString(_importPathBuffer);
		if (filesystem::exists(filesystem::path(L"../Resources/Assets/") / assetFile))
		{
			ImportAsset(assetFile);
			_importStatus = "Imported: " + string(_importPathBuffer);
		}
		else
		{
			_importStatus = "File not found: " + string(_importPathBuffer);
		}
	}
	if (!_importStatus.empty())
		ImGui::TextUnformatted(_importStatus.c_str());
	ImGui::End();
}
