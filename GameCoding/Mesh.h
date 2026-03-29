#pragma once
#include "ResourceBase.h"

class Mesh : public ResourceBase
{
    using Super = ResourceBase;

public:
    Mesh(ComPtr<ID3D11Device> device);
    virtual ~Mesh();

	void CreateDefautRectangle();

	shared_ptr<VertexBuffer> GetVertexBuffer() { return _vertexBuffer; }
	shared_ptr<IndexBuffer> GetIndexBuffer() { return _indexBuffer; }

private:
	ComPtr<ID3D11Device> _device;

	// Mesh
	shared_ptr<Geometry<VertexTextureData>> _geometry;
	shared_ptr<VertexBuffer> _vertexBuffer;
	shared_ptr<IndexBuffer> _indexBuffer;
};

