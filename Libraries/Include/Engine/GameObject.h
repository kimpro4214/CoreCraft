#pragma once
#include "Component.h"
class MonoBehaviour;
class Transform;
class Camera;
class MeshRenderer;
class ModelRenderer;
class ModelAnimator;
class BlueprintComponent;
class Light;
class AnimatedModelRenderer;
class PlayerController;

using ObjectId = uint64;

class GameObject : public enable_shared_from_this<GameObject>
{
public:
	GameObject();
	~GameObject();

	void Awake();
	void Start();
	void Update();
	void LateUpdate();
	void FixedUpdate();
	void Render();

	ObjectId GetId() const { return _id; }
	void SetId(ObjectId id);
	const string& GetName() const { return _name; }
	void SetName(const string& name) { _name = name; }
	bool IsActive() const { return _active; }
	void SetActive(bool active) { _active = active; }
	void SetRenderDuringUpdate(bool enabled) { _renderDuringUpdate = enabled; }

	shared_ptr<Component> GetFixedComponent(ComponentType type);
	shared_ptr<Transform> GetTransform();
	shared_ptr<Camera> GetCamera();
	shared_ptr<MeshRenderer> GetMeshRenderer();
	shared_ptr<ModelRenderer> GetModelRenderer();
	shared_ptr<ModelAnimator> GetModelAnimator();
	shared_ptr<BlueprintComponent> GetBlueprint();
	shared_ptr<Light> GetLight();
	shared_ptr<AnimatedModelRenderer> GetAnimatedModelRenderer();
	shared_ptr<PlayerController> GetPlayerController();
	const array<shared_ptr<Component>, FIXED_COMPONENT_COUNT>& GetFixedComponents() const { return _components; }
	const vector<shared_ptr<MonoBehaviour>>& GetScripts() const { return _scripts; }

	shared_ptr<Transform> GetOrAddTransform();
	void AddComponent(shared_ptr<Component> component);

protected:
	ObjectId _id = 0;
	string _name = "GameObject";
	bool _active = true;
	bool _renderDuringUpdate = true;
	array<shared_ptr<Component>, FIXED_COMPONENT_COUNT> _components;
	vector<shared_ptr<MonoBehaviour>> _scripts;
};

