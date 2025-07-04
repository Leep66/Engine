#pragma once
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/IntVec2.hpp"

extern unsigned char const KEYCODE_F1;
extern unsigned char const KEYCODE_F2;
extern unsigned char const KEYCODE_F3;
extern unsigned char const KEYCODE_F4;
extern unsigned char const KEYCODE_F5;
extern unsigned char const KEYCODE_F6;
extern unsigned char const KEYCODE_F7;
extern unsigned char const KEYCODE_F8;
extern unsigned char const KEYCODE_F9;
extern unsigned char const KEYCODE_F10;
extern unsigned char const KEYCODE_F11;
extern unsigned char const KEYCODE_ESC;
extern unsigned char const KEYCODE_UPARROW;
extern unsigned char const KEYCODE_DOWNARROW;
extern unsigned char const KEYCODE_LEFTARROW;
extern unsigned char const KEYCODE_RIGHTARROW;
extern unsigned char const KEYCODE_LEFT_MOUSE;
extern unsigned char const KEYCODE_RIGHT_MOUSE;
extern unsigned char const KEYCODE_TILDE;
extern unsigned char const KEYCODE_LEFTBRACKET;
extern unsigned char const KEYCODE_RIGHTBRACKET;
extern unsigned char const KEYCODE_ENTER;
extern unsigned char const KEYCODE_BACKSPACE;
extern unsigned char const KEYCODE_INSERT;
extern unsigned char const KEYCODE_DELETE;
extern unsigned char const KEYCODE_HOME;
extern unsigned char const KEYCODE_END;
extern unsigned char const KEYCODE_SHIFT;
extern unsigned char const KEYCODE_CONTROL;
extern unsigned char const KEYCODE_1;
extern unsigned char const KEYCODE_2;
extern unsigned char const KEYCODE_3;
extern unsigned char const KEYCODE_4;
extern unsigned char const KEYCODE_5;
extern unsigned char const KEYCODE_6;
extern unsigned char const KEYCODE_7;
extern unsigned char const KEYCODE_8;
extern unsigned char const KEYCODE_9;
extern unsigned char const KEYCODE_0;


constexpr int NUM_KEYCODES = 256;
constexpr int NUM_XBOX_CONTROLLERS = 4;

struct InputConfig
{
	
};

enum class CursorMode
{
	POINTER,
	FPS,
};

struct CusorState
{
	IntVec2 m_cursorClientDelta;
	IntVec2 m_cursorClientPosition;

	CursorMode m_cursorMode = CursorMode::POINTER;
};

class InputSystem
{
public:
	InputSystem(InputConfig const& config);
	~InputSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();
	bool WasKeyJustPressed(unsigned char keyCode) const;
	bool WasKeyJustReleased(unsigned char keyCode) const;
	bool IsKeyDown(unsigned char keyCode) const;
	void HandleKeyPressed(unsigned char keyCode);
	void HandleKeyReleased(unsigned char keyCode);
	XboxController const& GetController(int controllerID);

	void SetCursorMode(CursorMode cursorMode);
	Vec2 GetCursorClientDelta() const;
	Vec2 GetCursorClientPosition() const;
	Vec2 GetCursorNormalizedPosition() const;

	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_KeyReleased(EventArgs& args);

	

protected:
	KeyButtonState m_keyState[NUM_KEYCODES];
	XboxController m_controllers[NUM_XBOX_CONTROLLERS];
	CusorState m_cursorState;
};