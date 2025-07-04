#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

class Renderer;
class Camera;
class BitmapFont;
class Timer;
struct AABB2;

class DevConsole;
extern DevConsole* g_theDevConsole;

struct DevConsoleLine
{
	Rgba8 m_color;
	std::string m_text;
	/*int m_frameNumberPrinted;
	double m_timePrinted;*/
};

enum class DevConsoleMode
{
	OPEN_FULL,
	OPEN_PARTIAL,
	COMMAND_PROMPT_ONLY,
	HIDDEN,

	NUM_DEV_CONSOLE
};

struct DevConsoleConfig
{
	/*float m_linesOnScreen = 0.f;
	Renderer* m_defaultRenderer = nullptr;
	std::string m_defaultFontName;*/
	Renderer* m_renderer = nullptr;
	Camera* m_camera = nullptr;
	std::string m_fontName = "SquirrelFixedFont";
	float m_fontAspect = 0.7f;
	int m_linesOnScreen = 40;
	int m_maxCommandHistory = 128;
	bool m_startOpen = false;
};

class DevConsole
{
public:
	DevConsole(DevConsoleConfig const& config);
	~DevConsole();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void Execute(std::string const& consoleCommandText, bool echoCommand = true);
	void AddLine(Rgba8 const& color, std::string const& text);
	void Render(AABB2 const& bounds, Renderer* rendererOverride = nullptr) const;

	DevConsoleMode GetMode() const;
	void SetMode(DevConsoleMode mode);
	void ToggleOpen();
	bool IsOpen() const;
	
	static const Rgba8 ERROR_COLOR;
	static const Rgba8 WARNING;
	static const Rgba8 INFO_MAJOR;
	static const Rgba8 INFO_MINOR;
	static const Rgba8 ECHO;
	static const Rgba8 INPUT_TEXT;
	static const Rgba8 INPUT_INSERTION_POINT;
	static const Rgba8 GAME_MAJOR;
	static const Rgba8 GAME_MINOR;
	static const Rgba8 GAME_DEBUG;

	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_CharInput(EventArgs& args);
	static bool Command_Clear(EventArgs& args);
	static bool Command_Help(EventArgs& args);

protected:
	void Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font) const;
	
protected:
	DevConsoleConfig m_config;
	bool m_isOpen = false;
	std::string m_inputText;
	int m_insertionPointPosition = 0;
	bool m_insertionPointVisible = true;
	Timer* m_insertionPointBlinkTimer;
	std::vector<std::string> m_commandHistory;
	int m_historyIndex = 0;
	std::string m_fontPath;
	DevConsoleMode m_mode = DevConsoleMode::HIDDEN;
	std::vector<DevConsoleLine> m_lineVerts;
	int m_frameNumber = 0;
	BitmapFont* m_defaultFont = nullptr;
};