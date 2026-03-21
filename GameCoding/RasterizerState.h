#pragma once


class RasterizerState
{
public:
	RasterizerState(ComPtr<ID3D11Device> device);
	~RasterizerState();

	ComPtr< ID3D11RasterizerState> GetComPtr() { return _rasterizerState; }

	void Create();

private:
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11RasterizerState> _rasterizerState;
};
