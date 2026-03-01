#include "Button.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Window/Window.hpp"

extern Window* g_theWindow;
extern InputSystem* g_theInput;
extern Renderer* g_theRenderer;
extern BitmapFont* g_theFont;
extern AudioSystem* g_theAudio;

Button::Button(const AABB2& bounds, const std::string& text, Camera* camera, Texture* texture /*= nullptr*/, Rgba8 const& color /*= Rgba8::WHITE*/, SoundID sound /*= MISSING_SOUND_ID*/)
	: Widget(bounds, text, color, texture)
	, m_camera(camera)
{
	UpdateTextHeight();
	if (sound != MISSING_SOUND_ID)
	{
		m_clickSound = sound;
	}
}

void Button::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (!m_isEnabled) return;
	if (IsHovered())
	{
		if (m_onHover) m_onHover();
		if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
		{
			if (m_onPressed) m_onPressed();
		}
		if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_MOUSE) && IsClicked())
		{
			if (m_onClick)
			{
				m_onClick();

				if (m_clickSound != MISSING_SOUND_ID)
				{
					g_theAudio->StartSound(m_clickSound);
				}
				
			}
			if (m_onReleased) m_onReleased();
		}
	}
}

void Button::Render() const
{
	if (!m_isEnabled) return;
	std::vector<Vertex_PCU> verts;
	Rgba8 colorToUse = IsHovered() ? m_hoverColor : m_color;

	AddVertsForAABB2D(verts, m_bounds, colorToUse);

	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(verts);

	std::vector<Vertex_PCU> textVerts;
	g_theFont->AddVertsForTextInBox2D(
		textVerts,
		m_text,
		m_bounds,
		m_textHeight,
		Rgba8::WHITE,
		1.f,
		Vec2(0.5f, 0.5f)
	);

	g_theRenderer->BindTexture(&g_theFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);

	g_theRenderer->BindTexture(nullptr);
}


bool Button::IsHovered() const
{
	if (m_camera == nullptr)
		return false;

	Vec2 mousePos = g_theInput->GetCursorClientPosition();

	IntVec2 windowSize = g_theWindow->GetClientDimensions();
	Vec2 screenDims = m_camera->GetOrthographicBounds().GetDimensions();

	float scaleX = (float)windowSize.x / screenDims.x;
	float scaleY = (float)windowSize.y / screenDims.y;

	Vec2 mins = m_bounds.m_mins;
	Vec2 maxs = m_bounds.m_maxs;

	float windowMinY = windowSize.y - (maxs.y * scaleY);
	float windowMaxY = windowSize.y - (mins.y * scaleY);

	Vec2 windowMins(mins.x * scaleX, windowMinY);
	Vec2 windowMaxs(maxs.x * scaleX, windowMaxY);

	AABB2 windowBounds(windowMins, windowMaxs);

	return windowBounds.IsPointInside(mousePos);
}

bool Button::IsClicked() const
{
	return IsHovered() && g_theInput->WasKeyJustReleased(KEYCODE_LEFT_MOUSE);
}

void Button::UpdateTextHeight()
{
	Vec2 boxDimensions = m_bounds.GetDimensions();
	float boxHeight = boxDimensions.y;
	float boxWidth = boxDimensions.x;

	float paddingFraction = 0.3f;
	float paddingX = boxWidth * paddingFraction;
	float paddingY = boxHeight * paddingFraction;

	float usableWidth = boxWidth - paddingX * 2.f;
	float usableHeight = boxHeight - paddingY * 2.f;

	float maxTextHeight = usableHeight;
	float minTextHeight = 12.f;

	int numChars = (int)m_text.size();
	if (numChars == 0)
	{
		m_textHeight = minTextHeight;
		return;
	}

	float charWidthRatio = 0.6f;

	float estimatedCharWidth = maxTextHeight * charWidthRatio;

	float totalTextWidth = estimatedCharWidth * numChars;

	if (totalTextWidth > usableWidth)
	{
		float scale = usableWidth / totalTextWidth;
		m_textHeight = maxTextHeight * scale;

		if (m_textHeight < minTextHeight)
		{
			m_textHeight = minTextHeight;
		}
	}
	else
	{
		m_textHeight = maxTextHeight;
	}
}

