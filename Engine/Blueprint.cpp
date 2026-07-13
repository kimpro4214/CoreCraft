#include "pch.h"
#include "Blueprint.h"
#include "GameObject.h"
#include "Transform.h"
#include "tinyxml2.h"

using namespace tinyxml2;

namespace
{
	const char* NodeTypeName(BlueprintNodeType type)
	{
		switch (type)
		{
		case BlueprintNodeType::BeginPlay: return "BeginPlay";
		case BlueprintNodeType::Tick: return "Tick";
		case BlueprintNodeType::SetLocation: return "SetLocation";
		case BlueprintNodeType::AddOffset: return "AddOffset";
		case BlueprintNodeType::AddRotation: return "AddRotation";
		case BlueprintNodeType::PrintLog: return "PrintLog";
		}
		return "Unknown";
	}

	BlueprintNodeType ParseNodeType(const char* name)
	{
		if (strcmp(name, "Tick") == 0) return BlueprintNodeType::Tick;
		if (strcmp(name, "SetLocation") == 0) return BlueprintNodeType::SetLocation;
		if (strcmp(name, "AddOffset") == 0) return BlueprintNodeType::AddOffset;
		if (strcmp(name, "AddRotation") == 0) return BlueprintNodeType::AddRotation;
		if (strcmp(name, "PrintLog") == 0) return BlueprintNodeType::PrintLog;
		return BlueprintNodeType::BeginPlay;
	}
}

uint64 BlueprintAsset::NextId()
{
	return _nextId++;
}

BlueprintNode& BlueprintAsset::AddNode(BlueprintNodeType type, const Vec2& position)
{
	BlueprintNode node;
	node.id = NextId();
	node.type = type;
	node.position = position;
	if (type != BlueprintNodeType::BeginPlay && type != BlueprintNodeType::Tick)
		node.inputPin = NextId();
	node.outputPin = NextId();
	_nodes.push_back(move(node));
	return _nodes.back();
}

bool BlueprintAsset::AddLink(uint64 startPin, uint64 endPin)
{
	if (startPin == 0 || endPin == 0 || startPin == endPin)
		return false;
	if (FindNodeByPin(startPin) == nullptr || FindNodeByPin(endPin) == nullptr)
		return false;
	for (const BlueprintLink& link : _links)
	{
		if (link.startPin == startPin || link.endPin == endPin)
			return false;
	}
	_links.push_back({ NextId(), startPin, endPin });
	return true;
}

bool BlueprintAsset::RemoveNode(uint64 nodeId)
{
	auto found = find_if(_nodes.begin(), _nodes.end(), [nodeId](const BlueprintNode& node) { return node.id == nodeId; });
	if (found == _nodes.end())
		return false;
	const uint64 inputPin = found->inputPin;
	const uint64 outputPin = found->outputPin;
	_links.erase(remove_if(_links.begin(), _links.end(), [inputPin, outputPin](const BlueprintLink& link)
	{
		return link.startPin == inputPin || link.startPin == outputPin || link.endPin == inputPin || link.endPin == outputPin;
	}), _links.end());
	_nodes.erase(found);
	return true;
}

bool BlueprintAsset::RemoveLink(uint64 linkId)
{
	auto oldSize = _links.size();
	_links.erase(remove_if(_links.begin(), _links.end(), [linkId](const BlueprintLink& link) { return link.id == linkId; }), _links.end());
	return oldSize != _links.size();
}

const BlueprintNode* BlueprintAsset::FindNodeByPin(uint64 pinId) const
{
	auto found = find_if(_nodes.begin(), _nodes.end(), [pinId](const BlueprintNode& node)
	{
		return node.inputPin == pinId || node.outputPin == pinId;
	});
	return found == _nodes.end() ? nullptr : &*found;
}

bool BlueprintAsset::Compile(CompiledBlueprint& output, vector<string>& diagnostics) const
{
	output = {};
	diagnostics.clear();
	bool beginResult = CompileEvent(BlueprintNodeType::BeginPlay, output.beginPlay, diagnostics);
	bool tickResult = CompileEvent(BlueprintNodeType::Tick, output.tick, diagnostics);
	return beginResult && tickResult;
}

bool BlueprintAsset::CompileEvent(BlueprintNodeType eventType, vector<BlueprintInstruction>& instructions, vector<string>& diagnostics) const
{
	auto event = find_if(_nodes.begin(), _nodes.end(), [eventType](const BlueprintNode& node) { return node.type == eventType; });
	if (event == _nodes.end())
		return true;

	unordered_map<uint64, bool> visited;
	const BlueprintNode* current = &*event;
	while (current)
	{
		if (visited[current->id])
		{
			diagnostics.push_back(string(NodeTypeName(eventType)) + " 실행 흐름에 순환이 있습니다.");
			return false;
		}
		visited[current->id] = true;

		if (current->type != BlueprintNodeType::BeginPlay && current->type != BlueprintNodeType::Tick)
		{
			BlueprintInstruction instruction;
			instruction.vectorValue = current->vectorValue;
			instruction.textValue = current->textValue;
			switch (current->type)
			{
			case BlueprintNodeType::SetLocation: instruction.operation = BlueprintOperation::SetLocation; break;
			case BlueprintNodeType::AddOffset: instruction.operation = BlueprintOperation::AddOffset; break;
			case BlueprintNodeType::AddRotation: instruction.operation = BlueprintOperation::AddRotation; break;
			case BlueprintNodeType::PrintLog: instruction.operation = BlueprintOperation::PrintLog; break;
			default: break;
			}
			instructions.push_back(move(instruction));
		}

		auto link = find_if(_links.begin(), _links.end(), [current](const BlueprintLink& candidate)
		{
			return candidate.startPin == current->outputPin;
		});
		current = link == _links.end() ? nullptr : FindNodeByPin(link->endPin);
	}
	return true;
}

