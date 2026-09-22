#include "pch.h"
#include "DebugDraw.h"

void DebugDraw::DrawLine(const Vec3& from, const Vec3& to, const Color& color)
{
	if (!_enabled)
		return;

	_vertices.push_back({ from, color });
	_vertices.push_back({ to, color });
}

void DebugDraw::DrawBox(const Vec3& center, const Vec3& extents, const Color& color)
{
	DrawBox(center, extents, Matrix::Identity, color);
}

void DebugDraw::DrawBox(const Vec3& center, const Vec3& extents, const Matrix& world, const Color& color)
{
	const Vec3 mn = center - extents;
	const Vec3 mx = center + extents;

	Vec3 corners[8] =
	{
		{ mn.x, mn.y, mn.z }, { mx.x, mn.y, mn.z }, { mx.x, mx.y, mn.z }, { mn.x, mx.y, mn.z },
		{ mn.x, mn.y, mx.z }, { mx.x, mn.y, mx.z }, { mx.x, mx.y, mx.z }, { mn.x, mx.y, mx.z },
	};
	for (Vec3& corner : corners)
		corner = Vec3::Transform(corner, world);

	static const int32 edges[12][2] =
	{
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
		{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
	};

	for (const auto& edge : edges)
		DrawLine(corners[edge[0]], corners[edge[1]], color);
}

void DebugDraw::DrawGrid(float halfSize, float step, const Color& color)
{
	if (step <= 0.f)
		return;

	for (float v = -halfSize; v <= halfSize + 0.0001f; v += step)
	{
		DrawLine({ v, 0.f, -halfSize }, { v, 0.f, halfSize }, color);
		DrawLine({ -halfSize, 0.f, v }, { halfSize, 0.f, v }, color);
	}
}

void DebugDraw::Flush()
{
	if (_vertices.empty())
		return;

	EnsureInit();
	EnsureCapacity(static_cast<uint32>(_vertices.size()));

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = DC->Map(_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	CHECK(hr);
	::memcpy(mapped.pData, _vertices.data(), sizeof(VertexColorData) * _vertices.size());
	DC->Unmap(_vertexBuffer.Get(), 0);

	const uint32 stride = sizeof(VertexColorData);
	const uint32 offset = 0;
	DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DC->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);
	_shader->Draw(0, 0, static_cast<uint32>(_vertices.size()));

	_vertices.clear();
}

void DebugDraw::EnsureInit()
{
	if (_shader)
		return;

	_shader = make_shared<Shader>(L"22. DebugLine.fx");
	RENDER->RegisterShader(_shader);
	RENDER->Update(); // 등록 직후 이번 프레임의 VP를 바인딩
}

void DebugDraw::EnsureCapacity(uint32 vertexCount)
{
	if (_vertexBuffer && vertexCount <= _capacity)
		return;

	_capacity = max(vertexCount, _capacity == 0 ? 4096u : _capacity * 2);

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = sizeof(VertexColorData) * _capacity;

	_vertexBuffer.Reset();
	HRESULT hr = DEVICE->CreateBuffer(&desc, nullptr, _vertexBuffer.GetAddressOf());
	CHECK(hr);
}
