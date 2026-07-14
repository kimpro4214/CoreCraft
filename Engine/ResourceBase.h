#pragma once

enum class ResourceType : uint8
{
	None,
	Mesh,
	Shader,
	Texture,
	Material,
	Animation,

	End
};

enum
{
	RESOURCE_TYPE_COUNT = static_cast<uint8>(ResourceType::End)
};

class ResourceBase : public enable_shared_from_this<ResourceBase>
{
public:
	ResourceBase(ResourceType type);
	virtual ~ResourceBase();

	ResourceType GetType() { return _type; }

	void SetName(const wstring& name) { _name = name; }
	const wstring& GetName() { return _name; }
	void SetPath(const wstring& path) { _path = path; }
	const wstring& GetPath() const { return _path; }
	uint32 GetID() { return _id; }

protected:
	virtual void Load(const wstring& path) { }
	virtual void Save(const wstring& path) { }

protected:
	ResourceType _type = ResourceType::None;
	wstring _name;
	wstring _path;
	uint32 _id = 0;
};

