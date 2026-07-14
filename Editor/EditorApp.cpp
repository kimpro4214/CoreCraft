#include "pch.h"
#include "EditorApp.h"
#include "Scene.h"
#include "SceneSerializer.h"
#include "GameObject.h"
#include "Transform.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "RenderTexture.h"
#include "Blueprint.h"
#include "Light.h"
#include "AnimatedModelRenderer.h"
#include "PlayerController.h"
#include "Model.h"

namespace ed = ax::NodeEditor;

namespace
{
	constexpr float RadToDeg = 180.f / XM_PI;
	constexpr float DegToRad = XM_PI / 180.f;

	const char* BlueprintNodeName(BlueprintNodeType type)
	{
		switch (type)
		{
		case BlueprintNodeType::BeginPlay: return "Event BeginPlay";
		case BlueprintNodeType::Tick: return "Event Tick";
		case BlueprintNodeType::SetLocation: return "Set Location";
		case BlueprintNodeType::AddOffset: return "Add Offset";
		case BlueprintNodeType::AddRotation: return "Add Rotation";
		case BlueprintNodeType::PrintLog: return "Print Log";
		}
		return "Unknown";
	}

	ImColor BlueprintNodeColor(BlueprintNodeType type)
	{
		if (type == BlueprintNodeType::BeginPlay || type == BlueprintNodeType::Tick)
			return ImColor(150, 35, 45);
		return ImColor(45, 85, 145);
	}
}

void EditorApp::Init()
{
	RESOURCES->Init();
	_shader = make_shared<Shader>(L"20. CoreCraft.fx");
	RENDER->Init(_shader);
	_animationShader = make_shared<Shader>(L"21. CoreCraftAnimation.fx");
	RENDER->RegisterShader(_animationShader);

	_viewportTarget = make_unique<RenderTexture>();
	_viewportTarget->Create(_viewportWidth, _viewportHeight);

	_editorCamera = make_shared<GameObject>();
	_editorCamera->SetName("EditorCamera");
	_editorCamera->GetOrAddTransform()->SetPosition({ 0.f, 3.f, -8.f });
	_editorCamera->GetTransform()->SetRotation({ 0.2f, 0.f, 0.f });
	_editorCamera->AddComponent(make_shared<Camera>());
	_editorCamera->GetCamera()->SetViewportSize(static_cast<float>(_viewportWidth), static_cast<float>(_viewportHeight));

	ed::Config config;
	config.SettingsFile = "CoreCraftBlueprint.json";
	_nodeEditor = ed::CreateEditor(&config);

	CreateDefaultScene();
	Log("Core Craft Editor started.");
}

void EditorApp::CreateDefaultScene()
{
	_scene = SCENE->CreateScene("Demo");
	CreatePrimitive("Sphere", L"Sphere", { -1.5f, 0.f, 0.f });
	CreatePrimitive("Cube", L"Cube", { 1.5f, 0.f, 0.f });
	shared_ptr<GameObject> sun = CreateLight("Directional Light", LightType::Directional, { 0.f, 4.f, -2.f });
	sun->GetTransform()->SetRotation({ 0.75f, -0.45f, 0.f });
	shared_ptr<GameObject> point = CreateLight("Point Light", LightType::Point, { 0.f, 2.5f, -1.f });
	point->GetLight()->SetColor(Color(0.35f, 0.65f, 1.f, 1.f));
	point->GetLight()->SetIntensity(2.f);
	_scene->SetDirty(false);
}

void EditorApp::CreateThirdPersonDemo()
{
	StopPlay();
	_scene = SCENE->CreateScene("Third Person Demo");
	_editScene.reset();
	_selectedId = 0;

	shared_ptr<GameObject> ground = CreatePrimitive("Ground", L"Cube", { 0.f, -0.15f, 0.f });
	ground->GetTransform()->SetScale({ 12.f, 0.15f, 12.f });
	ground->GetMeshRenderer()->GetMaterial()->GetMaterialDesc().ambient = Vec4(0.35f, 0.38f, 0.4f, 1.f);
	ground->GetMeshRenderer()->GetMaterial()->GetMaterialDesc().diffuse = Vec4(0.42f, 0.48f, 0.42f, 1.f);

	CreatePrimitive("Demo Cube", L"Cube", { 3.f, 0.75f, 2.f });
	CreatePrimitive("Demo Sphere", L"Sphere", { -3.f, 1.f, 1.f });
	shared_ptr<GameObject> sun = CreateLight("Directional Light", LightType::Directional, { 0.f, 5.f, -3.f });
	sun->GetTransform()->SetRotation({ 0.75f, -0.45f, 0.f });
	shared_ptr<GameObject> point = CreateLight("Point Light", LightType::Point, { -2.f, 3.f, -1.f });
	point->GetLight()->SetColor(Color(0.35f, 0.65f, 1.f, 1.f));
	point->GetLight()->SetIntensity(3.f);
	point->GetLight()->SetRange(8.f);

	shared_ptr<GameObject> camera = _scene->CreateGameObject("Player Camera");
	camera->GetTransform()->SetPosition({ 0.f, 2.f, -5.5f });
	camera->AddComponent(make_shared<Camera>());

	shared_ptr<GameObject> player = _scene->CreateGameObject("Kachujin Player");
	player->GetTransform()->SetScale(Vec3(0.01f));
	shared_ptr<AnimatedModelRenderer> animator = make_shared<AnimatedModelRenderer>(_animationShader);
	animator->LoadModel(L"Kachujin/Kachujin", L"Kachujin/Kachujin", { L"Kachujin/Idle", L"Kachujin/Run", L"Kachujin/Slash" });
	player->AddComponent(animator);
	shared_ptr<PlayerController> controller = make_shared<PlayerController>();
	controller->SetCamera(camera);
	player->AddComponent(controller);
	_selectedId = player->GetId();
	_scene->SetDirty(true);
	_focusViewport = true;
	Log("Third Person Demo scene created. WASD move, LMB slash, RMB camera, Space jump.");
}

