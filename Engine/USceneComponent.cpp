#include "pch.h"
#include "USceneComponent.h"

USceneComponent::USceneComponent()
{
}

USceneComponent::~USceneComponent()
{
}

bool USceneComponent::AttachToComponent(const shared_ptr<USceneComponent>& parent)
{
	if (!parent || parent.get() == this)
		return false;

	for (shared_ptr<USceneComponent> ancestor = parent; ancestor != nullptr; ancestor = ancestor->GetAttachParent())
	{
		if (ancestor.get() == this)
			return false;
	}

	DetachFromParent();

	_parent = parent;
	parent->_children.push_back(static_pointer_cast<USceneComponent>(shared_from_this()));

	UpdateWorldTransform();
	return true;
}

void USceneComponent::DetachFromParent()
{
	shared_ptr<USceneComponent> parent = _parent.lock();
	if (!parent)
		return;

	shared_ptr<USceneComponent> self = static_pointer_cast<USceneComponent>(shared_from_this());
	vector<shared_ptr<USceneComponent>>& siblings = parent->_children;
	siblings.erase(remove(siblings.begin(), siblings.end(), self), siblings.end());

	_parent.reset();
	UpdateWorldTransform();
}

void USceneComponent::UpdateWorldTransform()
{
	Matrix matScale = Matrix::CreateScale(_relativeScale);
	Matrix matRotation = Matrix::CreateRotationX(_relativeRotation.x);
	matRotation *= Matrix::CreateRotationY(_relativeRotation.y);
	matRotation *= Matrix::CreateRotationZ(_relativeRotation.z);
	Matrix matTranslation = Matrix::CreateTranslation(_relativeLocation);

	Matrix local = matScale * matRotation * matTranslation;

	shared_ptr<USceneComponent> parent = _parent.lock();
	_worldMatrix = parent ? local * parent->GetWorldMatrix() : local;

	for (shared_ptr<USceneComponent>& child : _children)
		child->UpdateWorldTransform();
}
