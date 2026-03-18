#include "pch.h"
#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(ComPtr<ID3D11Device> device)
	: _device(device)
{

}

VertexBuffer::~VertexBuffer()
{

}
