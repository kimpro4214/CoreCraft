#include "pch.h"
#include "RenderManager.h"
#include "Camera.h"

void RenderManager::Init(shared_ptr<Shader> shader)
{
	_bindings.clear();
	_globalBuffer = make_shared<ConstantBuffer<GlobalDesc>>();
	_globalBuffer->Create();

	_transformBuffer = make_shared<ConstantBuffer<TransformDesc>>();
	_transformBuffer->Create();

	_lightBuffer = make_shared<ConstantBuffer<LightDesc>>();
	_lightBuffer->Create();

	_materialBuffer = make_shared<ConstantBuffer<MaterialDesc>>();
	_materialBuffer->Create();

	_boneBuffer = make_shared<ConstantBuffer<BoneDesc>>();
	_boneBuffer->Create();

	_keyframeBuffer = make_shared<ConstantBuffer<KeyframeDesc>>();
	_keyframeBuffer->Create();

	_tweenBuffer = make_shared<ConstantBuffer<TweenDesc>>();
	_tweenBuffer->Create();
	RegisterShader(shader);
}

void RenderManager::RegisterShader(const shared_ptr<Shader>& shader)
{
	if (shader == nullptr)
		return;
	for (const ShaderBindings& binding : _bindings)
	{
		if (binding.shader == shader)
			return;
	}

	ShaderBindings binding;
	binding.shader = shader;
	binding.global = shader->GetConstantBuffer("GlobalBuffer");
	binding.transform = shader->GetConstantBuffer("TransformBuffer");
	binding.light = shader->GetConstantBuffer("LightBuffer");
	binding.material = shader->GetConstantBuffer("MaterialBuffer");
	binding.bone = shader->GetConstantBuffer("BoneBuffer");
	binding.keyframe = shader->GetConstantBuffer("KeyframeBuffer");
	binding.tween = shader->GetConstantBuffer("TweenBuffer");
	_bindings.push_back(move(binding));
}

void RenderManager::Update()
{
	PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);
}

void RenderManager::PushGlobalData(const Matrix& view, const Matrix& projection)
{
	_globalDesc.V = view; 
	_globalDesc.P = projection;
	_globalDesc.VP = view * projection;
	_globalDesc.VInv = view.Invert();
	_globalBuffer->CopyData(_globalDesc);
	for (ShaderBindings& binding : _bindings)
		binding.global->SetConstantBuffer(_globalBuffer->GetComPtr().Get());
}

void RenderManager::PushTransformData(const TransformDesc& desc)
{
	_transformDesc = desc;
	_transformBuffer->CopyData(_transformDesc);
	for (ShaderBindings& binding : _bindings)
		binding.transform->SetConstantBuffer(_transformBuffer->GetComPtr().Get());
}

void RenderManager::PushLightData(const LightDesc& desc)
{
	_lightDesc = desc;
	_lightBuffer->CopyData(_lightDesc);
	for (ShaderBindings& binding : _bindings)
		binding.light->SetConstantBuffer(_lightBuffer->GetComPtr().Get());
}

void RenderManager::PushMaterialData(const MaterialDesc& desc)
{
	_materialDesc = desc;
	_materialBuffer->CopyData(_materialDesc);
	for (ShaderBindings& binding : _bindings)
		binding.material->SetConstantBuffer(_materialBuffer->GetComPtr().Get());
}

void RenderManager::PushBoneData(const BoneDesc& desc)
{
	_boneDesc = desc;
	_boneBuffer->CopyData(_boneDesc);
	for (ShaderBindings& binding : _bindings)
		binding.bone->SetConstantBuffer(_boneBuffer->GetComPtr().Get());
}

void RenderManager::PushKeyframeData(const KeyframeDesc& desc)
{
	_keyframeDesc = desc;
	_keyframeBuffer->CopyData(_keyframeDesc);
	for (ShaderBindings& binding : _bindings)
		binding.keyframe->SetConstantBuffer(_keyframeBuffer->GetComPtr().Get());
}

void RenderManager::PushTweenData(const TweenDesc& desc)
{
	_tweenDesc = desc;
	_tweenBuffer->CopyData(_tweenDesc);
	for (ShaderBindings& binding : _bindings)
		binding.tween->SetConstantBuffer(_tweenBuffer->GetComPtr().Get());
}
