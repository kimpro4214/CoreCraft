#pragma once
#include "VertexData.h"

class Shader;

// 프레임 동안 요청된 디버그 라인을 모아 Flush()에서 드로우콜 한 번으로 그린다.
class DebugDraw
{
	DECLARE_SINGLE(DebugDraw);

public:
	void DrawLine(const Vec3& from, const Vec3& to, const Color& color);
	void DrawBox(const Vec3& center, const Vec3& extents, const Color& color);
	void DrawBox(const Vec3& center, const Vec3& extents, const Matrix& world, const Color& color);
	void DrawGrid(float halfSize, float step, const Color& color);

	void Flush();

	void SetEnabled(bool enable) { _enabled = enable; }
	bool IsEnabled() const { return _enabled; }

private:
	void EnsureInit();
	void EnsureCapacity(uint32 vertexCount);

private:
	bool _enabled = true;
	shared_ptr<Shader> _shader;
	ComPtr<ID3D11Buffer> _vertexBuffer;
	uint32 _capacity = 0;
	vector<VertexColorData> _vertices;
};
