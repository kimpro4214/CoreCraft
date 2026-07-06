#include "pch.h"
#include "AnimationDemo.h"
#include "GeometryHelper.h"
#include "Camera.h"
#include "GameObject.h"
#include "CameraScript.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "Material.h"
#include "Model.h"
#include "ModelRenderer.h"
#include "ModelAnimator.h"

void AnimationDemo::Init()
{
	RESOURCES->Init();
	_shader = make_shared<Shader>(L"16. AnimationDemo.fx");

	// Camera
	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(make_shared<CameraScript>());

	CreateKachujin();

	RENDER->Init(_shader);
}

void AnimationDemo::Update()
{
	_camera->Update();
	RENDER->Update();

	{
		LightDesc lightDesc;
		lightDesc.ambient = Vec4(0.4f);
		lightDesc.diffuse = Vec4(1.f);
		lightDesc.specular = Vec4(0.f);
		lightDesc.direction = Vec3(1.f, 0.f, 1.f);
		RENDER->PushLightData(lightDesc);
	}

	{
		_obj->Update();
	}
}

void AnimationDemo::Render()
{

}

void AnimationDemo::CreateKachujin()
{
	shared_ptr<class Model> m1 = make_shared<Model>();
	m1->ReadModel(L"Kachujin/Kachujin");
	m1->ReadMaterial(L"Kachujin/Kachujin");
	m1->ReadAnimation(L"Kachujin/Idle");
	m1->ReadAnimation(L"Kachujin/Run");
	m1->ReadAnimation(L"Kachujin/Slash");

	_obj = make_shared<GameObject>();
	_obj->GetOrAddTransform()->SetPosition(Vec3(0, 0, 1));
	_obj->GetOrAddTransform()->SetScale(Vec3(0.01f));

	_obj->AddComponent(make_shared<ModelAnimator>(_shader));
	{
		_obj->GetModelAnimator()->SetModel(m1);
		//_obj->GetModelAnimator()->SetPass(1);
	}
}
