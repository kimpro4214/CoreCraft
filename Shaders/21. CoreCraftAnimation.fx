#include "20. CoreCraftLight.fx"

#define MAX_MODEL_TRANSFORMS 250

struct KeyframeDesc
{
	int animIndex;
	uint currFrame;
	uint nextFrame;
	float ratio;
	float sumTime;
	float speed;
	float2 padding;
};

struct TweenFrameDesc
{
	float tweenDuration;
	float tweenRatio;
	float tweenSumTime;
	float padding;
	KeyframeDesc curr;
	KeyframeDesc next;
};

cbuffer TweenBuffer
{
	TweenFrameDesc TweenFrames;
};

cbuffer BoneBuffer
{
	matrix BoneTransforms[MAX_MODEL_TRANSFORMS];
};

uint BoneIndex;
Texture2DArray TransformMap;

matrix SampleAnimation(VertexTextureNormalTangentBlend input, KeyframeDesc frame)
{
	float indices[4] = { input.blendIndices.x, input.blendIndices.y, input.blendIndices.z, input.blendIndices.w };
	float weights[4] = { input.blendWeights.x, input.blendWeights.y, input.blendWeights.z, input.blendWeights.w };
	matrix result = 0;
	[unroll]
	for (int index = 0; index < 4; ++index)
	{
		float4 c0 = TransformMap.Load(int4(indices[index] * 4 + 0, frame.currFrame, frame.animIndex, 0));
		float4 c1 = TransformMap.Load(int4(indices[index] * 4 + 1, frame.currFrame, frame.animIndex, 0));
		float4 c2 = TransformMap.Load(int4(indices[index] * 4 + 2, frame.currFrame, frame.animIndex, 0));
		float4 c3 = TransformMap.Load(int4(indices[index] * 4 + 3, frame.currFrame, frame.animIndex, 0));
		matrix current = matrix(c0, c1, c2, c3);
		float4 n0 = TransformMap.Load(int4(indices[index] * 4 + 0, frame.nextFrame, frame.animIndex, 0));
		float4 n1 = TransformMap.Load(int4(indices[index] * 4 + 1, frame.nextFrame, frame.animIndex, 0));
		float4 n2 = TransformMap.Load(int4(indices[index] * 4 + 2, frame.nextFrame, frame.animIndex, 0));
		float4 n3 = TransformMap.Load(int4(indices[index] * 4 + 3, frame.nextFrame, frame.animIndex, 0));
		matrix next = matrix(n0, n1, n2, n3);
		result += weights[index] * lerp(current, next, frame.ratio);
	}
	return result;
}

matrix GetAnimationMatrix(VertexTextureNormalTangentBlend input)
{
	matrix current = SampleAnimation(input, TweenFrames.curr);
	if (TweenFrames.next.animIndex < 0)
		return current;
	matrix next = SampleAnimation(input, TweenFrames.next);
	return lerp(current, next, saturate(TweenFrames.tweenRatio));
}

MeshOutput VS(VertexTextureNormalTangentBlend input)
{
	MeshOutput output;
	matrix animation = GetAnimationMatrix(input);
	float4 animatedPosition = mul(input.position, animation);
	float3 animatedNormal = mul(float4(input.normal, 0.f), animation).xyz;
	output.position = mul(animatedPosition, W);
	output.worldPosition = output.position.xyz;
	output.position = mul(output.position, VP);
	output.uv = input.uv;
	output.normal = mul(animatedNormal, (float3x3)W);
	output.tangent = mul(input.tangent, (float3x3)W);
	return output;
}

float4 PS(MeshOutput input) : SV_TARGET
{
	return ComputeCoreCraftLight(input.normal, input.uv, input.worldPosition);
}

technique11 T0
{
	PASS_VP(P0, VS, PS)
};