shared_ptr<GameObject> EditorApp::CreatePrimitive(const string& name, const wstring& meshName, const Vec3& position)
{
	shared_ptr<GameObject> object = _scene->CreateGameObject(name);
	object->GetTransform()->SetPosition(position);

	shared_ptr<MeshRenderer> renderer = make_shared<MeshRenderer>();
	renderer->SetMesh(RESOURCES->Get<Mesh>(meshName));
	shared_ptr<Material> material = make_shared<Material>();
	material->SetShader(_shader);
	material->GetMaterialDesc().ambient = Vec4(0.2f);
	material->GetMaterialDesc().diffuse = Vec4(0.72f, 0.78f, 0.88f, 1.f);
	material->GetMaterialDesc().specular = Vec4(1.f);
	renderer->SetMaterial(material);
	object->AddComponent(renderer);
	_selectedId = object->GetId();
	return object;
}

shared_ptr<GameObject> EditorApp::CreateLight(const string& name, LightType type, const Vec3& position)
{
	shared_ptr<GameObject> object = _scene->CreateGameObject(name);
	object->GetTransform()->SetPosition(position);
	shared_ptr<Light> light = make_shared<Light>();
	light->SetLightType(type);
	object->AddComponent(light);
	_selectedId = object->GetId();
	return object;
}

void EditorApp::Update()
{
	if (!_playing && _viewportHovered && ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGui::GetIO().WantCaptureKeyboard)
	{
		shared_ptr<Transform> transform = _editorCamera->GetTransform();
		Vec3 position = transform->GetPosition();
		const float speed = 5.f * DT;
		if (ImGui::IsKeyDown(ImGuiKey_W)) position += transform->GetLook() * speed;
		if (ImGui::IsKeyDown(ImGuiKey_S)) position -= transform->GetLook() * speed;
		if (ImGui::IsKeyDown(ImGuiKey_D)) position += transform->GetRight() * speed;
		if (ImGui::IsKeyDown(ImGuiKey_A)) position -= transform->GetRight() * speed;
		transform->SetPosition(position);

		ImVec2 delta = ImGui::GetIO().MouseDelta;
		Vec3 rotation = transform->GetLocalRotation();
		rotation.x += delta.y * 0.003f;
		rotation.y += delta.x * 0.003f;
		transform->SetLocalRotation(rotation);
	}

	if (_playing && !_paused)
	{
		for (const shared_ptr<GameObject>& object : _scene->GetGameObjects())
		{
			if (shared_ptr<PlayerController> player = object->GetPlayerController())
				player->SetInputEnabled(_viewportHovered);
		}
		_scene->Update();
		_scene->LateUpdate();
	}

	shared_ptr<Camera> sceneCamera;
	if (_playing)
	{
		for (const shared_ptr<GameObject>& object : _scene->GetGameObjects())
		{
			if (object->GetCamera())
			{
				sceneCamera = object->GetCamera();
				break;
			}
		}
	}
	if (sceneCamera) sceneCamera->Update(); else _editorCamera->Update();
	RENDER->Update();
	LightDesc light;
	_scene->BuildLightData(light);
	RENDER->PushLightData(light);
}

void EditorApp::Render()
{
	_viewportTarget->Bind();
	_scene->Render();
	_viewportTarget->Unbind();
}

void EditorApp::RenderUI()
{
	ImGuiID dockspaceId = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	if (_resetLayout)
		BuildDefaultLayout(dockspaceId);
	DrawMainMenu();
	DrawToolbar();
	if (_showHierarchy) DrawHierarchy();
	if (_showInspector) DrawInspector();
	if (_showViewport) DrawViewport();
	if (_showContentBrowser) DrawContentBrowser();
	if (_showOutputLog) DrawOutputLog();
	if (_showBlueprint) DrawBlueprintEditor();
	if (_focusViewport)
	{
		ImGui::SetWindowFocus("Viewport");
		_focusViewport = false;
	}
	if (_focusBlueprint)
	{
		ImGui::SetWindowFocus("Blueprint");
		_focusBlueprint = false;
	}
}

void EditorApp::BuildDefaultLayout(ImGuiID dockspaceId)
{
	_resetLayout = false;
	_focusViewport = true;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::DockBuilderRemoveNode(dockspaceId);
	ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

	ImGuiID center = dockspaceId;
	ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.18f, nullptr, &center);
	ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.22f, nullptr, &center);
	ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.27f, nullptr, &center);
	ImGuiID toolbar = ImGui::DockBuilderSplitNode(center, ImGuiDir_Up, 0.08f, nullptr, &center);

	ImGui::DockBuilderDockWindow("Toolbar", toolbar);
	ImGui::DockBuilderDockWindow("Hierarchy", left);
	ImGui::DockBuilderDockWindow("Inspector", right);
	ImGui::DockBuilderDockWindow("Blueprint", center);
	ImGui::DockBuilderDockWindow("Viewport", center);
	ImGui::DockBuilderDockWindow("Content Browser", bottom);
	ImGui::DockBuilderDockWindow("Output Log", bottom);
	ImGui::DockBuilderFinish(dockspaceId);
}

