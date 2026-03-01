#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/Widget.hpp"

//--------------------------------------------------------------
UISystem::UISystem()
    : m_config()
{
}

//--------------------------------------------------------------
UISystem::UISystem(UISystemConfig const& config)
    : m_config(config)
{
}

//--------------------------------------------------------------
UISystem::~UISystem()
{
    Shutdown();
}

//--------------------------------------------------------------
void UISystem::Startup()
{
}

//--------------------------------------------------------------
void UISystem::Shutdown()
{
    for (Widget* widget : m_widgets)
    {
        if (!widget) continue;
        delete widget;
        widget = nullptr;
    }
    m_widgets.clear();
}

//--------------------------------------------------------------
void UISystem::Update(float deltaSeconds)
{
    for (Widget* widget : m_widgets)
    {
        widget->Update(deltaSeconds);
    }
}

//--------------------------------------------------------------
void UISystem::Render() const
{
    for (Widget* widget : m_widgets)
    {
        widget->Render();
    }
}

//--------------------------------------------------------------
void UISystem::AddWidget(Widget* widget)
{
    m_widgets.push_back(widget);
}
