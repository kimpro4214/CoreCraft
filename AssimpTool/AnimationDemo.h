#pragma once
#include "IExecute.h"

class AnimationDemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

	void CreateKachujin();

private:
	shared_ptr<Shader> _shader;
	shared_ptr<GameObject> _obj;
	shared_ptr<GameObject> _camera;
};

