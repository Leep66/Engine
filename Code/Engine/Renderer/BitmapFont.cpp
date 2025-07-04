#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

BitmapFont::BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture)
	: m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension)
	, m_fontGlyphsSpriteSheet(fontTexture, IntVec2(16, 16))
{

}


Texture& BitmapFont::GetTexture()
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

void BitmapFont::AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspectScale)
{
	Vec2 cursor = textMins;

	for (char c : text)
	{
		const SpriteDefinition& SpriteDef = m_fontGlyphsSpriteSheet.GetSpriteDef(c);
		Vec2 uvMins, uvMaxs;
		SpriteDef.GetUVs(uvMins, uvMaxs);
		float glyphWidth = cellHeight * GetGlyphAspect(c) * cellAspectScale;
		AABB2 bounds(Vec2(cursor.x, cursor.y), Vec2(cursor.x + glyphWidth, cursor.y + cellHeight));
		AddVertsForAABB2D(vertexArray, bounds, tint, uvMins, uvMaxs);
		cursor.x += glyphWidth;
	}
}



void BitmapFont::AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, std::string const& text, AABB2 const& box, float cellHeight, Rgba8 const& tint, float cellAspectScale, Vec2 const& alignment, TextBoxMode mode, int maxGlyphsToDraw, float paddingY)
{
	Strings lines = SplitStringOnDelimiter(text, '\n');
	int lineCount = static_cast<int>(lines.size());
	std::vector<float> lineWidths(lineCount);

	float maxWidth = 0.f;
	for (int i = 0; i < lineCount; ++i) {
		lineWidths[i] = GetTextWidth(1.f, lines[i]);
		maxWidth = std::max(maxWidth, lineWidths[i]);
	}

	Vec2 textSize(cellAspectScale * cellHeight * maxWidth, cellHeight * lineCount);
	Vec2 boxSize = box.GetDimensions();

	if (mode == TextBoxMode::SHRINK_TO_FIT) {
		float scaleX = (textSize.x > boxSize.x) ? boxSize.x / textSize.x : 1.f;
		float scaleY = (textSize.y > boxSize.y) ? boxSize.y / textSize.y : 1.f;

		float scale = std::min(scaleX, scaleY);
		textSize *= scale;
		cellHeight *= scale;
	}

	Vec2 padding = boxSize - textSize;
	Vec2 offset = padding * alignment;
	Vec2 textPos = box.m_mins + offset;

	int totalGlyphs = 0;
	for (int i = 0; i < lineCount && totalGlyphs < maxGlyphsToDraw; ++i) {
		std::string& line = lines[i];
		int glyphsInLine = static_cast<int>(line.length());

		if (totalGlyphs + glyphsInLine > maxGlyphsToDraw) {
			glyphsInLine = maxGlyphsToDraw - totalGlyphs;
			line = line.substr(0, glyphsInLine);
		}

		totalGlyphs += glyphsInLine;

		float lineWidth = lineWidths[i] * cellHeight * cellAspectScale;
		float extraX = textSize.x - lineWidth;
		float alignX = GetClampedZeroToOne(alignment.x);
		float lineX = textPos.x + extraX * alignX;
		float lineY = textPos.y + (cellHeight + paddingY) * (lineCount - 1 - i);

		AddVertsForText2D(vertexArray, Vec2(lineX, lineY), cellHeight, line, tint, cellAspectScale);

		if (totalGlyphs >= maxGlyphsToDraw) {
			break;
		}
	}
}


void BitmapFont::AddVertsForText3DAtOriginXForward(std::vector<Vertex_PCU>& verts, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, int maxGlyphsToDraw)
{
	std::string maxGlyphsText = text;
	if (maxGlyphsToDraw > 0 && maxGlyphsToDraw < (int)text.length())
	{
		maxGlyphsText = text.substr(0, maxGlyphsToDraw);
	}

	std::vector<Vertex_PCU> textVerts2D;
	AddVertsForText2D(textVerts2D, Vec2::ZERO, cellHeight, maxGlyphsText, tint, cellAspect);

	float textWidth = GetTextWidth(cellHeight, maxGlyphsText, cellAspect);

	Vec2 startOffset = Vec2(-textWidth * alignment.x, -cellHeight * alignment.y);

	for (Vertex_PCU& vertex : textVerts2D)
	{
		Vec3 position3D = Vec3(0.f, vertex.m_position.x + startOffset.x, vertex.m_position.y + startOffset.y);
		verts.push_back(Vertex_PCU(position3D, vertex.m_color, vertex.m_uvTexCoords));
	}
}

float BitmapFont::GetTextWidth(float cellHeight, std::string const& text, float cellAspectScale)
{
	float totalWidth = 0.0f;
	for (char c : text)
	{
		float glyphAspect = GetGlyphAspect(static_cast<int>(c));
		totalWidth += cellHeight * glyphAspect * cellAspectScale;
	}
	return totalWidth;
}


float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	UNUSED (glyphUnicode)
	return 1.0f;
}
