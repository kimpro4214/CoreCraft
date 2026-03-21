#pragma once

class Texture
{
public:
	Texture(ComPtr<ID3D11Device> device);
	~Texture();

	ComPtr<ID3D11ShaderResourceView> GetComPtr() { return _shaderResourveView; }

	void Create(const wstring& path);

private:
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11ShaderResourceView> _shaderResourveView;
};

