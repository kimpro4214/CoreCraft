#pragma once

#include "IExecute.h"

namespace ax::NodeEditor
{
	struct EditorContext;
}

class Scene;
class Shader;
class GameObject;
class RenderTexture;
class BlueprintAsset;
enum class LightType : uint8;

class EditorApp : public IExecute
{
public:
	virtual void Init() override;
	virtual void Update() override;
	virtual void Render() override;
	virtual void RenderUI() override;
	virtual void Shutdown() override;

private:
	struct EditorCommand
	{
		function<void()> undo;
		function<void()> redo;
	};

	void CreateDefaultScene();
	void CreateThirdPersonDemo();
	shared_ptr<GameObject> CreatePrimitive(const string& name, const wstring& meshName, const Vec3& position);
	shared_ptr<GameObject> CreateLight(const string& name, LightType type, const Vec3& position);
	void DrawMainMenu();
	void BuildDefaultLayout(ImGuiID dockspaceId);
	void DrawToolbar();
	void DrawHierarchy();
	void DrawHierarchyNode(const shared_ptr<GameObject>& object);
	void DrawInspector();
	void DrawViewport();
	void DrawContentBrowser();
	void DrawOutputLog();
	void DrawBlueprintEditor();
	void DrawGizmo();
	void DrawViewportGrid();
	void DrawLightIcons();
	void PickViewportObject(const ImVec2& mousePosition);

	void NewScene();
	void OpenScene();
	void SaveScene(bool saveAs = false);
	void StartPlay();
	void StopPlay();
	void PackageProject();
	void DeleteSelected();
	void AttachBlueprint();
	void PushCommand(EditorCommand command);
	void Undo();
	void Redo();
	void Log(const string& message);

	filesystem::path SelectScenePath(bool save) const;
	shared_ptr<GameObject> GetSelected() const;

	shared_ptr<Scene> _scene;
	shared_ptr<Scene> _editScene;
	shared_ptr<GameObject> _editorCamera;
	shared_ptr<Shader> _shader;
	shared_ptr<Shader> _animationShader;
	unique_ptr<RenderTexture> _viewportTarget;
	shared_ptr<BlueprintAsset> _blueprint;
	ax::NodeEditor::EditorContext* _nodeEditor = nullptr;

	ObjectId _selectedId = 0;
	uint32 _viewportWidth = 960;
	uint32 _viewportHeight = 540;
	ImVec2 _viewportPosition = {};
	ImVec2 _viewportSize = {};
	bool _viewportHovered = false;
	bool _playing = false;
	bool _paused = false;
	bool _showHierarchy = true;
	bool _showInspector = true;
	bool _showViewport = true;
	bool _showContentBrowser = true;
	bool _showOutputLog = true;
	bool _showBlueprint = true;
	bool _resetLayout = true;
	bool _focusViewport = true;
	bool _focusBlueprint = false;
	int _gizmoOperation = 0;
	bool _gizmoLocal = false;
	bool _gizmoWasUsing = false;
	Vec3 _gizmoBeforePosition = Vec3::Zero;
	Vec3 _gizmoBeforeRotation = Vec3::Zero;
	Vec3 _gizmoBeforeScale = Vec3::One;

	vector<EditorCommand> _undoStack;
	vector<EditorCommand> _redoStack;
	vector<string> _logs;
};
