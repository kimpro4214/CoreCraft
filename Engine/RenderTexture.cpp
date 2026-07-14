#include "pch.h"
#include "RenderTexture.h"

bool RenderTexture::Create(uint32 width, uint32 height)
{
	if (width == 0 || height == 0)
		return false;

	_width = width;
	_height = height;
	_colorTexture.Reset();
	_renderTargetView.Reset();
	_shaderResourceView.Reset();
	_depthTexture.Reset();
	_depthStencilView.Reset();

	D3D11_TEXTURE2D_DESC colorDesc = {};
	colorDesc.Width = width;
	colorDesc.Height = height;
	colorDesc.MipLevels = 1;
	colorDesc.ArraySize = 1;
	colorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	colorDesc.SampleDesc.Count = 1;
	colorDesc.Usage = D3D11_USAGE_DEFAULT;
	colorDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(DEVICE->CreateTexture2D(&colorDesc, nullptr, _colorTexture.GetAddressOf())))
		return false;
	if (FAILED(DEVICE->CreateRenderTargetView(_colorTexture.Get(), nullptr, _renderTargetView.GetAddressOf())))
		return false;
	if (FAILED(DEVICE->CreateShaderResourceView(_colorTexture.Get(), nullptr, _shaderResourceView.GetAddressOf())))
		return false;

	D3D11_TEXTURE2D_DESC depthDesc = colorDesc;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	if (FAILED(DEVICE->CreateTexture2D(&depthDesc, nullptr, _depthTexture.GetAddressOf())))
		return false;
	if (FAILED(DEVICE->CreateDepthStencilView(_depthTexture.Get(), nullptr, _depthStencilView.GetAddressOf())))
		return false;

	_viewport.Width = static_cast<float>(width);
	_viewport.Height = static_cast<float>(height);
	_viewport.MinDepth = 0.f;
	_viewport.MaxDepth = 1.f;
	return true;
}

bool RenderTexture::Resize(uint32 width, uint32 height)
{
	if (_width == width && _height == height)
		return true;
	return Create(width, height);
}

void RenderTexture::Bind(const Color& clearColor)
{
	DC->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());
	DC->RSSetViewports(1, &_viewport);
	DC->ClearRenderTargetView(_renderTargetView.Get(), reinterpret_cast<const float*>(&clearColor));
	DC->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}

void RenderTexture::Unbind()
{
	GRAPHICS->BindBackBuffer();
}
