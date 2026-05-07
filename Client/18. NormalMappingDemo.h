#pragma once
#include "IExecute.h"
#include "Geometry.h"

class Frustum;

class NormalMappingDemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

	shared_ptr<Shader> _shader;

	shared_ptr<GameObject> _obj;
	shared_ptr<GameObject> _obj2;
	shared_ptr<GameObject> _camera;

	shared_ptr<Frustum> _frustum;
};