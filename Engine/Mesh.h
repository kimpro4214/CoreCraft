#pragma once
#include "ResourceBase.h"
#include "Geometry.h"

class Mesh : public ResourceBase
{
    using Super = ResourceBase;

public:
    Mesh();
    virtual ~Mesh();

	void CreateQuad();
	void CreateCube();
	void CreateGrid(int32 sizeX, int32 sizeZ);
	void CreateSphere();

	shared_ptr<VertexBuffer> GetVertexBuffer() { return _vertexBuffer; }
	shared_ptr<IndexBuffer> GetIndexBuffer() { return _indexBuffer; }

	// 피킹용 CPU 측 정점/인덱스와 로컬 공간 AABB
	const shared_ptr<Geometry<VertexTextureNormalTangentData>>& GetGeometry() const { return _geometry; }
	const BoundingBox& GetBounds() const { return _bounds; }

private:
	void CreateBuffers();

private:
	// Mesh
	shared_ptr<Geometry<VertexTextureNormalTangentData>> _geometry;
	BoundingBox _bounds;
	shared_ptr<VertexBuffer> _vertexBuffer;
	shared_ptr<IndexBuffer> _indexBuffer;
};

