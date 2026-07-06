#pragma once
#include "IExecute.h"

class AssimpTool : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

};