void EditorApp::DrawMainMenu()
{
	if (!ImGui::BeginMainMenuBar())
		return;
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New Scene", "Ctrl+N")) NewScene();
		if (ImGui::MenuItem("Open...", "Ctrl+O")) OpenScene();
		if (ImGui::MenuItem("Save", "Ctrl+S")) SaveScene();
		if (ImGui::MenuItem("Save As...")) SaveScene(true);
		ImGui::Separator();
		if (ImGui::MenuItem("Exit")) PostMessageW(GAME->GetGameDesc().hWnd, WM_CLOSE, 0, 0);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Undo", "Ctrl+Z", false, !_undoStack.empty())) Undo();
		if (ImGui::MenuItem("Redo", "Ctrl+Y", false, !_redoStack.empty())) Redo();
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Window"))
	{
		if (ImGui::MenuItem("Reset Layout")) _resetLayout = true;
		ImGui::Separator();
		ImGui::MenuItem("Hierarchy", nullptr, &_showHierarchy);
		ImGui::MenuItem("Inspector", nullptr, &_showInspector);
		ImGui::MenuItem("Viewport", nullptr, &_showViewport);
		ImGui::MenuItem("Content Browser", nullptr, &_showContentBrowser);
		ImGui::MenuItem("Output Log", nullptr, &_showOutputLog);
		ImGui::MenuItem("Blueprint", nullptr, &_showBlueprint);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Build"))
	{
		if (ImGui::MenuItem("Package Project")) PackageProject();
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Demo"))
	{
		if (ImGui::MenuItem("Create Third Person Demo")) CreateThirdPersonDemo();
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Help"))
	{
		ImGui::MenuItem("Core Craft Editor MVP");
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();

	ImGuiIO& io = ImGui::GetIO();
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) SaveScene();
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) Undo();
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y)) Redo();
}

void EditorApp::DrawToolbar()
{
	ImGui::SetNextWindowSizeConstraints({ 0.f, 42.f }, { FLT_MAX, 42.f });
	ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	if (ImGui::Button("Save")) SaveScene();
	ImGui::SameLine();
	if (ImGui::RadioButton("Move", _gizmoOperation == 0)) _gizmoOperation = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", _gizmoOperation == 1)) _gizmoOperation = 1;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", _gizmoOperation == 2)) _gizmoOperation = 2;
	ImGui::SameLine();
	if (ImGui::Button(_gizmoLocal ? "Local" : "World")) _gizmoLocal = !_gizmoLocal;
	ImGui::SameLine();
	ImGui::TextUnformatted("|");
	ImGui::SameLine();
	if (!_playing)
	{
		if (ImGui::Button("Play")) StartPlay();
	}
	else
	{
		if (ImGui::Button(_paused ? "Resume" : "Pause")) _paused = !_paused;
		ImGui::SameLine();
		if (ImGui::Button("Stop")) StopPlay();
	}
	ImGui::SameLine();
	if (ImGui::Button("Build")) PackageProject();
	ImGui::End();
}

void EditorApp::DrawHierarchy()
{
	ImGui::Begin("Hierarchy", &_showHierarchy);
	if (ImGui::Button("+ Cube"))
	{
		shared_ptr<GameObject> object = CreatePrimitive("Cube", L"Cube", Vec3::Zero);
		PushCommand({ [this, id = object->GetId()] { _scene->DestroyGameObject(id); }, [this, object] { _scene->AddGameObject(object); } });
	}
	ImGui::SameLine();
	if (ImGui::Button("+ Sphere"))
	{
		shared_ptr<GameObject> object = CreatePrimitive("Sphere", L"Sphere", Vec3::Zero);
		PushCommand({ [this, id = object->GetId()] { _scene->DestroyGameObject(id); }, [this, object] { _scene->AddGameObject(object); } });
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete")) DeleteSelected();
	if (ImGui::Button("+ Sun"))
	{
		shared_ptr<GameObject> object = CreateLight("Directional Light", LightType::Directional, { 0.f, 4.f, 0.f });
		PushCommand({ [this, id = object->GetId()] { _scene->DestroyGameObject(id); }, [this, object] { _scene->AddGameObject(object); } });
	}
	ImGui::SameLine();
	if (ImGui::Button("+ Point Light"))
	{
		shared_ptr<GameObject> object = CreateLight("Point Light", LightType::Point, { 0.f, 2.f, 0.f });
		PushCommand({ [this, id = object->GetId()] { _scene->DestroyGameObject(id); }, [this, object] { _scene->AddGameObject(object); } });
	}
	ImGui::Separator();
	for (const shared_ptr<GameObject>& root : _scene->GetRootObjects())
		DrawHierarchyNode(root);
	ImGui::End();
}

void EditorApp::DrawHierarchyNode(const shared_ptr<GameObject>& object)
{
	shared_ptr<Transform> transform = object->GetTransform();
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (transform == nullptr || transform->GetChildren().empty()) flags |= ImGuiTreeNodeFlags_Leaf;
	if (_selectedId == object->GetId()) flags |= ImGuiTreeNodeFlags_Selected;
	bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(object->GetId()), flags, "%s", object->GetName().c_str());
	if (ImGui::IsItemClicked()) _selectedId = object->GetId();
	if (open)
	{
		if (transform)
		{
			for (const shared_ptr<Transform>& child : transform->GetChildren())
				DrawHierarchyNode(child->GetGameObject());
		}
		ImGui::TreePop();
	}
}

