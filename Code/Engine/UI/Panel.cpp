#include "Engine/UI/Panel.hpp"

//--------------------------------------------------------------
Panel::Panel()
{
}

//--------------------------------------------------------------
Panel::~Panel()
{
    for (Widget* child : m_children)
    {
        delete child;
        child = nullptr;
    }
    m_children.clear();
}

//--------------------------------------------------------------
void Panel::Update(float deltaSeconds)
{
    for (Widget* child : m_children)
    {
        child->Update(deltaSeconds);
    }
}

//--------------------------------------------------------------
void Panel::Render() const
{
    for (Widget* child : m_children)
    {
        child->Render();
    }
}
