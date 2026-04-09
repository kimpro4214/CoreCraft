#include "pch.h"
#include "08. NormalDemo.h"
#include "GeometryHelper.h"
#include "Camera.h"
#include "GameObject.h"
#include "CameraScript.h"

void NormalDemo::Init()
{
	_shader = make_shared<Shader>(L"07. Normal.fx");

	// Object
	_geometry = make_shared<Geometry<VertexTextureNormalData>>();
	//GeometryHelper::CreateCube(_geometry);
	GeometryHelper::CreateSphere(_geometry);
	_vertexBuffer = make_shared<VertexBuffer>();
	_vertexBuffer->Create(_geometry->GetVertices());
	_indexBuffer = make_shared<IndexBuffer>();
	_indexBuffer->Create(_geometry->GetIndices());

	// Camera
	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform();
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(make_shared<CameraScript>());

	// Texture
	_texture = RESOURCES->Load<Texture>(L"Veigar", L"..\\Resources\\Textures\\veigar.jpg");

}

void NormalDemo::Update()
{
	_camera->Update();
}

void NormalDemo::Render()
{
	_shader->GetMatrix("World")->SetMatrix((float*)&_world);
	_shader->GetMatrix("View")->SetMatrix((float*)&Camera::S_MatView);
	_shader->GetMatrix("Projection")->SetMatrix((float*)&Camera::S_MatProjection);
	_shader->GetSRV("Texture0")->SetResource(_texture->GetComPtr().Get());
	_shader->GetVector("LightDir")->SetFloatVector((float*)&_lightDir);

	uint32 stride = _vertexBuffer->GetStride();
	uint32 offset = _vertexBuffer->GetOffset();

	DC->IASetVertexBuffers(0, 1, _vertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
	DC->IASetIndexBuffer(_indexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);

	_shader->DrawIndexed(0, 0, _indexBuffer->GetCount(), 0, 0);
}