void EditorApp::DrawInspector()
{
	ImGui::Begin("Inspector", &_showInspector);
	shared_ptr<GameObject> object = GetSelected();
	if (object == nullptr)
	{
		ImGui::TextDisabled("Select an Actor");
		ImGui::End();
		return;
	}

	char name[128] = {};
	strncpy_s(name, object->GetName().c_str(), _TRUNCATE);
	if (ImGui::InputText("Name", name, sizeof(name)))
	{
		object->SetName(name);
		_scene->SetDirty(true);
	}
	bool active = object->IsActive();
	if (ImGui::Checkbox("Active", &active))
	{
		object->SetActive(active);
		_scene->SetDirty(true);
	}

	ImGui::SeparatorText("Transform");
	shared_ptr<Transform> transform = object->GetTransform();
	Vec3 before;
	Vec3 value = transform->GetLocalPosition();
	static Vec3 editBefore;
	if (ImGui::DragFloat3("Location", &value.x, 0.05f)) transform->SetLocalPosition(value);
	if (ImGui::IsItemActivated()) editBefore = transform->GetLocalPosition();
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		before = editBefore;
		Vec3 after = transform->GetLocalPosition();
		weak_ptr<Transform> target = transform;
		PushCommand({ [target, before] { if (auto item = target.lock()) item->SetLocalPosition(before); }, [target, after] { if (auto item = target.lock()) item->SetLocalPosition(after); } });
		_scene->SetDirty(true);
	}

	value = transform->GetLocalRotation() * RadToDeg;
	if (ImGui::DragFloat3("Rotation", &value.x, 0.25f)) transform->SetLocalRotation(value * DegToRad);
	if (ImGui::IsItemActivated()) editBefore = transform->GetLocalRotation();
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		before = editBefore;
		Vec3 after = transform->GetLocalRotation();
		weak_ptr<Transform> target = transform;
		PushCommand({ [target, before] { if (auto item = target.lock()) item->SetLocalRotation(before); }, [target, after] { if (auto item = target.lock()) item->SetLocalRotation(after); } });
		_scene->SetDirty(true);
	}

	value = transform->GetLocalScale();
	if (ImGui::DragFloat3("Scale", &value.x, 0.02f, 0.001f, 1000.f)) transform->SetLocalScale(value);
	if (ImGui::IsItemActivated()) editBefore = transform->GetLocalScale();
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		before = editBefore;
		Vec3 after = transform->GetLocalScale();
		weak_ptr<Transform> target = transform;
		PushCommand({ [target, before] { if (auto item = target.lock()) item->SetLocalScale(before); }, [target, after] { if (auto item = target.lock()) item->SetLocalScale(after); } });
		_scene->SetDirty(true);
	}

	if (object->GetMeshRenderer())
	{
		ImGui::SeparatorText("Mesh Renderer");
		ImGui::Text("Mesh: %ls", object->GetMeshRenderer()->GetMesh()->GetName().c_str());
	}
	if (object->GetAnimatedModelRenderer())
	{
		ImGui::SeparatorText("Animated Model");
		ImGui::Text("Animation: %d", object->GetAnimatedModelRenderer()->GetAnimationIndex());
		ImGui::TextWrapped("Model: %ls", object->GetAnimatedModelRenderer()->GetModelPath().c_str());
	}
	if (shared_ptr<PlayerController> player = object->GetPlayerController())
	{
		ImGui::SeparatorText("Player Controller");
		float moveSpeed = player->GetMoveSpeed();
		if (ImGui::DragFloat("Move Speed", &moveSpeed, 0.05f, 0.f, 100.f)) { player->SetMoveSpeed(moveSpeed); _scene->SetDirty(true); }
		float jumpSpeed = player->GetJumpSpeed();
		if (ImGui::DragFloat("Jump Speed", &jumpSpeed, 0.05f, 0.f, 100.f)) { player->SetJumpSpeed(jumpSpeed); _scene->SetDirty(true); }
		float gravity = player->GetGravity();
		if (ImGui::DragFloat("Gravity", &gravity, 0.1f, -100.f, 0.f)) { player->SetGravity(gravity); _scene->SetDirty(true); }
		float cameraDistance = player->GetCameraDistance();
		if (ImGui::DragFloat("Camera Distance", &cameraDistance, 0.05f, 0.1f, 100.f)) { player->SetCameraDistance(cameraDistance); _scene->SetDirty(true); }
		float cameraHeight = player->GetCameraHeight();
		if (ImGui::DragFloat("Camera Height", &cameraHeight, 0.05f, -10.f, 20.f)) { player->SetCameraHeight(cameraHeight); _scene->SetDirty(true); }
	}
	if (shared_ptr<Light> light = object->GetLight())
	{
		ImGui::SeparatorText("Light");
		int type = light->GetLightType() == LightType::Directional ? 0 : 1;
		if (ImGui::Combo("Type", &type, "Directional\0Point\0"))
		{
			light->SetLightType(type == 0 ? LightType::Directional : LightType::Point);
			_scene->SetDirty(true);
		}
		Color color = light->GetColor();
		if (ImGui::ColorEdit3("Color", &color.x))
		{
			light->SetColor(color);
			_scene->SetDirty(true);
		}
		float intensity = light->GetIntensity();
		if (ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.f, 100.f))
		{
			light->SetIntensity(intensity);
			_scene->SetDirty(true);
		}
		if (light->GetLightType() == LightType::Point)
		{
			float range = light->GetRange();
			if (ImGui::DragFloat("Range", &range, 0.1f, 0.01f, 1000.f))
			{
				light->SetRange(range);
				_scene->SetDirty(true);
			}
		}
		else
		{
			ImGui::TextDisabled("Position controls the editor icon only.");
			ImGui::TextDisabled("Rotation controls the light direction.");
		}
	}
	if (object->GetBlueprint())
	{
		ImGui::SeparatorText("Blueprint");
		if (object->GetBlueprint()->GetAsset())
			ImGui::TextWrapped("%s", object->GetBlueprint()->GetAsset()->GetPath().generic_string().c_str());
		if (ImGui::Button("Open Blueprint")) _blueprint = object->GetBlueprint()->GetAsset();
	}
	else if (ImGui::Button("Add Blueprint"))
	{
		AttachBlueprint();
	}
	ImGui::End();
}

