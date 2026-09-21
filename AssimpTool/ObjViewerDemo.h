#pragma once
#include "IExecute.h"

class GameObject;
class Model;
class Shader;
class AActor;
class UWorld;
class UActorComponent;

class ObjViewerDemo : public IExecute
{
public:
	virtual void Init() override;
	virtual void Update() override;
	virtual void Render() override;
	virtual void RenderUI() override;

private:
	shared_ptr<AActor> ImportAsset(const wstring& assetFile);
	shared_ptr<AActor> CreateStaticMeshActor(const shared_ptr<Model>& model, const string& name);

	void DrawOutliner();
	void DrawOutlinerNode(const shared_ptr<AActor>& actor);
	void DrawInspector();
	void DrawImportPanel();

private:
	shared_ptr<UWorld> _world;
	shared_ptr<Shader> _shader;
	shared_ptr<GameObject> _camera;

	vector<pair<wstring, shared_ptr<Model>>> _availableModels;

	UObjectId _selectedId = 0;
	char _importPathBuffer[260] = "Tower/Tower.fbx";
	string _importStatus;
};
