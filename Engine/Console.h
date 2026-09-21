#pragma once

class Console
{
	DECLARE_SINGLE(Console);
public:
	using CommandCallback = function<void(const vector<string>& args)>;

	void Init();
	void Update();
	void Draw();

	void RegisterCommand(const string& name, CommandCallback callback);
	void Execute(const string& commandLine);
	void Log(const string& message);

	bool IsOpen() const { return _open; }
	void SetOpen(bool open) { _open = open; }

	const vector<string>& GetHistory() const { return _history; }

private:
	bool _open = false;
	bool _focusInput = false;
	vector<string> _history;
	char _inputBuffer[256] = {};
	unordered_map<string, CommandCallback> _commands;
};
