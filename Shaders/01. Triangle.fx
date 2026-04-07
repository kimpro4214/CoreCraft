
struct VertexInput
{
	float4 position : POSITION;
};

struct VertexOutput
{
	float4 position : SV_POSITION;
};

VertexOutput VS(VertexInput input)
{
	VertexOutput output;
	output.position = input.position;

	return output;
}

float4 PS(VertexOutput input) : SV_TARGET
{
	return float4(1, 0, 0, 1);
}

float4 PS2(VertexOutput input) : SV_TARGET
{
	return float4(0, 1, 0, 1);
}

float4 PS3(VertexOutput input) : SV_TARGET
{
	return float4(0, 0, 1, 1);
}

technique11 T0
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}

	pass P1
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS2()));
	}
};

technique11 T1
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS3()));
	}
};