#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/UI/Widget.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include <string>
#include <functional>

class Camera;
class Button : public Widget
{
public:
	Button(const AABB2& bounds, const std::string& text, Camera* camera, Texture* texture = nullptr, Rgba8 const& color = Rgba8::WHITE, SoundID sound = MISSING_SOUND_ID);

	void Update(float deltaSeconds);
	void Render() const;
	bool IsHovered() const;
	bool IsClicked() const;

	void UpdateTextHeight();

public:
	Rgba8 m_hoverColor = Rgba8(200, 200, 200);
	float m_textHeight = 0.f;
	Camera* m_camera = nullptr;

	std::function<void()> m_onClick;
	std::function<void()> m_onHover;
	std::function<void()> m_onPressed;
	std::function<void()> m_onReleased;
	SoundID m_clickSound;
};
