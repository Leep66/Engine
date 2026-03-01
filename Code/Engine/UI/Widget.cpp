#include "Engine/UI/Widget.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

extern BitmapFont* g_theFont;
extern Renderer* g_theRenderer;
//--------------------------------------------------------------
Widget::Widget()
{
}

Widget::Widget(const AABB2& bounds, const std::string& text /*= ""*/, Rgba8 color /*= Rgba8::WHITE*/, Texture* texture /*= nullptr*/)
	: m_bounds(bounds)
	, m_text(text)
	, m_color(color)
	, m_texture(texture)
{
	Vec2 dims = m_bounds.GetDimensions();

	float widthPercent = 0.8f;   
	float heightPercent = 0.2f; 
	float offsetYPercent = 0.3f; 

	Vec2 textDims = Vec2(dims.x * widthPercent, dims.y * heightPercent);

	m_textBounds = AABB2(Vec2::ZERO, textDims);
	m_textBounds.SetCenter(m_bounds.GetCenter());
	m_textBounds.Translate(Vec2(0.f, dims.y * offsetYPercent));
}

//--------------------------------------------------------------
Widget::~Widget()
{
}

//--------------------------------------------------------------
void Widget::Update(float deltaSeconds)
{
	if (!m_isEnabled) return;
	for (Widget* w : m_children)
	{
		if (!w) continue;
		w->Update(deltaSeconds);
	}
}

//--------------------------------------------------------------
void Widget::Render() const
{
	if (!m_isEnabled) return;

	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D(verts, m_bounds, m_color);

	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(verts);

	//----------------------------------------------
	// Draw text
	AABB2 textBounds = m_textBounds;

	std::vector<Vertex_PCU> textVerts;
	g_theFont->AddVertsForShadowTextInBox2D(textVerts, m_text, textBounds, m_textHeight, m_textColor, 0.7f, m_textAlignment);

	g_theRenderer->BindTexture(&g_theFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);

	//----------------------------------------------
	// Draw children
	for (Widget* w : m_children)
	{
		w->Render();
	}
}

void Widget::ToggleOpen()
{
	m_isEnabled = !m_isEnabled;

	for (Widget* w : m_children)
	{
		w->m_isEnabled = !w->m_isEnabled;
	}
}

void Widget::Enable()
{
	m_isEnabled = true;

	for (Widget* w : m_children)
	{
		w->m_isEnabled = true;
	}
}

void Widget::Disable()
{
	m_isEnabled = false;

	for (Widget* w : m_children)
	{
		w->m_isEnabled = false;
	}
}

void Widget::AddChild(Widget* widget)
{
	m_children.push_back(widget);
}

void Widget::SetText(const std::string& text, const AABB2& bounds, float textHeight, Vec2 alignment /*= Vec2(0.5f, 0.5f)*/, const Rgba8& textColor /*= Rgba8::LIGHT_GRAY*/)
{
	m_text = text;
	m_textBounds = bounds;
	m_textHeight = textHeight;
	m_textAlignment = alignment;
	m_textColor = textColor;
}

