#pragma once
#include "MonoBehaviour.h"

class AnimatedModelRenderer;

class PlayerController : public MonoBehaviour
{
public:
	virtual void Update() override;
	virtual void LateUpdate() override;

	void SetCamera(const shared_ptr<GameObject>& camera) { _camera = camera; }
	shared_ptr<GameObject> GetCamera() const { return _camera.lock(); }
	void SetInputEnabled(bool enabled) { _inputEnabled = enabled; }

	float GetMoveSpeed() const { return _moveSpeed; }
	void SetMoveSpeed(float value) { _moveSpeed = (std::max)(0.f, value); }
	float GetJumpSpeed() const { return _jumpSpeed; }
	void SetJumpSpeed(float value) { _jumpSpeed = (std::max)(0.f, value); }
	float GetGravity() const { return _gravity; }
	void SetGravity(float value) { _gravity = value; }
	float GetCameraDistance() const { return _cameraDistance; }
	void SetCameraDistance(float value) { _cameraDistance = (std::max)(0.1f, value); }
	float GetCameraHeight() const { return _cameraHeight; }
	void SetCameraHeight(float value) { _cameraHeight = value; }

private:
	weak_ptr<GameObject> _camera;
	float _moveSpeed = 4.f;
	float _jumpSpeed = 7.f;
	float _gravity = -18.f;
	float _cameraDistance = 5.5f;
	float _cameraHeight = 1.8f;
	float _verticalVelocity = 0.f;
	float _cameraYaw = 0.f;
	float _cameraPitch = 0.25f;
	POINT _lastMousePosition = {};
	bool _mouseInitialized = false;
	bool _grounded = true;
	bool _inputEnabled = true;
};
