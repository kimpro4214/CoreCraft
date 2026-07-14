#include "pch.h"
#include "RuntimeApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR commandLine, int)
{
	filesystem::path scenePath = L"..\\Resources\\Scenes\\Startup.scene.xml";
	if (commandLine && commandLine[0] != '\0')
		scenePath = filesystem::path(commandLine);

	GameDesc desc;
	desc.appName = L"Core Craft";
	desc.hInstance = hInstance;
	desc.width = 1280.f;
	desc.height = 720.f;
	desc.vsync = true;
	desc.enableImGui = false;
	desc.clearColor = Color(0.055f, 0.075f, 0.11f, 1.f);
	desc.app = make_shared<RuntimeApp>(scenePath);
	return static_cast<int>(GAME->Run(desc));
}
