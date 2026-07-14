#pragma once

#include "Component.h"

enum class BlueprintNodeType : uint8
{
	BeginPlay,
	Tick,
	SetLocation,
	AddOffset,
	AddRotation,
	PrintLog,
};

enum class BlueprintPinKind : uint8
{
	Input,
	Output,
};

struct BlueprintPin
{
	uint64 id = 0;
	uint64 nodeId = 0;
	BlueprintPinKind kind = BlueprintPinKind::Input;
};

struct BlueprintNode
{
	uint64 id = 0;
	BlueprintNodeType type = BlueprintNodeType::BeginPlay;
	Vec2 position = Vec2::Zero;
	Vec3 vectorValue = Vec3::Zero;
	string textValue;
	uint64 inputPin = 0;
	uint64 outputPin = 0;
};

struct BlueprintLink
{
	uint64 id = 0;
	uint64 startPin = 0;
	uint64 endPin = 0;
};

enum class BlueprintOperation : uint8
{
	SetLocation,
	AddOffset,
	AddRotation,
	PrintLog,
};

struct BlueprintInstruction
{
	BlueprintOperation operation = BlueprintOperation::AddOffset;
	Vec3 vectorValue = Vec3::Zero;
	string textValue;
};

struct CompiledBlueprint
{
	vector<BlueprintInstruction> beginPlay;
	vector<BlueprintInstruction> tick;
};

class BlueprintAsset
{
public:
	BlueprintNode& AddNode(BlueprintNodeType type, const Vec2& position = Vec2::Zero);
	bool AddLink(uint64 startPin, uint64 endPin);
	bool RemoveNode(uint64 nodeId);
	bool RemoveLink(uint64 linkId);

	bool Compile(CompiledBlueprint& output, vector<string>& diagnostics) const;
	bool Save(const filesystem::path& path, string* error = nullptr) const;
	static shared_ptr<BlueprintAsset> Load(const filesystem::path& path, string* error = nullptr);

	vector<BlueprintNode>& GetNodes() { return _nodes; }
	const vector<BlueprintNode>& GetNodes() const { return _nodes; }
	vector<BlueprintLink>& GetLinks() { return _links; }
	const vector<BlueprintLink>& GetLinks() const { return _links; }
	const filesystem::path& GetPath() const { return _path; }
	void SetPath(const filesystem::path& path) { _path = path; }

private:
	uint64 NextId();
	const BlueprintNode* FindNodeByPin(uint64 pinId) const;
	bool CompileEvent(BlueprintNodeType eventType, vector<BlueprintInstruction>& instructions, vector<string>& diagnostics) const;

	filesystem::path _path;
	vector<BlueprintNode> _nodes;
	vector<BlueprintLink> _links;
	uint64 _nextId = 1;
};

class BlueprintComponent : public Component
{
	using Super = Component;

public:
	BlueprintComponent();

	void SetAsset(const shared_ptr<BlueprintAsset>& asset);
	shared_ptr<BlueprintAsset> GetAsset() const { return _asset; }
	const vector<string>& GetDiagnostics() const { return _diagnostics; }
	bool Recompile();

	virtual void Awake() override;
	virtual void Update() override;

private:
	void Execute(const vector<BlueprintInstruction>& instructions, float deltaTime);

	shared_ptr<BlueprintAsset> _asset;
	CompiledBlueprint _compiled;
	vector<string> _diagnostics;
	bool _valid = false;
};
