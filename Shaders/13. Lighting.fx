#include "00. Global.fx"
#include "00. Light.fx"

MeshOutput VS(VertexTextureNormal input)
{
	MeshOutput output;
	output.position = mul(input.position, W);
	output.worldPosition = input.position.xyz;
	output.position = mul(output.position, VP);	
	output.uv = input.uv;
	output.normal = mul(input.normal, (float3x3)W);

	return output;
}

float4 PS(MeshOutput input) : SV_TARGET
{
	float4 color = ComputeLight(input.normal, input.uv, input.worldPosition);

	return color;
}

technique11 T0
{
	PASS_VP(P0, VS, PS)
};
