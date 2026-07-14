#include "pch.h"
#include "AnimatedModelRenderer.h"
#include "Model.h"
#include "ModelAnimation.h"
#include "ModelMesh.h"
#include "Material.h"
#include "Transform.h"
#include "Shader.h"

AnimatedModelRenderer::AnimatedModelRenderer(const shared_ptr<Shader>& shader)
	: Super(ComponentType::AnimatedRenderer), _shader(shader)
{
}

void AnimatedModelRenderer::SetModel(const shared_ptr<Model>& model)
{
	_model = model;
	_texture.Reset();
	_shaderResourceView.Reset();
	_animationTextureAttempted = false;
	_animationTransforms.clear();
	if (_model == nullptr)
		return;
	for (const shared_ptr<Material>& material : _model->GetMaterials())
		material->SetShader(_shader);
	RENDER->RegisterShader(_shader);
}

bool AnimatedModelRenderer::LoadModel(const wstring& modelPath, const wstring& materialPath, const vector<wstring>& animationPaths)
{
	shared_ptr<Model> model = make_shared<Model>();
	model->ReadModel(modelPath);
	model->ReadMaterial(materialPath);
	for (const wstring& animationPath : animationPaths)
		model->ReadAnimation(animationPath);
	_modelPath = modelPath;
	_materialPath = materialPath;
	_animationPaths = animationPaths;
	SetModel(model);
	return _model->GetMeshCount() > 0;
}

void AnimatedModelRenderer::PlayAnimation(int32 index, float blendDuration)
{
	if (_model == nullptr || index < 0 || static_cast<uint32>(index) >= _model->GetAnimationCount())
		return;
	if (_tween.curr.animIndex == index || _tween.next.animIndex == index)
		return;

	if (blendDuration <= 0.f)
	{
		_tween.curr = KeyframeDesc{};
		_tween.curr.animIndex = index;
		_tween.ClearNextAnim();
		return;
	}

	_tween.ClearNextAnim();
	_tween.tweenDuration = blendDuration;
	_tween.next = KeyframeDesc{};
	_tween.next.animIndex = index;
}

void AnimatedModelRenderer::AdvanceFrame(KeyframeDesc& frame, float deltaTime)
{
	shared_ptr<ModelAnimation> animation = _model->GetAnimationByIndex(frame.animIndex);
	if (animation == nullptr || animation->frameCount == 0 || animation->frameRate <= 0.f)
		return;
	float timePerFrame = 1.f / (animation->frameRate * (std::max)(frame.speed, 0.01f));
	frame.sumTime += (std::min)(deltaTime, 0.1f);
	if (!std::isfinite(frame.sumTime))
		frame.sumTime = 0.f;
	if (frame.sumTime >= timePerFrame)
	{
		const double advance = floor(static_cast<double>(frame.sumTime) / timePerFrame);
		const uint32 frameAdvance = static_cast<uint32>(fmod(advance, animation->frameCount));
		frame.sumTime = fmod(frame.sumTime, timePerFrame);
		frame.currFrame = (frame.currFrame + frameAdvance) % animation->frameCount;
		frame.nextFrame = (frame.currFrame + 1) % animation->frameCount;
	}
	frame.ratio = frame.sumTime / timePerFrame;
}

void AnimatedModelRenderer::Update()
{
	if (_model == nullptr || _model->GetAnimationCount() == 0)
		return;
	if (!_animationTextureAttempted)
		CreateAnimationTexture();

	AdvanceFrame(_tween.curr, DT);
	if (_tween.next.animIndex < 0)
		return;
	AdvanceFrame(_tween.next, DT);
	_tween.tweenSumTime += DT;
	_tween.tweenRatio = _tween.tweenSumTime / (std::max)(_tween.tweenDuration, 0.001f);
	if (_tween.tweenRatio >= 1.f)
	{
		_tween.curr = _tween.next;
		_tween.ClearNextAnim();
	}
}

