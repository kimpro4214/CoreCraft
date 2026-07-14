#pragma once

class RenderTexture
{
public:
	bool Create(uint32 width, uint32 height);
	bool Resize(uint32 width, uint32 height);
	void Bind(const Color& clearColor = Color(0.075f, 0.085f, 0.105f, 1.f));
	void Unbind();

	ID3D11ShaderResourceView* GetShaderResourceView() const { return _shaderResourceView.Get(); }
	uint32 GetWidth() const { return _width; }
	uint32 GetHeight() const { return _height; }

private:
	uint32 _width = 0;
	uint32 _height = 0;
	ComPtr<ID3D11Texture2D> _colorTexture;
	ComPtr<ID3D11RenderTargetView> _renderTargetView;
	ComPtr<ID3D11ShaderResourceView> _shaderResourceView;
	ComPtr<ID3D11Texture2D> _depthTexture;
	ComPtr<ID3D11DepthStencilView> _depthStencilView;
	D3D11_VIEWPORT _viewport = {};
};
