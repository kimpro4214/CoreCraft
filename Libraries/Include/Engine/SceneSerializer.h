#pragma once

class Scene;
class Shader;

class SceneSerializer
{
public:
	static bool Save(const shared_ptr<Scene>& scene, const filesystem::path& path, string* error = nullptr);
	static shared_ptr<Scene> Load(const filesystem::path& path, const shared_ptr<Shader>& defaultShader, string* error = nullptr);
};
