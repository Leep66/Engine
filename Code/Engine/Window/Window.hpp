#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/EventSystem.hpp"
#include <string>

class InputSystem;

struct WindowConfig
{
	float			m_aspectRatio = (16.f / 9.f);
	InputSystem*	m_inputSystem = nullptr;
	std::string		m_windowTitle = "Unnamed SD Engine Application";
};

class Window
{
public:
	Window(WindowConfig const& config);
	~Window();

	void Startup();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	WindowConfig const& GetConfig() const;
	void* GetDisplayContext() const;
	// Vec2 GetNormalizedMouseUV() const;

	void* GetHwnd() const;
	IntVec2 GetClientDimensions() const;

	float GetAspect() const { return m_config.m_aspectRatio; }

	bool HasFocus() const;

	static Window* s_mainWindow;

private:
	void CreateOSWindow();
	void RunMessagePump();
	


private:
	WindowConfig m_config;
	void* m_displayContext = nullptr;
	void* m_windowHandle = nullptr;
	IntVec2 m_clientDimensions;
};