#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Windows.h"

unsigned char const KEYCODE_F1				= VK_F1;
unsigned char const KEYCODE_F2				= VK_F2;
unsigned char const KEYCODE_F3				= VK_F3;
unsigned char const KEYCODE_F4				= VK_F4;
unsigned char const KEYCODE_F5				= VK_F5;
unsigned char const KEYCODE_F6				= VK_F6;
unsigned char const KEYCODE_F7				= VK_F7;
unsigned char const KEYCODE_F8				= VK_F8;
unsigned char const KEYCODE_F9				= VK_F9;
unsigned char const KEYCODE_F10				= VK_F10;
unsigned char const KEYCODE_F11				= VK_F11;
unsigned char const KEYCODE_ESC				= VK_ESCAPE;
unsigned char const KEYCODE_UPARROW			= VK_UP;
unsigned char const KEYCODE_DOWNARROW		= VK_DOWN;
unsigned char const KEYCODE_LEFTARROW		= VK_LEFT;
unsigned char const KEYCODE_RIGHTARROW		= VK_RIGHT;
unsigned char const KEYCODE_LEFT_MOUSE		= VK_LBUTTON;
unsigned char const KEYCODE_RIGHT_MOUSE		= VK_RBUTTON;
unsigned char const KEYCODE_ENTER			= VK_RETURN;
unsigned char const KEYCODE_BACKSPACE		= VK_BACK;
unsigned char const KEYCODE_INSERT			= VK_INSERT;
unsigned char const KEYCODE_DELETE			= VK_DELETE;
unsigned char const KEYCODE_HOME			= VK_HOME;
unsigned char const KEYCODE_END				= VK_END;
unsigned char const KEYCODE_SHIFT			= VK_SHIFT;
unsigned char const KEYCODE_CONTROL			= VK_CONTROL;

unsigned char const KEYCODE_TILDE			= 0xC0;
unsigned char const KEYCODE_LEFTBRACKET		= 0xDB;
unsigned char const KEYCODE_RIGHTBRACKET	= 0xDD;
unsigned char const KEYCODE_1				= 0x31;
unsigned char const KEYCODE_2				= 0x32;
unsigned char const KEYCODE_3				= 0x33;
unsigned char const KEYCODE_4				= 0x34;
unsigned char const KEYCODE_5				= 0x35;
unsigned char const KEYCODE_6				= 0x36;
unsigned char const KEYCODE_7				= 0x37;
unsigned char const KEYCODE_8				= 0x38;
unsigned char const KEYCODE_9				= 0x39;
unsigned char const KEYCODE_0				= 0x30;




extern InputSystem* g_theInput;
extern EventSystem* g_theEventSystem;
extern DevConsole* g_theDevConsole;
extern Window* g_theWindow;

InputSystem::InputSystem(InputConfig const& config)
{
	(void)config;
}

InputSystem::~InputSystem()
{
}

void InputSystem::Startup()
{
	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; ++controllerIndex)
	{
		m_controllers[controllerIndex].m_id = controllerIndex;
	}
	g_theEventSystem->SubscribeEventCallbackFunction("KeyPressed", InputSystem::Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("KeyReleased", InputSystem::Event_KeyReleased);
}

void InputSystem::Shutdown()
{
}

