#pragma once
#include "Component.h"

class Mesh;
class Shader;
class Material;

class MeshRenderer : public Component
{
	using Super = Component;
public:
	MeshRenderer();
	virtual ~MeshRenderer();

	virtual void Update() override;

	void SetMesh(shared_ptr<Mesh> mesh) { _mesh = mesh; }
	void SetMaterial(shared_ptr<Material> material) { _material = material; }

	// Legacy
	void SetTexture(shared_ptr<Texture> texture) { }
	void SetShader(shared_ptr<Shader> shader) {  }

private:
	shared_ptr<Mesh> _mesh;
	shared_ptr<Material> _material;
	//shared_ptr<Texture> _texture;
	//shared_ptr<Shader> _shader;
};

