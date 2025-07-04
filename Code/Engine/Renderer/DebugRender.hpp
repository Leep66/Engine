#pragma once
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <string>

enum class DebugRenderMode
{
	ALWAYS,
	USE_DEPTH,
	X_RAY,
};

struct DebugRenderConfig
{
	Renderer* m_renderer = nullptr;
	std::string m_fontName = "SquirrelFixedFont";
};

void DebugRenderSystemStartup(const DebugRenderConfig& config);
void DebugRenderSystemShutdown();

void DebugRenderSetVisible();
void DebugRenderSetHidden();
void DebugRenderClear();

void DebugRenderBeginFrame();
void DebugRenderWorld(const Camera& camera);
void DebugRenderScreen(const Camera& camera);
void DebugRenderEndFrame();

void DebugRenderWorldObjects();
void DebugRenderWorldTexts();

void DebugRenderWorldBillboardTexts(const Camera& camera);

void DebugAddRaycastResult(RaycastResult3D result,
	float rayRadius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH
);

void DebugAddWorldWireAABB3(AABB3 box,
	float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldPoint(const Vec3& pos, 
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE, 
	const Rgba8& endColor = Rgba8::WHITE, 
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldLine(const Vec3& start, const Vec3& end,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldWireSphere(const Vec3& center,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldArrow(const Vec3& start, const Vec3& end,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldText(const std::string& text,
	const Mat44& transform, float textHeight,
	float alignment, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldBillboardText(
	const std::string& text,
	const Vec3& origin, float textHeight,
	const Vec2& alignment, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH,
	BillboardType type = BillboardType::NONE);

void DebugAddWorldBasis(const Mat44& transform, float duration,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldCone(
	const Vec3& discCenter, const Vec3& tipPos,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH);

void DebugAddWorldWireCone(
	const Vec3& discCenter, const Vec3& tipPos,
	float radius, float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE,
	DebugRenderMode mode = DebugRenderMode::USE_DEPTH
);

void DebugAddScreenText(const std::string& text, const AABB2& box, float cellHeight,
	const Vec2& alignment, float duration,
	const Rgba8& start = Rgba8::WHITE, const Rgba8& endColor = Rgba8::WHITE);

void DebugAddMessage(const std::string& text,
	float duration,
	const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE);

bool Command_DebugRenderClear(EventArgs& args);
bool Command_DebugRenderToggle(EventArgs& args);




