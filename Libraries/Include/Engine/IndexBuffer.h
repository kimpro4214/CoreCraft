#pragma once

class IndexBuffer
{
public:
	IndexBuffer();
	~IndexBuffer();

	ComPtr<ID3D11Buffer> GetComPtr() { return _indexBuffer; }
	uint32 GetStride() { return _stride; }
	uint32 GetOffset() { return _offset; }
	uint32 GetCount() { return _count; }

	void Create(const vector<uint32>& indices);

private:
	ComPtr<ID3D11Buffer> _indexBuffer;

	uint32 _stride = 0;
	uint32 _offset = 0;
	uint32 _count = 0;
};

