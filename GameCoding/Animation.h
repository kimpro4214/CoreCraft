#pragma once
#include "ResourceBase.h"

struct Keyframe
{
	Vec2 offset = Vec2{ 0.f, 0.f };
	Vec2 size = Vec2{0.f, 0.f};
	float time = 0.f;
};

class Texture;

class Animation : public ResourceBase
{
	using Super = ResourceBase;

public:
	Animation();
	virtual ~Animation();

	virtual void Load(const wstring& path) override;
	virtual void Save(const wstring& path) override;

	void SetLoop(bool loop) { _loop = loop; }
	bool IsLoop() { return _loop; }

	void SetTexture(shared_ptr<Texture> texture) { _texture = texture; }
	shared_ptr<Texture> GetTexture() { return _texture; }
	Vec2 GetTextureSize();

	const Keyframe& GetKeyframe(int32 index);
	int32 GetKeyframeCount();
	void AddKeyframe(const Keyframe& keyframe);

private:
	bool _loop = false;
	shared_ptr<Texture> _texture;
	vector<Keyframe> _keyframes;
};

