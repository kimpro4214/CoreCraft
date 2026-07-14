#pragma once

#include "GameObject.h"

struct LightDesc;

class Scene
{
public:
	explicit Scene(string name = "Untitled");

	shared_ptr<GameObject> CreateGameObject(const string& name = "GameObject");
	void AddGameObject(const shared_ptr<GameObject>& gameObject);
	bool DestroyGameObject(ObjectId id);
	void Clear();

	shared_ptr<GameObject> FindGameObject(ObjectId id) const;
	vector<shared_ptr<GameObject>> GetRootObjects() const;
	const vector<shared_ptr<GameObject>>& GetGameObjects() const { return _gameObjects; }

	void Awake();
	void Start();
	void Update();
	void LateUpdate();
	void Render();
	void BuildLightData(LightDesc& data) const;

	const string& GetName() const { return _name; }
	void SetName(const string& name) { _name = name; }
	const filesystem::path& GetPath() const { return _path; }
	void SetPath(const filesystem::path& path) { _path = path; }
	bool IsDirty() const { return _dirty; }
	void SetDirty(bool dirty) { _dirty = dirty; }

private:
	string _name;
	filesystem::path _path;
	vector<shared_ptr<GameObject>> _gameObjects;
	bool _awakened = false;
	bool _started = false;
	bool _dirty = false;
};

class SceneManager
{
	DECLARE_SINGLE(SceneManager);

public:
	shared_ptr<Scene> CreateScene(const string& name = "Untitled");
	void SetActiveScene(const shared_ptr<Scene>& scene) { _activeScene = scene; }
	shared_ptr<Scene> GetActiveScene() const { return _activeScene; }

private:
	shared_ptr<Scene> _activeScene;
};
