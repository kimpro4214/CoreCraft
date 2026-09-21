#pragma once
#include "UMeshComponent.h"

class Model;
class Shader;
class Material;

class UStaticMeshComponent : public UMeshComponent
{
	DECLARE_UCLASS(UStaticMeshComponent, UMeshComponent)
public:
	UStaticMeshComponent();
	virtual ~UStaticMeshComponent();

	virtual void Render() override;

	void SetStaticMesh(const shared_ptr<Model>& model);
	shared_ptr<Model> GetStaticMesh() const { return _model; }

	void SetShader(const shared_ptr<Shader>& shader);
	shared_ptr<Shader> GetShader() const { return _shader; }

	void SetPass(uint8 pass) { _pass = pass; }
	uint8 GetPass() const { return _pass; }

	virtual uint32 GetNumMaterials() const override;
	virtual shared_ptr<Material> GetMaterial(uint32 index) const override;
	virtual void SetMaterial(uint32 index, const shared_ptr<Material>& material) override;

private:
	shared_ptr<Model> _model;
	shared_ptr<Shader> _shader;
	uint8 _pass = 0;
};
