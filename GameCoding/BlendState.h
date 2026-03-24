#pragma once

class BlendState
{
public:
	BlendState(ComPtr<ID3D11Device> device);
	~BlendState();

	const float* GetBlendFactor() { return &_blendFactor; }
	uint32 GetSampleMask() { return _sampleMask; }
	ComPtr<ID3D11BlendState> GetComPtr() { return _blendState; }

	void Create(D3D11_RENDER_TARGET_BLEND_DESC blendDesc =
		{
			true,
			D3D11_BLEND_SRC_ALPHA,
			D3D11_BLEND_INV_SRC_ALPHA,
			D3D11_BLEND_OP_ADD,
			D3D11_BLEND_ONE,
			D3D11_BLEND_ZERO,
			D3D11_BLEND_OP_ADD,
			D3D11_COLOR_WRITE_ENABLE_ALL
		}, float factor = 0.f);


private:
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11BlendState> _blendState;
	float _blendFactor = 0.f;
	uint32 _sampleMask = 0xFFFFFFFF;
};

