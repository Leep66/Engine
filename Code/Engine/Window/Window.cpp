#include "Engine/Window/Window.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/backends/imgui_impl_win32.h"


Window* Window::s_mainWindow = nullptr;
extern DevConsole* g_theDevConsole;

//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called back by Windows whenever we tell it to (by calling DispatchMessage).
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(windowHandle, wmMessageCode, wParam, lParam))
		return 1;

	ImGuiIO* ioPtr = ImGui::GetCurrentContext() ? &ImGui::GetIO() : nullptr;
	const bool wantKbd = ioPtr && ioPtr->WantCaptureKeyboard;
	const bool wantMouse = ioPtr && ioPtr->WantCaptureMouse;

	InputSystem* input = nullptr;
	if (Window::s_mainWindow)
	{
		WindowConfig const& config = Window::s_mainWindow->GetConfig();
		input = config.m_inputSystem;
	}

	switch (wmMessageCode)
	{
	case WM_CLOSE:
	{
		FireEvent("quit");
		return 0;
	}
	case WM_KEYDOWN:
	{
		if (!wantKbd) {
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
			FireEvent("KeyPressed", args);
		}
		return 0;
	}
	case WM_KEYUP:
	{
		if (!wantKbd) {
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
			FireEvent("KeyReleased", args);
		}
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		if (!wantMouse) {
			EventArgs args; args.SetValue("KeyCode", Stringf("%d", (unsigned char)KEYCODE_LEFT_MOUSE));
			FireEvent("KeyPressed", args);
		}
		return 0;
	}
	case WM_LBUTTONUP:
	{
		if (!wantMouse) {
			EventArgs args; args.SetValue("KeyCode", Stringf("%d", (unsigned char)KEYCODE_LEFT_MOUSE));
			FireEvent("KeyReleased", args);
		}
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		if (!wantMouse) {
			EventArgs args; args.SetValue("KeyCode", Stringf("%d", (unsigned char)KEYCODE_RIGHT_MOUSE));
			FireEvent("KeyPressed", args);
		}
		return 0;
	}
	case WM_RBUTTONUP:
	{
		if (!wantMouse) {
			EventArgs args; args.SetValue("KeyCode", Stringf("%d", (unsigned char)KEYCODE_RIGHT_MOUSE));
			FireEvent("KeyReleased", args);
		}
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		if (!wantMouse) {
			short delta = GET_WHEEL_DELTA_WPARAM(wParam);
			EventArgs args; args.SetValue("WheelDelta", Stringf("%d", (int)delta));
			FireEvent("MouseWheel", args);
		}
		return 0;
	}
	case WM_CHAR:
	{
		if (!wantKbd && g_theDevConsole && g_theDevConsole->IsOpen())
		{
			EventArgs args;
			args.SetValue("Char", Stringf("%d", (unsigned char)wParam));
			FireEvent("CharInput", args);
		}
		return 0;
	}
	}

	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}

void Window::CreateOSWindow()
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	HMODULE applicationInstanceHandle = ::GetModuleHandle(NULL);
	float clientAspect = m_config.m_aspectRatio;

	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	// #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
	DWORD windowStyleFlags = WS_CAPTION | WS_BORDER /*| WS_THICKFRAME*/ | WS_SYSMENU | WS_OVERLAPPED;
	DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	
	GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 0.90f;
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;

	if (clientAspect > desktopAspect)
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / clientAspect;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * clientAspect;
	}

	m_clientDimensions = IntVec2(static_cast<int>(clientWidth), static_cast<int>(clientHeight));

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);


	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	if (m_config.m_isFullScreen)
	{
		windowStyleFlags = WS_POPUP;
		windowStyleExFlags = WS_EX_APPWINDOW;

		clientRect.left = 0;
		clientRect.top = 0;
		clientRect.right = (int)desktopWidth;
		clientRect.bottom = (int)desktopHeight;
	}


	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	if (!m_config.m_isFullScreen)
		AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	
	m_windowHandle = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL);

	ShowWindow(static_cast<HWND>(m_windowHandle), SW_SHOW);
	SetForegroundWindow(static_cast<HWND>(m_windowHandle));
	SetFocus(static_cast<HWND>(m_windowHandle));

	if (m_config.m_isFullScreen)
	{
		SetWindowPos(
			static_cast<HWND>(m_windowHandle),
			HWND_TOP,
			0, 0,
			(int)desktopWidth,
			(int)desktopHeight,
			SWP_NOZORDER | SWP_FRAMECHANGED
		);
	}

	m_displayContext = GetDC(static_cast<HWND>(m_windowHandle));

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);
}

Window::Window(WindowConfig const& config)
	:m_config(config)
{
	s_mainWindow = this;
	m_windowHandle = nullptr;
}

Window::~Window()
{
}

void Window::Startup()
{
	CreateOSWindow();
	
}

void Window::BeginFrame()
{
	RunMessagePump();
}

void Window::EndFrame()
{
}

void Window::ShutDown()
{
}

WindowConfig const& Window::GetConfig() const
{
	return m_config;
}

void* Window::GetDisplayContext() const
{
	return m_displayContext;
}

/*
Vec2 Window::GetNormalizedMouseUV() const
{
	HWND windowHandle = static_cast<HWND>(m_windowHandle);
	POINT cursorCoords;
	RECT clientRect;
	::GetCursorPos(&cursorCoords);
	::ScreenToClient(windowHandle, &cursorCoords);
	::GetClientRect(windowHandle, &clientRect);
	float cursorX = static_cast<float>(cursorCoords.x) / static_cast<float>(clientRect.right);
	float cursorY = static_cast<float>(cursorCoords.y) / static_cast<float>(clientRect.bottom);

	return Vec2(cursorX, 1.f - cursorY);
}*/

//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)

void Window::RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		BOOL const wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}

void* Window::GetHWND() const
{
	return m_windowHandle;
}

IntVec2 Window::GetClientDimensions() const
{
	return m_clientDimensions;
}

bool Window::HasFocus() const
{
	return GetForegroundWindow() == (HWND)GetHWND();
}

