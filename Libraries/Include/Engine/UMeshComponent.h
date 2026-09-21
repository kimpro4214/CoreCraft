#pragma once
#include "UPrimitiveComponent.h"

class Material;

class UMeshComponent : public UPrimitiveComponent
{
	DECLARE_UCLASS(UMeshComponent, UPrimitiveComponent)
public:
	UMeshComponent();
	virtual ~UMeshComponent();

	virtual uint32 GetNumMaterials() const { return 0; }
	virtual shared_ptr<Material> GetMaterial(uint32 index) const { return nullptr; }
	virtual void SetMaterial(uint32 index, const shared_ptr<Material>& material) { }
};