void EditorApp::DrawViewport()
{
	ImGui::Begin("Viewport", &_showViewport, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	_viewportPosition = ImGui::GetCursorScreenPos();
	_viewportSize = ImGui::GetContentRegionAvail();
	_viewportHovered = ImGui::IsWindowHovered();
	uint32 width = max(1u, static_cast<uint32>(_viewportSize.x));
	uint32 height = max(1u, static_cast<uint32>(_viewportSize.y));
	if (width != _viewportWidth || height != _viewportHeight)
	{
		_viewportWidth = width;
		_viewportHeight = height;
		_viewportTarget->Resize(width, height);
		_editorCamera->GetCamera()->SetViewportSize(static_cast<float>(width), static_cast<float>(height));
		if (_playing)
		{
			for (const shared_ptr<GameObject>& object : _scene->GetGameObjects())
				if (object->GetCamera()) object->GetCamera()->SetViewportSize(static_cast<float>(width), static_cast<float>(height));
		}
	}
	ImGui::Image(_viewportTarget->GetShaderResourceView(), _viewportSize);
	bool imageClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
	if (_viewportHovered && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		if (ImGui::IsKeyPressed(ImGuiKey_W)) _gizmoOperation = 0;
		if (ImGui::IsKeyPressed(ImGuiKey_E)) _gizmoOperation = 1;
		if (ImGui::IsKeyPressed(ImGuiKey_R)) _gizmoOperation = 2;
	}
	DrawViewportGrid();
	DrawLightIcons();
	DrawGizmo();
	if (imageClicked && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing())
		PickViewportObject(ImGui::GetIO().MousePos);
	ImGui::End();
}

void EditorApp::DrawViewportGrid()
{
	if (_playing)
		return;
	Matrix view = _editorCamera->GetCamera()->GetViewMatrix();
	Matrix projection = _editorCamera->GetCamera()->GetProjectionMatrix();
	Matrix identity = Matrix::Identity;
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(_viewportPosition.x, _viewportPosition.y, _viewportSize.x, _viewportSize.y);
	ImGuizmo::DrawGridCustomColor(
		&view._11, &projection._11, &identity._11,
		100.f, 5.f, 5,
		IM_COL32(105, 115, 135, 190),
		IM_COL32(65, 75, 92, 130),
		IM_COL32(155, 75, 75, 220));
}

void EditorApp::DrawLightIcons()
{
	if (_playing || _scene == nullptr)
		return;
	DirectX::SimpleMath::Viewport viewport(0.f, 0.f, _viewportSize.x, _viewportSize.y);
	Matrix view = _editorCamera->GetCamera()->GetViewMatrix();
	Matrix projection = _editorCamera->GetCamera()->GetProjectionMatrix();
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	for (const shared_ptr<GameObject>& object : _scene->GetGameObjects())
	{
		shared_ptr<Light> light = object->GetLight();
		if (light == nullptr || !object->IsActive())
			continue;
		Vec3 projected = viewport.Project(object->GetTransform()->GetPosition(), projection, view, Matrix::Identity);
		if (projected.z < 0.f || projected.z > 1.f)
			continue;
		ImVec2 center(_viewportPosition.x + projected.x, _viewportPosition.y + projected.y);
		ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(light->GetColor().x, light->GetColor().y, light->GetColor().z, 1.f));
		drawList->AddCircleFilled(center, 7.f, color);
		drawList->AddCircle(center, 9.f, IM_COL32(255, 255, 255, 220), 16, 2.f);
		if (light->GetLightType() == LightType::Directional)
		{
			Vec3 tipWorld = object->GetTransform()->GetPosition() + object->GetTransform()->GetLook() * 1.5f;
			Vec3 tip = viewport.Project(tipWorld, projection, view, Matrix::Identity);
			drawList->AddLine(center, ImVec2(_viewportPosition.x + tip.x, _viewportPosition.y + tip.y), color, 2.f);
		}
	}
}

