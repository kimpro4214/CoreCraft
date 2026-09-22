#include "pch.h"
#include "Console.h"

void Console::Init()
{
	RegisterCommand("help", [this](const vector<string>& args)
	{
		for (const auto& command : _commands)
			Log(command.first);
	});

	RegisterCommand("clear", [this](const vector<string>& args)
	{
		_history.clear();
	});

	// wireframe [0|1] — 인자 없으면 토글
	RegisterCommand("wireframe", [this](const vector<string>& args)
	{
		bool enable = !GRAPHICS->IsWireframe();
		if (!args.empty())
			enable = (args[0] == "1" || args[0] == "on" || args[0] == "true");

		GRAPHICS->SetWireframe(enable);
		Log(string("wireframe: ") + (enable ? "on" : "off"));
	});
}

void Console::Update()
{
	if (INPUT->GetButtonDown(KEY_TYPE::TILDE))
	{
		_open = !_open;
		if (_open)
			_focusInput = true;
	}
}

void Console::Draw()
{
	if (!_open)
		return;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float height = 300.f;
	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - height));
	ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, height));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("Console", &_open, flags);

	float inputHeight = ImGui::GetFrameHeightWithSpacing();
	ImGui::BeginChild("ConsoleHistory", ImVec2(0, -inputHeight), false);
	for (const string& line : _history)
		ImGui::TextUnformatted(line.c_str());
	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		ImGui::SetScrollHereY(1.f);
	ImGui::EndChild();

	ImGui::Separator();

	bool submit = false;
	ImGui::SetNextItemWidth(-1);
	if (_focusInput)
	{
		ImGui::SetKeyboardFocusHere();
		_focusInput = false;
	}
	if (ImGui::InputText("##ConsoleInput", _inputBuffer, sizeof(_inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
		submit = true;

	ImGui::End();

	if (submit)
	{
		Execute(_inputBuffer);
		_inputBuffer[0] = '\0';
		_focusInput = true;
	}
}

void Console::RegisterCommand(const string& name, CommandCallback callback)
{
	_commands[name] = move(callback);
}

void Console::Execute(const string& commandLine)
{
	istringstream iss(commandLine);
	vector<string> tokens;
	string token;
	while (iss >> token)
		tokens.push_back(token);

	if (tokens.empty())
		return;

	Log("> " + commandLine);

	string name = tokens[0];
	vector<string> args(tokens.begin() + 1, tokens.end());

	auto found = _commands.find(name);
	if (found == _commands.end())
	{
		Log("Unknown command: " + name);
		return;
	}

	found->second(args);
}

void Console::Log(const string& message)
{
	_history.push_back(message);
}
