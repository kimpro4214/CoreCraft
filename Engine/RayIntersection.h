#pragma once

class Mesh;

// Möller–Trumbore 레이-삼각형 교차. direction은 정규화 불필요, outT는 direction 배수 단위.
bool RayTriangleIntersect(const Vec3& origin, const Vec3& direction, const Vec3& v0, const Vec3& v1, const Vec3& v2, float& outT);

// 메시 로컬 공간 레이 판정: AABB로 먼저 거른 뒤 삼각형 단위로 가장 가까운 교차점을 찾는다.
// direction은 정규화된 값이어야 한다(AABB 선검사 요구사항).
bool RayMeshIntersect(const Vec3& origin, const Vec3& direction, const Mesh& mesh, float& outT);
