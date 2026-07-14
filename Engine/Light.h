#pragma once
#include "Component.h"

enum class LightType : uint8
{
	Directional,
	Point,
};

class Light : public Component
{
	using Super = Component;

public:
	Light();

	LightType GetLightType() const { return _lightType; }
	void SetLightType(LightType type) { _lightType = type; }
	const Color& GetColor() const { return _color; }
	void SetColor(const Color& color) { _color = color; }
	float GetIntensity() const { return _intensity; }
	void SetIntensity(float intensity) { _intensity = max(0.f, intensity); }
	float GetRange() const { return _range; }
	void SetRange(float range) { _range = max(0.01f, range); }

private:
	LightType _lightType = LightType::Directional;
	Color _color = Color(1.f, 0.96f, 0.88f, 1.f);
	float _intensity = 1.f;
	float _range = 10.f;
};
