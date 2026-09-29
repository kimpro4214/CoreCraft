#include "pch.h"
#include "RayIntersection.h"
#include "Mesh.h"

bool RayTriangleIntersect(const Vec3& origin, const Vec3& direction, const Vec3& v0, const Vec3& v1, const Vec3& v2, float& outT)
{
	constexpr float epsilon = 1e-7f;

	const Vec3 edge1 = v1 - v0;
	const Vec3 edge2 = v2 - v0;
	const Vec3 p = direction.Cross(edge2);
	const float determinant = edge1.Dot(p);
	// 레이가 삼각형 평면과 평행 (양면 판정이므로 절댓값 사용)
	if (fabsf(determinant) < epsilon)
		return false;

	const float inverseDeterminant = 1.f / determinant;
	const Vec3 s = origin - v0;
	const float u = s.Dot(p) * inverseDeterminant;
	if (u < 0.f || u > 1.f)
		return false;

	const Vec3 q = s.Cross(edge1);
	const float v = direction.Dot(q) * inverseDeterminant;
	if (v < 0.f || u + v > 1.f)
		return false;

	const float t = edge2.Dot(q) * inverseDeterminant;
	// 레이 시작점 뒤쪽 교차는 무시
	if (t <= epsilon)
		return false;

	outT = t;
	return true;
}

bool RayMeshIntersect(const Vec3& origin, const Vec3& direction, const Mesh& mesh, float& outT)
{
	const auto& geometry = mesh.GetGeometry();
	if (geometry == nullptr)
		return false;

	float boundsDistance = 0.f;
	if (!mesh.GetBounds().Intersects(origin, direction, boundsDistance))
		return false;

	const vector<VertexTextureNormalTangentData>& vertices = geometry->GetVertices();
	const vector<uint32>& indices = geometry->GetIndices();
	float nearest = FLT_MAX;
	for (size_t i = 0; i + 2 < indices.size(); i += 3)
	{
		float t = 0.f;
		if (RayTriangleIntersect(origin, direction, vertices[indices[i]].position, vertices[indices[i + 1]].position, vertices[indices[i + 2]].position, t) && t < nearest)
			nearest = t;
	}

	if (nearest == FLT_MAX)
		return false;
	outT = nearest;
	return true;
}