void EditorApp::PickViewportObject(const ImVec2& mousePosition)
{
	if (_scene == nullptr || _playing)
		return;
	float localX = mousePosition.x - _viewportPosition.x;
	float localY = mousePosition.y - _viewportPosition.y;
	DirectX::SimpleMath::Viewport viewport(0.f, 0.f, _viewportSize.x, _viewportSize.y);
	Matrix view = _editorCamera->GetCamera()->GetViewMatrix();
	Matrix projection = _editorCamera->GetCamera()->GetProjectionMatrix();
	Vec3 nearPoint = viewport.Unproject({ localX, localY, 0.f }, projection, view, Matrix::Identity);
	Vec3 farPoint = viewport.Unproject({ localX, localY, 1.f }, projection, view, Matrix::Identity);
	Vec3 direction = farPoint - nearPoint;
	direction.Normalize();
	DirectX::SimpleMath::Ray ray(nearPoint, direction);
	float nearest = FLT_MAX;
	ObjectId picked = 0;
	for (const shared_ptr<GameObject>& object : _scene->GetGameObjects())
	{
		if (!object->IsActive() || (object->GetMeshRenderer() == nullptr && object->GetLight() == nullptr && object->GetModelAnimator() == nullptr))
			continue;
		Vec3 scale = object->GetTransform()->GetScale();
		float radius = 0.5f;
		if (object->GetLight() == nullptr)
		{
			radius = (std::max)(radius, fabsf(scale.x));
			radius = (std::max)(radius, fabsf(scale.y));
			radius = (std::max)(radius, fabsf(scale.z));
		}
		DirectX::BoundingSphere sphere(object->GetTransform()->GetPosition(), radius);
		float distance = 0.f;
		if (ray.Intersects(sphere, distance) && distance < nearest)
		{
			nearest = distance;
			picked = object->GetId();
		}
	}
	_selectedId = picked;
}

