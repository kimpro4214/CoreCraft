#pragma once
#include "IExecute.h"

class StaticMeshDemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

	void CreateTower();
	void CreateTank();

private:
	shared_ptr<Shader> _shader;
	shared_ptr<GameObject> _obj;
	shared_ptr<GameObject> _camera;
};