void InputSystem::BeginFrame()
{
	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; ++controllerIndex)
	{
		m_controllers[controllerIndex].Update();
	}
	bool isCursorVisible = (m_cursorState.m_cursorMode == CursorMode::POINTER);
	while (ShowCursor(isCursorVisible) >= 0 && !isCursorVisible);
	while (ShowCursor(isCursorVisible) < 0 && isCursorVisible);

	IntVec2 prevCursorPos = m_cursorState.m_cursorClientPosition;
	m_cursorState.m_cursorClientPosition = IntVec2((int)GetCursorClientPosition().x, (int)GetCursorClientPosition().y);
	m_cursorState.m_cursorClientDelta = m_cursorState.m_cursorClientPosition - prevCursorPos;

	static CursorMode prevCursorMode = m_cursorState.m_cursorMode;

	if (m_cursorState.m_cursorMode == CursorMode::FPS)
	{
		IntVec2 centerPos = g_theWindow->GetClientDimensions();
		IntVec2 windowCenter(centerPos.x / 2, centerPos.y / 2);

		if (prevCursorMode == CursorMode::POINTER)
		{
			m_cursorState.m_cursorClientDelta = IntVec2(0, 0);
		}
		else
		{
			m_cursorState.m_cursorClientDelta = m_cursorState.m_cursorClientPosition - windowCenter;
		}

		POINT screenCenter = { windowCenter.x, windowCenter.y };
		ClientToScreen((HWND)g_theWindow->GetHwnd(), &screenCenter);
		SetCursorPos(screenCenter.x, screenCenter.y);

		m_cursorState.m_cursorClientPosition = windowCenter;
	}

	prevCursorMode = m_cursorState.m_cursorMode;
}



void InputSystem::EndFrame()
{
	for (int keyIndex = 0; keyIndex < 256; ++keyIndex)
	{
		m_keyState[keyIndex].wasKeyJustPressed = m_keyState[keyIndex].isKeyDown;

	}
}

bool InputSystem::WasKeyJustPressed(unsigned char keyCode) const
{
	return m_keyState[keyCode].isKeyDown && !m_keyState[keyCode].wasKeyJustPressed;
}

bool InputSystem::WasKeyJustReleased(unsigned char keyCode) const
{
	return !m_keyState[keyCode].isKeyDown && m_keyState[keyCode].wasKeyJustPressed;
}

bool InputSystem::IsKeyDown(unsigned char keyCode) const
{
	return m_keyState[keyCode].isKeyDown;
}

void InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	m_keyState[keyCode].isKeyDown = true;
}

void InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	m_keyState[keyCode].isKeyDown = false;
}


XboxController const& InputSystem::GetController(int controllerID)
{
	return m_controllers[controllerID];
}

void InputSystem::SetCursorMode(CursorMode cursorMode)
{
	if (m_cursorState.m_cursorMode == cursorMode) {
		return;
	}

	m_cursorState.m_cursorMode = cursorMode;

	if (cursorMode == CursorMode::FPS) {
		while (ShowCursor(FALSE) >= 0);
		ClipCursor(nullptr);
	}
	else {
		while (ShowCursor(TRUE) < 0);
		ClipCursor(nullptr);
	}
}


Vec2 InputSystem::GetCursorClientDelta() const
{
	if (m_cursorState.m_cursorMode == CursorMode::FPS)
	{
		return Vec2((float)m_cursorState.m_cursorClientDelta.x, (float)m_cursorState.m_cursorClientDelta.y);
	}
	else
	{
		return Vec2();
	}
}

Vec2 InputSystem::GetCursorClientPosition() const
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient((HWND)g_theWindow->GetHwnd(), &cursorPos);

	return Vec2((float)cursorPos.x, (float)cursorPos.y);
}


Vec2 InputSystem::GetCursorNormalizedPosition() const
{
	IntVec2 clientPos = IntVec2((int)GetCursorClientPosition().x, (int)GetCursorClientPosition().y);
	IntVec2 windowSize = g_theWindow->GetClientDimensions();

	if (windowSize.x == 0 || windowSize.y == 0) {
		return Vec2(0.5f, 0.5f);
	}

	float normalizedX = (float)clientPos.x / (float)windowSize.x;
	float normalizedY = 1.0f - (float)clientPos.y / (float)windowSize.y;

	return Vec2(normalizedX, normalizedY);
}


bool InputSystem::Event_KeyPressed(EventArgs& args)
{
	if (!g_theInput)
	{
		return false;
	}
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	if (g_theDevConsole && g_theDevConsole->IsOpen())
	{
		return DevConsole::Event_KeyPressed(args);
	}
	g_theInput->HandleKeyPressed(keyCode);
	return true;
}

bool InputSystem::Event_KeyReleased(EventArgs& args)
{
	if (!g_theInput)
	{
		return false;
	}
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	g_theInput->HandleKeyReleased(keyCode);
	return true;
}

