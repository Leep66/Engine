#pragma once
#include <vector>
#include "Engine/UI/Widget.hpp"

struct UISystemConfig
{
	float m_screenWidth = 1600.f;
	float m_screenHeight = 800.f;
};

class UISystem
{
public:
	UISystem();
	UISystem(UISystemConfig const& config);
	~UISystem();

	void Startup();
	void Shutdown();

	void Update(float deltaSeconds);
	void Render() const;

	void AddWidget(Widget* widget);

private:
	UISystemConfig m_config;

	std::vector<Widget*> m_widgets;
};
