#pragma once
#include "IExecute.h"

class TriangleDemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;


	shared_ptr<Shader> _shader;
	vector<VertexData> _vertices;
	shared_ptr<VertexBuffer> _buffer;
};