bool BlueprintAsset::Save(const filesystem::path& path, string* error) const
{
	tinyxml2::XMLDocument document;
	document.InsertEndChild(document.NewDeclaration());
	XMLElement* root = document.NewElement("Blueprint");
	root->SetAttribute("version", 1);
	document.InsertEndChild(root);

	for (const BlueprintNode& node : _nodes)
	{
		XMLElement* element = document.NewElement("Node");
		element->SetAttribute("id", static_cast<int64_t>(node.id));
		element->SetAttribute("type", NodeTypeName(node.type));
		element->SetAttribute("x", node.position.x);
		element->SetAttribute("y", node.position.y);
		element->SetAttribute("vx", node.vectorValue.x);
		element->SetAttribute("vy", node.vectorValue.y);
		element->SetAttribute("vz", node.vectorValue.z);
		element->SetAttribute("text", node.textValue.c_str());
		element->SetAttribute("input", static_cast<int64_t>(node.inputPin));
		element->SetAttribute("output", static_cast<int64_t>(node.outputPin));
		root->InsertEndChild(element);
	}
	for (const BlueprintLink& link : _links)
	{
		XMLElement* element = document.NewElement("Link");
		element->SetAttribute("id", static_cast<int64_t>(link.id));
		element->SetAttribute("start", static_cast<int64_t>(link.startPin));
		element->SetAttribute("end", static_cast<int64_t>(link.endPin));
		root->InsertEndChild(element);
	}

	filesystem::create_directories(path.parent_path());
	if (document.SaveFile(path.string().c_str()) != XML_SUCCESS)
	{
		if (error) *error = "Blueprint 파일을 저장하지 못했습니다.";
		return false;
	}
	return true;
}

shared_ptr<BlueprintAsset> BlueprintAsset::Load(const filesystem::path& path, string* error)
{
	tinyxml2::XMLDocument document;
	if (document.LoadFile(path.string().c_str()) != XML_SUCCESS)
	{
		if (error) *error = "Blueprint 파일을 열지 못했습니다.";
		return nullptr;
	}
	XMLElement* root = document.FirstChildElement("Blueprint");
	if (root == nullptr || root->IntAttribute("version", 0) != 1)
	{
		if (error) *error = "지원하지 않는 Blueprint 형식입니다.";
		return nullptr;
	}

	shared_ptr<BlueprintAsset> asset = make_shared<BlueprintAsset>();
	asset->_path = path;
	for (XMLElement* element = root->FirstChildElement("Node"); element; element = element->NextSiblingElement("Node"))
	{
		BlueprintNode node;
		node.id = static_cast<uint64>(element->Int64Attribute("id"));
		node.type = ParseNodeType(element->Attribute("type") ? element->Attribute("type") : "BeginPlay");
		node.position = { element->FloatAttribute("x"), element->FloatAttribute("y") };
		node.vectorValue = { element->FloatAttribute("vx"), element->FloatAttribute("vy"), element->FloatAttribute("vz") };
		node.textValue = element->Attribute("text") ? element->Attribute("text") : "";
		node.inputPin = static_cast<uint64>(element->Int64Attribute("input"));
		node.outputPin = static_cast<uint64>(element->Int64Attribute("output"));
		asset->_nextId = max(asset->_nextId, max(node.id, max(node.inputPin, node.outputPin)) + 1);
		asset->_nodes.push_back(move(node));
	}
	for (XMLElement* element = root->FirstChildElement("Link"); element; element = element->NextSiblingElement("Link"))
	{
		BlueprintLink link;
		link.id = static_cast<uint64>(element->Int64Attribute("id"));
		link.startPin = static_cast<uint64>(element->Int64Attribute("start"));
		link.endPin = static_cast<uint64>(element->Int64Attribute("end"));
		asset->_nextId = max(asset->_nextId, link.id + 1);
		asset->_links.push_back(link);
	}
	return asset;
}

BlueprintComponent::BlueprintComponent()
	: Super(ComponentType::Blueprint)
{
}

void BlueprintComponent::SetAsset(const shared_ptr<BlueprintAsset>& asset)
{
	_asset = asset;
	Recompile();
}

bool BlueprintComponent::Recompile()
{
	_diagnostics.clear();
	_valid = _asset && _asset->Compile(_compiled, _diagnostics);
	return _valid;
}

void BlueprintComponent::Awake()
{
	if (_valid)
		Execute(_compiled.beginPlay, 0.f);
}

void BlueprintComponent::Update()
{
	if (_valid)
		Execute(_compiled.tick, DT);
}

void BlueprintComponent::Execute(const vector<BlueprintInstruction>& instructions, float deltaTime)
{
	shared_ptr<Transform> transform = GetTransform();
	for (const BlueprintInstruction& instruction : instructions)
	{
		switch (instruction.operation)
		{
		case BlueprintOperation::SetLocation:
			transform->SetLocalPosition(instruction.vectorValue);
			break;
		case BlueprintOperation::AddOffset:
			transform->SetLocalPosition(transform->GetLocalPosition() + instruction.vectorValue * deltaTime);
			break;
		case BlueprintOperation::AddRotation:
			transform->SetLocalRotation(transform->GetLocalRotation() + instruction.vectorValue * deltaTime);
			break;
		case BlueprintOperation::PrintLog:
			OutputDebugStringA((instruction.textValue + "\n").c_str());
			break;
		}
	}
}
