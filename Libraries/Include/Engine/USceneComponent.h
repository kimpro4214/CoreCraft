#pragma once
#include "UActorComponent.h"

class USceneComponent : public UActorComponent
{
	DECLARE_UCLASS(USceneComponent, UActorComponent)
public:
	USceneComponent();
	virtual ~USceneComponent();

	// Local (Relative)
	const Vec3& GetRelativeLocation() const { return _relativeLocation; }
	void SetRelativeLocation(const Vec3& location) { _relativeLocation = location; UpdateWorldTransform(); }
	const Vec3& GetRelativeRotation() const { return _relativeRotation; }
	void SetRelativeRotation(const Vec3& rotation) { _relativeRotation = rotation; UpdateWorldTransform(); }
	const Vec3& GetRelativeScale() const { return _relativeScale; }
	void SetRelativeScale(const Vec3& scale) { _relativeScale = scale; UpdateWorldTransform(); }

	// World (Cache)
	Vec3 GetWorldLocation() const { return _worldMatrix.Translation(); }
	const Matrix& GetWorldMatrix() const { return _worldMatrix; }

	bool AttachToComponent(const shared_ptr<USceneComponent>& parent);
	void DetachFromParent();
	shared_ptr<USceneComponent> GetAttachParent() const { return _parent.lock(); }
	const vector<shared_ptr<USceneComponent>>& GetAttachChildren() const { return _children; }

	void UpdateWorldTransform();

private:
	Vec3 _relativeLocation = { 0.f, 0.f, 0.f };
	Vec3 _relativeRotation = { 0.f, 0.f, 0.f };
	Vec3 _relativeScale = { 1.f, 1.f, 1.f };

	// Cache
	Matrix _worldMatrix = Matrix::Identity;

	weak_ptr<USceneComponent> _parent;
	vector<shared_ptr<USceneComponent>> _children;
};
