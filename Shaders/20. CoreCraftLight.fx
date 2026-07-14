#ifndef _CORE_CRAFT_LIGHT_FX_
#define _CORE_CRAFT_LIGHT_FX_

#include "00. Global.fx"

struct PointLightDesc
{
	float4 color;
	float3 position;
	float range;
	float intensity;
	float3 padding;
};

struct LightDesc
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 emissive;
	float3 direction;
	float padding0;
	PointLightDesc points[8];
	uint pointCount;
	float3 padding1;
};

struct MaterialDesc
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 emissive;
	uint useDiffuseMap;
	float3 padding;
};

cbuffer LightBuffer
{
	LightDesc GlobalLight;
};

cbuffer MaterialBuffer
{
	MaterialDesc Material;
};

Texture2D DiffuseMap;
Texture2D SpecularMap;
Texture2D NormalMap;

float4 ComputeCoreCraftLight(float3 normal, float2 uv, float3 worldPosition)
{
	float3 N = normalize(normal);
	float4 albedo = Material.useDiffuseMap != 0
		? DiffuseMap.Sample(LinearSampler, uv)
		: float4(1, 1, 1, 1);
	float4 result = albedo * GlobalLight.ambient * Material.ambient;
	float directional = saturate(dot(-normalize(GlobalLight.direction), N));
	result += albedo * directional * GlobalLight.diffuse * Material.diffuse;

	float3 eye = normalize(CameraPosition() - worldPosition);
	float3 reflected = reflect(normalize(GlobalLight.direction), N);
	float directionalSpecular = pow(saturate(dot(reflected, eye)), 24.0f);
	result += GlobalLight.specular * Material.specular * directionalSpecular;

	[loop]
	for (uint index = 0; index < min(GlobalLight.pointCount, 8u); ++index)
	{
		PointLightDesc light = GlobalLight.points[index];
		float3 toLight = light.position - worldPosition;
		float distanceToLight = length(toLight);
		float3 L = distanceToLight > 0.0001f ? toLight / distanceToLight : float3(0, 1, 0);
		float attenuation = saturate(1.0f - distanceToLight / max(light.range, 0.001f));
		attenuation *= attenuation;
		float diffuse = saturate(dot(N, L));
		result += albedo * Material.diffuse * light.color * diffuse * attenuation * light.intensity;
	}

	result += GlobalLight.emissive * Material.emissive;
	result.a = albedo.a * Material.diffuse.a;
	return saturate(result);
}

#endif
