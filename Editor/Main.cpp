#include "pch.h"
#include "EditorApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	GameDesc desc;
	desc.appName = L"Core Craft Editor";
	desc.hInstance = hInstance;
	desc.width = 1440.f;
	desc.height = 810.f;
	desc.vsync = true;
	desc.clearColor = Color(0.015f, 0.015f, 0.02f, 1.f);
	desc.app = make_shared<EditorApp>();
	return static_cast<int>(GAME->Run(desc));
}