void EditorApp::DrawGizmo()
{
	shared_ptr<GameObject> object = GetSelected();
	if (object == nullptr || _playing)
		return;

	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(_viewportPosition.x, _viewportPosition.y, _viewportSize.x, _viewportSize.y);
	ImGuizmo::OPERATION operation = _gizmoOperation == 0 ? ImGuizmo::TRANSLATE : (_gizmoOperation == 1 ? ImGuizmo::ROTATE : ImGuizmo::SCALE);
	Matrix world = object->GetTransform()->GetWorldMatrix();
	Matrix view = _editorCamera->GetCamera()->GetViewMatrix();
	Matrix projection = _editorCamera->GetCamera()->GetProjectionMatrix();
	Vec3 beforePosition = object->GetTransform()->GetLocalPosition();
	Vec3 beforeRotation = object->GetTransform()->GetLocalRotation();
	Vec3 beforeScale = object->GetTransform()->GetLocalScale();
	if (ImGuizmo::Manipulate(&view._11, &projection._11, operation, _gizmoLocal ? ImGuizmo::LOCAL : ImGuizmo::WORLD, &world._11))
	{
		float translation[3], rotation[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(&world._11, translation, rotation, scale);
		object->GetTransform()->SetPosition({ translation[0], translation[1], translation[2] });
		object->GetTransform()->SetRotation({ rotation[0] * DegToRad, rotation[1] * DegToRad, rotation[2] * DegToRad });
		object->GetTransform()->SetScale({ scale[0], scale[1], scale[2] });
		_scene->SetDirty(true);
	}
	bool usingNow = ImGuizmo::IsUsing();
	if (usingNow && !_gizmoWasUsing)
	{
		_gizmoBeforePosition = beforePosition;
		_gizmoBeforeRotation = beforeRotation;
		_gizmoBeforeScale = beforeScale;
	}
	if (!usingNow && _gizmoWasUsing)
	{
		Vec3 afterPosition = object->GetTransform()->GetLocalPosition();
		Vec3 afterRotation = object->GetTransform()->GetLocalRotation();
		Vec3 afterScale = object->GetTransform()->GetLocalScale();
		weak_ptr<Transform> target = object->GetTransform();
		Vec3 oldPosition = _gizmoBeforePosition;
		Vec3 oldRotation = _gizmoBeforeRotation;
		Vec3 oldScale = _gizmoBeforeScale;
		PushCommand({
			[target, oldPosition, oldRotation, oldScale] { if (auto value = target.lock()) { value->SetLocalPosition(oldPosition); value->SetLocalRotation(oldRotation); value->SetLocalScale(oldScale); } },
			[target, afterPosition, afterRotation, afterScale] { if (auto value = target.lock()) { value->SetLocalPosition(afterPosition); value->SetLocalRotation(afterRotation); value->SetLocalScale(afterScale); } }
		});
	}
	_gizmoWasUsing = usingNow;
}

void EditorApp::DrawContentBrowser()
{
	ImGui::Begin("Content Browser", &_showContentBrowser);
	filesystem::path root = L"..\\Resources";
	if (filesystem::exists(root))
	{
		for (const filesystem::directory_entry& entry : filesystem::recursive_directory_iterator(root))
		{
			if (!entry.is_regular_file()) continue;
			string relative = entry.path().lexically_relative(root).generic_string();
			ImGui::Selectable(relative.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", entry.path().generic_string().c_str());
		}
	}
	ImGui::End();
}

void EditorApp::DrawOutputLog()
{
	ImGui::Begin("Output Log", &_showOutputLog);
	if (ImGui::Button("Clear")) _logs.clear();
	ImGui::Separator();
	for (const string& message : _logs) ImGui::TextUnformatted(message.c_str());
	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.f);
	ImGui::End();
}

void EditorApp::DrawBlueprintEditor()
{
	if (!ImGui::Begin("Blueprint", &_showBlueprint))
	{
		ImGui::End();
		return;
	}
	if (_blueprint == nullptr)
	{
		ImGui::TextDisabled("Select an Actor and add or open a Blueprint.");
		ImGui::End();
		return;
	}

	if (ImGui::Button("Compile"))
	{
		CompiledBlueprint compiled;
		vector<string> diagnostics;
		if (_blueprint->Compile(compiled, diagnostics)) Log("Blueprint compile succeeded.");
		else for (const string& diagnostic : diagnostics) Log(diagnostic);
	}
	ImGui::SameLine();
	if (ImGui::Button("Save Blueprint"))
	{
		string error;
		if (_blueprint->Save(_blueprint->GetPath(), &error)) Log("Blueprint saved."); else Log(error);
	}
	ImGui::SameLine();
	if (ImGui::Button("+ Set Location")) _blueprint->AddNode(BlueprintNodeType::SetLocation);
	ImGui::SameLine();
	if (ImGui::Button("+ Add Offset")) _blueprint->AddNode(BlueprintNodeType::AddOffset);
	ImGui::SameLine();
	if (ImGui::Button("+ Add Rotation")) _blueprint->AddNode(BlueprintNodeType::AddRotation);
	ImGui::SameLine();
	if (ImGui::Button("+ Print")) _blueprint->AddNode(BlueprintNodeType::PrintLog);
	ImGui::Separator();

	ed::SetCurrentEditor(_nodeEditor);
	ed::Begin("BlueprintCanvas");
	for (BlueprintNode& node : _blueprint->GetNodes())
	{
		ed::BeginNode(ed::NodeId(node.id));
		ImGui::PushStyleColor(ImGuiCol_Text, BlueprintNodeColor(node.type).Value);
		ImGui::TextUnformatted(BlueprintNodeName(node.type));
		ImGui::PopStyleColor();
		if (node.inputPin)
		{
			ed::BeginPin(ed::PinId(node.inputPin), ed::PinKind::Input);
			ImGui::Text("-> In");
			ed::EndPin();
		}
		if (node.type == BlueprintNodeType::SetLocation || node.type == BlueprintNodeType::AddOffset || node.type == BlueprintNodeType::AddRotation)
			ImGui::DragFloat3("Value", &node.vectorValue.x, 0.05f);
		if (node.type == BlueprintNodeType::PrintLog)
		{
			char text[128] = {};
			strncpy_s(text, node.textValue.c_str(), _TRUNCATE);
			if (ImGui::InputText("Message", text, sizeof(text))) node.textValue = text;
		}
		ed::BeginPin(ed::PinId(node.outputPin), ed::PinKind::Output);
		ImGui::Text("Out ->");
		ed::EndPin();
		ed::EndNode();
	}
	for (const BlueprintLink& link : _blueprint->GetLinks())
		ed::Link(ed::LinkId(link.id), ed::PinId(link.startPin), ed::PinId(link.endPin), ImColor(220, 220, 220), 2.f);

	if (ed::BeginCreate())
	{
		ed::PinId startPin, endPin;
		if (ed::QueryNewLink(&startPin, &endPin) && startPin && endPin && ed::AcceptNewItem())
			_blueprint->AddLink(startPin.Get(), endPin.Get());
	}
	ed::EndCreate();
	if (ed::BeginDelete())
	{
		ed::LinkId linkId;
		while (ed::QueryDeletedLink(&linkId))
		{
			if (ed::AcceptDeletedItem()) _blueprint->RemoveLink(linkId.Get());
		}
		ed::NodeId nodeId;
		while (ed::QueryDeletedNode(&nodeId))
		{
			if (ed::AcceptDeletedItem()) _blueprint->RemoveNode(nodeId.Get());
		}
	}
	ed::EndDelete();
	ed::End();
	ed::SetCurrentEditor(nullptr);
	ImGui::End();
}

void EditorApp::NewScene()
{
	StopPlay();
	_scene = SCENE->CreateScene("Untitled");
	_selectedId = 0;
	Log("New Scene created.");
}

filesystem::path EditorApp::SelectScenePath(bool save) const
{
	wchar_t file[MAX_PATH] = {};
	OPENFILENAMEW dialog = {};
	dialog.lStructSize = sizeof(dialog);
	dialog.hwndOwner = GAME->GetGameDesc().hWnd;
	dialog.lpstrFile = file;
	dialog.nMaxFile = MAX_PATH;
	dialog.lpstrFilter = L"Core Craft Scene (*.scene.xml)\0*.scene.xml\0All Files\0*.*\0";
	dialog.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
	if (save ? GetSaveFileNameW(&dialog) : GetOpenFileNameW(&dialog))
		return filesystem::path(file);
	return {};
}

void EditorApp::OpenScene()
{
	filesystem::path path = SelectScenePath(false);
	if (path.empty()) return;
	string error;
	shared_ptr<Scene> loaded = SceneSerializer::Load(path, _shader, &error);
	if (loaded)
	{
		_scene = loaded;
		SCENE->SetActiveScene(_scene);
		_selectedId = 0;
		Log("Scene opened: " + path.generic_string());
	}
	else Log(error);
}

void EditorApp::SaveScene(bool saveAs)
{
	filesystem::path path = saveAs || _scene->GetPath().empty() ? SelectScenePath(true) : _scene->GetPath();
	if (path.empty()) return;
	string error;
	if (SceneSerializer::Save(_scene, path, &error)) Log("Scene saved: " + path.generic_string()); else Log(error);
}

void EditorApp::StartPlay()
{
	if (_playing) return;
	filesystem::path snapshot = filesystem::temp_directory_path() / L"CoreCraftPlay.scene.xml";
	filesystem::path originalPath = _scene->GetPath();
	bool dirty = _scene->IsDirty();
	string error;
	if (!SceneSerializer::Save(_scene, snapshot, &error))
	{
		Log(error);
		return;
	}
	_scene->SetPath(originalPath);
	_scene->SetDirty(dirty);
	_editScene = _scene;
	_scene = SceneSerializer::Load(snapshot, _shader, &error);
	if (_scene == nullptr)
	{
		_scene = _editScene;
		_editScene.reset();
		Log(error);
		return;
	}
	SCENE->SetActiveScene(_scene);
	_selectedId = 0;
	_playing = true;
	_paused = false;
	Log("Play started.");
}

void EditorApp::StopPlay()
{
	if (!_playing) return;
	_scene = _editScene;
	_editScene.reset();
	SCENE->SetActiveScene(_scene);
	_selectedId = 0;
	_playing = false;
	_paused = false;
	Log("Play stopped. Edit Scene restored.");
}

void EditorApp::DeleteSelected()
{
	shared_ptr<GameObject> object = GetSelected();
	if (object == nullptr) return;
	ObjectId id = object->GetId();
	shared_ptr<Transform> parent = object->GetTransform()->GetParent();
	_scene->DestroyGameObject(id);
	PushCommand({ [this, object, parent] { _scene->AddGameObject(object); object->GetTransform()->SetParent(parent); }, [this, id] { _scene->DestroyGameObject(id); } });
	_selectedId = 0;
}

void EditorApp::AttachBlueprint()
{
	shared_ptr<GameObject> object = GetSelected();
	if (object == nullptr) return;
	_blueprint = make_shared<BlueprintAsset>();
	_blueprint->AddNode(BlueprintNodeType::BeginPlay, { 0.f, 0.f });
	_blueprint->AddNode(BlueprintNodeType::Tick, { 0.f, 180.f });
	filesystem::path path = filesystem::path(L"..\\Resources\\Blueprints") / (object->GetName() + ".blueprint.xml");
	_blueprint->SetPath(path);
	shared_ptr<BlueprintComponent> component = make_shared<BlueprintComponent>();
	component->SetAsset(_blueprint);
	object->AddComponent(component);
	_scene->SetDirty(true);
	_showBlueprint = true;
	_focusBlueprint = true;
	Log("Blueprint attached: " + path.generic_string());
}

void EditorApp::PackageProject()
{
	if (_scene->GetPath().empty() || _scene->IsDirty())
		SaveScene();
	if (_scene->GetPath().empty() || _scene->IsDirty())
	{
		Log("패키징 전에 Scene을 저장해야 합니다.");
		return;
	}

	filesystem::path script = filesystem::absolute(L"..\\Scripts\\PackageProject.ps1");
	filesystem::path scene = filesystem::absolute(_scene->GetPath());
	wstring command = L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" + script.wstring() + L"\" -Scene \"" + scene.wstring() + L"\"";
	vector<wchar_t> commandLine(command.begin(), command.end());
	commandLine.push_back(L'\0');

	STARTUPINFOW startup = {};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process = {};
	Log("Packaging started...");
	if (!CreateProcessW(nullptr, commandLine.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process))
	{
		Log("패키징 프로세스를 시작하지 못했습니다.");
		return;
	}
	WaitForSingleObject(process.hProcess, INFINITE);
	DWORD exitCode = 1;
	GetExitCodeProcess(process.hProcess, &exitCode);
	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);
	Log(exitCode == 0 ? "Package created: Builds/CoreCraft" : "Packaging failed. MSBuild 출력을 확인하세요.");
}

void EditorApp::PushCommand(EditorCommand command)
{
	_undoStack.push_back(move(command));
	_redoStack.clear();
}

void EditorApp::Undo()
{
	if (_undoStack.empty()) return;
	EditorCommand command = move(_undoStack.back());
	_undoStack.pop_back();
	command.undo();
	_redoStack.push_back(move(command));
	_scene->SetDirty(true);
}

void EditorApp::Redo()
{
	if (_redoStack.empty()) return;
	EditorCommand command = move(_redoStack.back());
	_redoStack.pop_back();
	command.redo();
	_undoStack.push_back(move(command));
	_scene->SetDirty(true);
}

void EditorApp::Log(const string& message)
{
	_logs.push_back(message);
	OutputDebugStringA((message + "\n").c_str());
}

shared_ptr<GameObject> EditorApp::GetSelected() const
{
	return _scene ? _scene->FindGameObject(_selectedId) : nullptr;
}

void EditorApp::Shutdown()
{
	StopPlay();
	if (_nodeEditor)
	{
		ed::DestroyEditor(_nodeEditor);
		_nodeEditor = nullptr;
	}
	_viewportTarget.reset();
}
