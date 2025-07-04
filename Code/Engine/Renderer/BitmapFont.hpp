#pragma once
#include <vector>
#include <string>
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/Vec2.hpp"


struct Vertex_PCU;
class Texture;

enum TextBoxMode
{
	SHRINK_TO_FIT,
	OVERRUN,

	NUM_TEXT_DRAW_MODE
};

class BitmapFont
{
	friend class Renderer;

private:
	BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture);

public:
	Texture& GetTexture();

	void AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins,
		float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspectScale = 1.f);
	void AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, std::string const& text, AABB2 const& box, float cellHeight,
		Rgba8 const& tint = Rgba8::WHITE, float cellAspectScale = 1.f, Vec2 const& alignment = Vec2(.5f, .5f), TextBoxMode mode = TextBoxMode::SHRINK_TO_FIT, 
		int maxGlyphsToDraw = 99999999, float paddingY = 0.f);

	void AddVertsForText3DAtOriginXForward(std::vector<Vertex_PCU>& verts, 
		float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, 
		float cellAspect = 1.f, Vec2 const& alignment = Vec2(0.5f, 0.5f), int maxGlyphsToDraw = 99999999);
	float GetTextWidth(float cellHeight, std::string const& text, float cellAspectScale = 1.f);

private:
	float GetGlyphAspect(int glyphUnicode) const; 

protected:
	std::string	m_fontFilePathNameWithNoExtension;
	SpriteSheet	m_fontGlyphsSpriteSheet;
	float		m_fontDefaultAspect = 1.0f;
};
