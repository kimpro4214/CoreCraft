#pragma once
#include "USceneComponent.h"

class UPrimitiveComponent : public USceneComponent
{
	DECLARE_UCLASS(UPrimitiveComponent, USceneComponent)
public:
	UPrimitiveComponent();
	virtual ~UPrimitiveComponent();

	virtual void Render() { }

	bool IsVisible() const { return _visible; }
	void SetVisible(bool visible) { _visible = visible; }

private:
	bool _visible = true;
};
