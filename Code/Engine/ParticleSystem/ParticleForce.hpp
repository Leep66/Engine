#pragma once
#include "Engine/Math/Vec3.hpp"
#include <cstdint>

enum : uint32_t
{
	FORCE_GRAVITY = 0,
	FORCE_POINT = 1,
	FORCE_DIRECTION = 2,
	FORCE_FLOWCOLUMN = 3
};

static constexpr uint32_t FLOW_SWIRL_ENABLE = (1u << 0);
static constexpr uint32_t FLOW_RADIAL_ENABLE = (1u << 1);
static constexpr uint32_t FLOW_AXIAL_ENABLE = (1u << 2);
static constexpr uint32_t FLOW_SWIRL_INVERT = (1u << 3);
static constexpr uint32_t FLOW_USE_INV_RADIUS = (1u << 4);
static constexpr uint32_t FLOW_RADIAL_OUTWARD = (1u << 5);

class ParticleForce
{
public:
	uint32_t m_forceType = FORCE_GRAVITY;
	uint32_t m_enabled = 0;
	uint32_t m_flags = 0;
	float m_param0 = 0.f;

	float m_direction[3] = { 0.f, 0.f, 0.f };
	float m_strength = 0.f;

	float m_position[3] = { 0.f, 0.f, 0.f };
	float m_range = 0.f;

	float m_bottomRadius = 1.0f;
	float m_topRadius = 4.0f;
	float m_axialStrength = 0.f;
	float m_radialStrength = 0.f;

	float m_radialFalloffPow = 2.0f;
	float m_heightFalloffPow = 0.0f;
	float m_pad0 = 0.f; 
	float m_pad1 = 0.f;

public:
	ParticleForce() = default;

	static ParticleForce MakeParticleGravity(const Vec3& dir, float strength)
	{
		ParticleForce f;
		f.m_forceType = FORCE_GRAVITY;
		Vec3 n = dir.GetNormalized();
		if (n.GetLengthSquared() < 1e-6f) n = Vec3(0, 0, -1);
		f.m_direction[0] = n.x; f.m_direction[1] = n.y; f.m_direction[2] = n.z;
		f.m_strength = strength;
		f.m_enabled = 1u;
		return f;
	}

	static ParticleForce MakeParticleDirectionForce(
		const Vec3& dir,
		float strength,
		float range = 0.0f,
		const Vec3& position = Vec3(0.f, 0.f, 0.f)
	)
	{
		ParticleForce f;
		f.m_forceType = FORCE_DIRECTION;

		Vec3 n = dir.GetNormalized();
		if (n.GetLengthSquared() < 1e-6f) n = Vec3(1, 0, 0);

		f.m_direction[0] = n.x;
		f.m_direction[1] = n.y;
		f.m_direction[2] = n.z;

		f.m_strength = strength;
		f.m_range = range;
		f.m_position[0] = position.x;
		f.m_position[1] = position.y;
		f.m_position[2] = position.z;

		f.m_enabled = 1u;

		return f;
	}

	static ParticleForce MakeParticlePointForce(const Vec3& pos, float strength, float range)
	{
		ParticleForce f;
		f.m_forceType = FORCE_POINT;
		f.m_position[0] = pos.x; f.m_position[1] = pos.y; f.m_position[2] = pos.z;
		f.m_strength = strength;
		f.m_range = range;
		f.m_enabled = 1u;
		return f;
	}

	static ParticleForce MakeFlowColumn(
		const Vec3& basePos = Vec3(0, 0, 0),
		const Vec3& axisDir = Vec3(0, 0, 1),
		float height = 8.0f,
		float bottomRadius = 1.5f,
		float topRadius = 5.0f,
		float swirlStrength = 12.0f,
		float axialStrength = 6.0f,
		float radialStrength = 3.0f,
		float radialFalloffPow = 4.0f,
		float heightFalloffPow = 0.0f,
		uint32_t flags = 0u)
	{
		ParticleForce f;
		f.m_forceType = FORCE_FLOWCOLUMN;
		f.m_enabled = 1u;

		f.m_position[0] = basePos.x; f.m_position[1] = basePos.y; f.m_position[2] = basePos.z;

		Vec3 n = axisDir.GetNormalized();
		if (n.GetLengthSquared() < 1e-6f) n = Vec3(0, 0, 1);
		f.m_direction[0] = n.x; f.m_direction[1] = n.y; f.m_direction[2] = n.z;

		f.m_range = height;
		f.m_bottomRadius = bottomRadius;
		f.m_topRadius = topRadius;
		f.m_strength = swirlStrength;
		f.m_axialStrength = axialStrength;
		f.m_radialStrength = radialStrength;
		f.m_radialFalloffPow = radialFalloffPow;
		f.m_heightFalloffPow = heightFalloffPow;
		f.m_flags = flags;

		return f;
	}

	void ToggleEnabled() { m_enabled = (m_enabled != 0u) ? 0u : 1u; }
	void SetEnabled(bool enabled) { m_enabled = enabled ? 1u : 0u; }
	void SetDirection(const Vec3& dir)
	{
		Vec3 n = dir.GetNormalized();
		if (n.GetLengthSquared() < 1e-6f) n = Vec3(0, 0, 1);
		m_direction[0] = n.x; m_direction[1] = n.y; m_direction[2] = n.z;
	}
	void SetPosition(const Vec3& pos)
	{
		m_position[0] = pos.x; m_position[1] = pos.y; m_position[2] = pos.z;
	}
};

static_assert(sizeof(ParticleForce) == 80, "ParticleForce must be multiple of 16 bytes");