#pragma once

class Graphics
{
	DECLARE_SINGLE(Graphics);

public:
	void Init(HWND hwnd);

	void RenderBegin();
	void RenderEnd();
	void Resize(uint32 width, uint32 height);
	void BindBackBuffer();

	ComPtr<ID3D11Device> GetDevice() { return _device; }
	ComPtr<ID3D11DeviceContext> GetDeviceContext() { return _deviceContext; }
	ID3D11RenderTargetView* GetBackBufferRTV() const { return _renderTargetView.Get(); }
	ID3D11DepthStencilView* GetBackBufferDSV() const { return _depthStencilView.Get(); }
	const D3D11_VIEWPORT& GetViewport() const { return _viewport; }

	// 와이어프레임 디버그 모드 (활성화 시 모든 Pass의 RasterizerState를 덮어씀)
	void SetWireframe(bool enable);
	bool IsWireframe() const { return _wireframe; }
	ID3D11RasterizerState* GetWireframeRasterizerState() const { return _wireframeRasterizerState.Get(); }

private:
	void CreateDeviceAndSwapChain();
	void CreateRenderTargetView();
	void CreateDepthStencilView();
	void SetViewport(uint32 width, uint32 height);
	void CreateWireframeRasterizerState();

private:
	HWND _hwnd = {};

	// Device & SwapChain
	ComPtr<ID3D11Device> _device = nullptr;
	ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
	ComPtr<IDXGISwapChain> _swapChain = nullptr;

	// RTV
	ComPtr<ID3D11RenderTargetView> _renderTargetView;

	// DSV
	ComPtr<ID3D11Texture2D> _depthStencilTexture;
	ComPtr<ID3D11DepthStencilView> _depthStencilView;

	// Misc
	D3D11_VIEWPORT _viewport = { 0 };

	// Wireframe
	ComPtr<ID3D11RasterizerState> _wireframeRasterizerState;
	bool _wireframe = false;
};