void AnimatedModelRenderer::Render()
{
	if (_model == nullptr)
		return;
	if (!_animationTextureAttempted)
		CreateAnimationTexture();
	if (_shaderResourceView == nullptr)
		return;
	RENDER->PushTweenData(_tween);
	_shader->GetSRV("TransformMap")->SetResource(_shaderResourceView.Get());

	BoneDesc bones;
	uint32 boneCount = (std::min)(_model->GetBoneCount(), static_cast<uint32>(MAX_MODEL_TRANSFORMS));
	for (uint32 index = 0; index < boneCount; ++index)
		bones.transforms[index] = _model->GetBoneByIndex(index)->transform;
	RENDER->PushBoneData(bones);
	RENDER->PushTransformData(TransformDesc{ GetTransform()->GetWorldMatrix() });

	for (const shared_ptr<ModelMesh>& mesh : _model->GetMeshes())
	{
		if (mesh->material)
			mesh->material->Update();
		_shader->GetScalar("BoneIndex")->SetInt(mesh->boneIndex);
		uint32 stride = mesh->vertexBuffer->GetStride();
		uint32 offset = mesh->vertexBuffer->GetOffset();
		DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		DC->IASetVertexBuffers(0, 1, mesh->vertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
		DC->IASetIndexBuffer(mesh->indexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);
		_shader->DrawIndexed(0, 0, mesh->indexBuffer->GetCount(), 0, 0);
	}
}

bool AnimatedModelRenderer::CreateAnimationTexture()
{
	_animationTextureAttempted = true;
	uint32 animationCount = _model->GetAnimationCount();
	if (animationCount == 0)
		return false;
	_animationTransforms.resize(animationCount);
	for (uint32 index = 0; index < animationCount; ++index)
		CreateAnimationTransform(index);

	D3D11_TEXTURE2D_DESC description = {};
	description.Width = MAX_MODEL_TRANSFORMS * 4;
	description.Height = MAX_MODEL_KEYFRAMES;
	description.ArraySize = animationCount;
	description.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	description.Usage = D3D11_USAGE_IMMUTABLE;
	description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	description.MipLevels = 1;
	description.SampleDesc.Count = 1;

	const uint32 rowSize = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
	const uint32 pageSize = rowSize * MAX_MODEL_KEYFRAMES;
	vector<BYTE> pixels(static_cast<size_t>(pageSize) * animationCount);
	for (uint32 index = 0; index < animationCount; ++index)
		memcpy(pixels.data() + static_cast<size_t>(index) * pageSize, _animationTransforms[index].transforms.data(), pageSize);

	vector<D3D11_SUBRESOURCE_DATA> resources(animationCount);
	for (uint32 index = 0; index < animationCount; ++index)
	{
		resources[index].pSysMem = pixels.data() + static_cast<size_t>(index) * pageSize;
		resources[index].SysMemPitch = rowSize;
		resources[index].SysMemSlicePitch = pageSize;
	}
	HRESULT result = DEVICE->CreateTexture2D(&description, resources.data(), _texture.GetAddressOf());
	if (FAILED(result))
	{
		OutputDebugStringA("AnimatedModelRenderer: animation texture creation failed.\n");
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC view = {};
	view.Format = description.Format;
	view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	view.Texture2DArray.MipLevels = 1;
	view.Texture2DArray.ArraySize = animationCount;
	result = DEVICE->CreateShaderResourceView(_texture.Get(), &view, _shaderResourceView.GetAddressOf());
	if (FAILED(result))
	{
		_texture.Reset();
		OutputDebugStringA("AnimatedModelRenderer: animation texture SRV creation failed.\n");
		return false;
	}
	return true;
}

void AnimatedModelRenderer::CreateAnimationTransform(uint32 index)
{
	vector<Matrix> parentTransforms(MAX_MODEL_TRANSFORMS, Matrix::Identity);
	shared_ptr<ModelAnimation> animation = _model->GetAnimationByIndex(index);
	if (animation == nullptr)
		continue;
	uint32 frameCount = (std::min)(animation->frameCount, static_cast<uint32>(MAX_MODEL_KEYFRAMES));
	uint32 boneCount = (std::min)(_model->GetBoneCount(), static_cast<uint32>(MAX_MODEL_TRANSFORMS));
	for (uint32 frameIndex = 0; frameIndex < frameCount; ++frameIndex)
	{
		for (uint32 boneIndex = 0; boneIndex < boneCount; ++boneIndex)
		{
			shared_ptr<ModelBone> bone = _model->GetBoneByIndex(boneIndex);
			Matrix animationTransform = Matrix::Identity;
			shared_ptr<ModelKeyframe> keyframe = animation->GetKeyframe(bone->name);
			if (keyframe && frameIndex < keyframe->transforms.size())
			{
				const ModelKeyframeData& value = keyframe->transforms[frameIndex];
				animationTransform = Matrix::CreateScale(value.scale) * Matrix::CreateFromQuaternion(value.rotation) * Matrix::CreateTranslation(value.translation);
			}
			Matrix parent = bone->parentIndex >= 0 ? parentTransforms[bone->parentIndex] : Matrix::Identity;
			parentTransforms[boneIndex] = animationTransform * parent;
			_animationTransforms[index].transforms[frameIndex][boneIndex] = bone->transform.Invert() * parentTransforms[boneIndex];
		}
	}
}
