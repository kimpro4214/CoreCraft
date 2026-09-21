#include "pch.h"
#include "UStaticMeshComponent.h"
#include "Material.h"
#include "ModelMesh.h"
#include "Model.h"

UStaticMeshComponent::UStaticMeshComponent()
{
}

UStaticMeshComponent::~UStaticMeshComponent()
{
}

void UStaticMeshComponent::SetStaticMesh(const shared_ptr<Model>& model)
{
	_model = model;

	if (_model && _shader)
	{
		for (const shared_ptr<Material>& material : _model->GetMaterials())
			material->SetShader(_shader);
	}
}

void UStaticMeshComponent::SetShader(const shared_ptr<Shader>& shader)
{
	_shader = shader;

	if (_model && _shader)
	{
		for (const shared_ptr<Material>& material : _model->GetMaterials())
			material->SetShader(_shader);
	}
}

void UStaticMeshComponent::Render()
{
	if (_model == nullptr || _shader == nullptr)
		return;

	// Bones
	BoneDesc boneDesc;

	const uint32 boneCount = _model->GetBoneCount();
	for (uint32 i = 0; i < boneCount; i++)
	{
		shared_ptr<ModelBone> bone = _model->GetBoneByIndex(i);
		boneDesc.transforms[i] = bone->transform;
	}
	RENDER->PushBoneData(boneDesc);

	// Transform
	RENDER->PushTransformData(TransformDesc{ GetWorldMatrix() });

	const auto& meshes = _model->GetMeshes();
	for (auto& mesh : meshes)
	{
		if (mesh->material)
			mesh->material->Update();

		// BoneIndex
		_shader->GetScalar("BoneIndex")->SetInt(mesh->boneIndex);

		uint32 stride = mesh->vertexBuffer->GetStride();
		uint32 offset = mesh->vertexBuffer->GetOffset();

		DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		DC->IASetVertexBuffers(0, 1, mesh->vertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
		DC->IASetIndexBuffer(mesh->indexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);

		_shader->DrawIndexed(0, _pass, mesh->indexBuffer->GetCount(), 0, 0);
	}
}

uint32 UStaticMeshComponent::GetNumMaterials() const
{
	return _model ? static_cast<uint32>(_model->GetMeshCount()) : 0;
}

shared_ptr<Material> UStaticMeshComponent::GetMaterial(uint32 index) const
{
	if (!_model)
		return nullptr;

	shared_ptr<ModelMesh> mesh = _model->GetMeshByIndex(index);
	return mesh ? mesh->material : nullptr;
}

void UStaticMeshComponent::SetMaterial(uint32 index, const shared_ptr<Material>& material)
{
	if (!_model)
		return;

	shared_ptr<ModelMesh> mesh = _model->GetMeshByIndex(index);
	if (mesh)
		mesh->material = material;
}
