#include "pch.h"
#include "03. ConstBufferDemo.h"
#include "GeometryHelper.h"

void ConstBufferDemo::Init()
{
	_shader = make_shared<Shader>(L"03. ConstBuffer.fx");

	_geometry = make_shared<Geometry<VertexColorData>>();
	GeometryHelper::CreateQuad(_geometry, Color(0.f, 1.f, 0.f, 1.f));

	_vertexBuffer = make_shared<VertexBuffer>();
	_vertexBuffer->Create(_geometry->GetVertices());

	_indexBuffer = make_shared<IndexBuffer>();
	_indexBuffer->Create(_geometry->GetIndices());
}

void ConstBufferDemo::Update()
{
	float dt = TIME->GetDeltaTime();

	if (INPUT->GetButton(KEY_TYPE::A))
	{
		_translation.x -= 3.f * dt;
	}
	else if (INPUT->GetButton(KEY_TYPE::D))
	{
		_translation.x += 3.f * dt;
	}
	else if (INPUT->GetButton(KEY_TYPE::W))
	{
		_translation.y += 3.f * dt;
	}
	else if (INPUT->GetButton(KEY_TYPE::S))
	{
		_translation.y -= 3.f * dt;
	}

	// SRT
	_world = Matrix::CreateTranslation(_translation);
}

void ConstBufferDemo::Render()
{
	_shader->GetMatrix("World")->SetMatrix((float*)&_world);
	_shader->GetMatrix("View")->SetMatrix((float*)&_view);
	_shader->GetMatrix("Projection")->SetMatrix((float*)&_projection);

	uint32 stride = _vertexBuffer->GetStride();
	uint32 offset = _vertexBuffer->GetOffset();

	DC->IASetVertexBuffers(0, 1, _vertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
	DC->IASetIndexBuffer(_indexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);

	_shader->DrawIndexed(0, 0, _indexBuffer->GetCount(), 0, 0);
}
