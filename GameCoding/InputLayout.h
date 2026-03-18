#pragma once

class InputLayout
{
public:
	InputLayout(ComPtr<ID3D11Device> device);
	~InputLayout();

	ComPtr<ID3D11InputLayout> GetComPtr() { return _inputLayout; }

	void Create(const vector< D3D11_INPUT_ELEMENT_DESC>& descs, ComPtr<ID3DBlob> blob);

private:
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11InputLayout> _inputLayout;
};

