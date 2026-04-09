#include "pch.h"
#include "04. CameraDemo.h"
#include "GeometryHelper.h"
#include "Camera.h"
#include "GameObject.h"
#include "CameraScript.h"

void CameraDemo::Init()
{
	_shader = make_shared<Shader>(L"03. ConstBuffer.fx");

	// Object
	_geometry = make_shared<Geometry<VertexColorData>>();
	GeometryHelper::CreateQuad(_geometry, Color(0.f, 1.f, 0.f, 1.f));
	_vertexBuffer = make_shared<VertexBuffer>();
	_vertexBuffer->Create(_geometry->GetVertices());
	_indexBuffer = make_shared<IndexBuffer>();
	_indexBuffer->Create(_geometry->GetIndices());

	// Camera
	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform();
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(make_shared<CameraScript>());

	_camera->GetTransform()->SetPosition(Vec3(0.f, 0.f, -2.f));
}

void CameraDemo::Update()
{
	_camera->Update();
}

void CameraDemo::Render()
{
	_shader->GetMatrix("World")->SetMatrix((float*)&_world);
	_shader->GetMatrix("View")->SetMatrix((float*)&Camera::S_MatView);
	_shader->GetMatrix("Projection")->SetMatrix((float*)&Camera::S_MatProjection);

	uint32 stride = _vertexBuffer->GetStride();
	uint32 offset = _vertexBuffer->GetOffset();

	DC->IASetVertexBuffers(0, 1, _vertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
	DC->IASetIndexBuffer(_indexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);

	_shader->DrawIndexed(0, 0, _indexBuffer->GetCount(), 0, 0);
}
