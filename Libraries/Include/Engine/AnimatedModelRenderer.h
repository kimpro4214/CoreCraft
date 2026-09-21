#pragma once
#include "Component.h"

class Shader;
class Model;

struct AnimatedTransformPage
{
	using TransformArray = array<Matrix, MAX_MODEL_TRANSFORMS>;
	array<TransformArray, MAX_MODEL_KEYFRAMES> transforms;
};

class AnimatedModelRenderer : public Component
{
	using Super = Component;

public:
	explicit AnimatedModelRenderer(const shared_ptr<Shader>& shader);
	virtual void Update() override;
	virtual void Render() override;

	void SetModel(const shared_ptr<Model>& model);
	bool LoadModel(const wstring& modelPath, const wstring& materialPath, const vector<wstring>& animationPaths);
	shared_ptr<Model> GetModel() const { return _model; }
	const wstring& GetModelPath() const { return _modelPath; }
	const wstring& GetMaterialPath() const { return _materialPath; }
	const vector<wstring>& GetAnimationPaths() const { return _animationPaths; }
	void PlayAnimation(int32 index, float blendDuration = 0.15f);
	int32 GetAnimationIndex() const { return _tween.curr.animIndex; }

private:
	void AdvanceFrame(KeyframeDesc& frame, float deltaTime);
	bool CreateAnimationTexture();
	void CreateAnimationTransform(uint32 index);

	shared_ptr<Shader> _shader;
	shared_ptr<Model> _model;
	TweenDesc _tween;
	vector<AnimatedTransformPage> _animationTransforms;
	ComPtr<ID3D11Texture2D> _texture;
	ComPtr<ID3D11ShaderResourceView> _shaderResourceView;
	bool _animationTextureAttempted = false;
	wstring _modelPath;
	wstring _materialPath;
	vector<wstring> _animationPaths;
};
