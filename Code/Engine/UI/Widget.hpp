#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>
#include <string>

class Texture;

class Widget
{
public:
	Widget();
	Widget(const AABB2& bounds, const std::string& text = "", Rgba8 color = Rgba8::WHITE, Texture* texture = nullptr);
	virtual ~Widget();

	virtual void Update(float deltaSeconds);
	virtual void Render() const;

	void ToggleOpen();
	void Enable();
	void Disable();

	void AddChild(Widget* widget);
	void SetText(const std::string& text, const AABB2& bounds, float textHeight, Vec2 alignment = Vec2(0.5f, 0.5f), const Rgba8& textColor = Rgba8::LIGHT_GRAY);

public:
	bool m_isEnabled = false;
	AABB2 m_bounds;
	Rgba8 m_color = Rgba8::WHITE;
	Texture* m_texture = nullptr;
	std::vector<Widget*> m_children;

	std::string m_text = "";
	float m_textHeight = 50.f;
	AABB2 m_textBounds;
	Rgba8 m_textColor = Rgba8::LIGHT_GRAY;
	Vec2 m_textAlignment = Vec2(0.5f, 0.5f);
};
