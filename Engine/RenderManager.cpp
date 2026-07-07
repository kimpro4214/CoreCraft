#include "pch.h"
#include "RenderManager.h"
#include "Camera.h"

void RenderManager::Init(shared_ptr<Shader> shader)
{
	_shader = shader;

	_globalBuffer = make_shared<ConstantBuffer<GlobalDesc>>();
	_globalBuffer->Create();
	_globalEffectBuffer = _shader->GetConstantBuffer("GlobalBuffer");

	_transformBuffer = make_shared<ConstantBuffer<TransformDesc>>();
	_transformBuffer->Create();
	_transformEffectBuffer = _shader->GetConstantBuffer("TransformBuffer");

	_lightBuffer = make_shared<ConstantBuffer<LightDesc>>();
	_lightBuffer->Create();
	_lightEffectBuffer = _shader->GetConstantBuffer("LightBuffer");

	_materialBuffer = make_shared<ConstantBuffer<MaterialDesc>>();
	_materialBuffer->Create();
	_materialEffectBuffer = _shader->GetConstantBuffer("MaterialBuffer");

	_boneBuffer = make_shared<ConstantBuffer<BoneDesc>>();
	_boneBuffer->Create();
	_boneEffectBuffer = _shader->GetConstantBuffer("BoneBuffer");

	_keyframeBuffer = make_shared<ConstantBuffer<KeyframeDesc>>();
	_keyframeBuffer->Create();
	_keyframeEffectBuffer = _shader->GetConstantBuffer("KeyframeBuffer");

	_tweenBuffer = make_shared<ConstantBuffer<TweenDesc>>();
	_tweenBuffer->Create();
	_tweenEffectBuffer = _shader->GetConstantBuffer("TweenBuffer");
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
	_globalEffectBuffer->SetConstantBuffer(_globalBuffer->GetComPtr().Get());
}

void RenderManager::PushTransformData(const TransformDesc& desc)
{
	_transformDesc = desc;
	_transformBuffer->CopyData(_transformDesc);
	_transformEffectBuffer->SetConstantBuffer(_transformBuffer->GetComPtr().Get());
}

void RenderManager::PushLightData(const LightDesc& desc)
{
	_lightDesc = desc;
	_lightBuffer->CopyData(_lightDesc);
	_lightEffectBuffer->SetConstantBuffer(_lightBuffer->GetComPtr().Get());
}

void RenderManager::PushMaterialData(const MaterialDesc& desc)
{
	_materialDesc = desc;
	_materialBuffer->CopyData(_materialDesc);
	_materialEffectBuffer->SetConstantBuffer(_materialBuffer->GetComPtr().Get());
}

void RenderManager::PushBoneData(const BoneDesc& desc)
{
	_boneDesc = desc;
	_boneBuffer->CopyData(_boneDesc);
	_boneEffectBuffer->SetConstantBuffer(_boneBuffer->GetComPtr().Get());
}

void RenderManager::PushKeyframeData(const KeyframeDesc& desc)
{
	_keyframeDesc = desc;
	_keyframeBuffer->CopyData(_keyframeDesc);
	_keyframeEffectBuffer->SetConstantBuffer(_keyframeBuffer->GetComPtr().Get());
}

void RenderManager::PushTweenData(const TweenDesc& desc)
{
	_tweenDesc = desc;
	_tweenBuffer->CopyData(_tweenDesc);
	_tweenEffectBuffer->SetConstantBuffer(_tweenBuffer->GetComPtr().Get());
}