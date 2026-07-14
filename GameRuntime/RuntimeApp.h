#pragma once

#include "IExecute.h"

class Scene;
class Shader;
class GameObject;

class RuntimeApp : public IExecute
{
public:
	explicit RuntimeApp(filesystem::path scenePath);

	virtual void Init() override;
	virtual void Update() override;
	virtual void Render() override;
	virtual void Shutdown() override;

private:
	filesystem::path _scenePath;
	shared_ptr<Scene> _scene;
	shared_ptr<Shader> _shader;
	shared_ptr<GameObject> _fallbackCamera;
};
