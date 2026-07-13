#include "pch.h"
#include "PlayerController.h"
#include "GameObject.h"
#include "Transform.h"
#include "AnimatedModelRenderer.h"

void PlayerController::Update()
{
	shared_ptr<GameObject> object = GetGameObject();
	if (object == nullptr)
		return;
	POINT mouse = INPUT->GetMousePos();
	if (!_mouseInitialized)
	{
		_lastMousePosition = mouse;
		_mouseInitialized = true;
	}

	if (_inputEnabled && INPUT->GetButton(KEY_TYPE::RBUTTON))
	{
		_cameraYaw += static_cast<float>(mouse.x - _lastMousePosition.x) * 0.004f;
		_cameraPitch += static_cast<float>(mouse.y - _lastMousePosition.y) * 0.004f;
		_cameraPitch = (std::clamp)(_cameraPitch, -0.6f, 1.1f);
	}
	_lastMousePosition = mouse;

	Vec3 input = Vec3::Zero;
	if (_inputEnabled)
	{
		if (INPUT->GetButton(KEY_TYPE::W)) input.z += 1.f;
		if (INPUT->GetButton(KEY_TYPE::S)) input.z -= 1.f;
		if (INPUT->GetButton(KEY_TYPE::D)) input.x += 1.f;
		if (INPUT->GetButton(KEY_TYPE::A)) input.x -= 1.f;
	}

	bool moving = input.LengthSquared() > 0.001f;
	Vec3 position = GetTransform()->GetPosition();
	if (moving)
	{
		input.Normalize();
		Vec3 forward(sinf(_cameraYaw), 0.f, cosf(_cameraYaw));
		Vec3 right(cosf(_cameraYaw), 0.f, -sinf(_cameraYaw));
		Vec3 movement = forward * input.z + right * input.x;
		movement.Normalize();
		position += movement * _moveSpeed * DT;
		GetTransform()->SetRotation({ 0.f, atan2f(movement.x, movement.z), 0.f });
	}

	if (_inputEnabled && _grounded && INPUT->GetButtonDown(static_cast<KEY_TYPE>(VK_SPACE)))
	{
		_verticalVelocity = _jumpSpeed;
		_grounded = false;
	}
	_verticalVelocity += _gravity * DT;
	position.y += _verticalVelocity * DT;
	if (position.y <= 0.f)
	{
		position.y = 0.f;
		_verticalVelocity = 0.f;
		_grounded = true;
	}
	GetTransform()->SetPosition(position);

	shared_ptr<AnimatedModelRenderer> animator = object->GetAnimatedModelRenderer();
	if (animator)
		animator->PlayAnimation(moving ? 1 : 0, 0.15f);
}

void PlayerController::LateUpdate()
{
	shared_ptr<GameObject> camera = _camera.lock();
	if (camera == nullptr || camera->GetTransform() == nullptr)
		return;
	shared_ptr<Transform> cameraTransform = camera->GetTransform();
	cameraTransform->SetRotation({ _cameraPitch, _cameraYaw, 0.f });
	Vec3 target = GetTransform()->GetPosition() + Vec3(0.f, _cameraHeight, 0.f);
	cameraTransform->SetPosition(target - cameraTransform->GetLook() * _cameraDistance);
}
