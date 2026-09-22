#include "00. Global.fx"

struct LineOutput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

// 라인 정점은 이미 월드 공간이므로 W 없이 VP만 적용
LineOutput VS(VertexColor input)
{
	LineOutput output;
	output.position = mul(input.Position, VP);
	output.color = input.Color;
	return output;
}

float4 PS(LineOutput input) : SV_TARGET
{
	return input.color;
}

technique11 T0
{
	PASS_VP(P0, VS, PS)
};
