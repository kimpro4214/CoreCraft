#pragma once

struct StateBlock
{
	ComPtr<ID3D11RasterizerState> RSRasterizerState;
	ComPtr<ID3D11BlendState> OMBlendState;
	FLOAT OMBlendFactor[4];
	UINT OMSampleMask;
	ComPtr<ID3D11DepthStencilState> OMDepthStencilState;
	UINT OMStencilRef;
};

struct Pass
{
	wstring name;
	ID3DX11EffectPass* pass;
	D3DX11_PASS_DESC desc;

	ComPtr<ID3D11InputLayout> inputLayout;
	D3DX11_PASS_SHADER_DESC passVsDesc;
	D3DX11_EFFECT_SHADER_DESC effectVsDesc;
	vector<D3D11_SIGNATURE_PARAMETER_DESC> signatureDescs;

	D3DX11_STATE_BLOCK_MASK stateblockMask;
	shared_ptr<StateBlock> stateBlock;

	void Draw(UINT vertexCount, UINT startVertexLocation = 0);
	void DrawIndexed(UINT indexCount, UINT startIndexLocation = 0, INT baseVertexLocation = 0);
	void DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation = 0, UINT startInstanceLocation = 0);
	void DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation = 0, INT baseVertexLocation = 0, UINT startInstanceLocation = 0);

	void BeginDraw();
	void EndDraw();

	void Dispatch(UINT x, UINT y, UINT z);
};
