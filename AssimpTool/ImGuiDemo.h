#pragma once
#include "IExecute.h"

class ImGuiDemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

	void Test();

private:
	bool show_demo_window = true;
	bool show_another_window = false;
	Vec4 clear_color = Vec4(0.f);
};

