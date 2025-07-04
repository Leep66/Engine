#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/AABB2.hpp"



class Camera
{
public:
	enum Mode
	{
		eMode_Orthographic,
		eMode_Perspective,

		eMode_Count
	};


	void SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float zNear = 0.0f, float zfar = 1.0f);
	void SetPerspectiveView(float aspect, float fov, float zNear, float zfar);

	void SetPositionAndOrientation(const Vec3& position, const EulerAngles& orientation);
	void SetPosition(const Vec3& position);
	Vec3 GetPosition() const;
	void SetOrientation(const EulerAngles& orientation);
	EulerAngles GetOrientation() const;

	Mat44 GetCameraToWorldTransform() const;

	Mat44 GetWorldToCameraTransform() const;

	void SetCameraToRenderTransform(const Mat44& m);
	Mat44 GetCameraToRenderTransform() const;

	Mat44 GetRenderToClipTransform() const;

	Vec2 GetOrthographicBottomLeft() const;
	Vec2 GetOrthographicTopRight() const;

	AABB2 GetOrthographicBounds() const { return AABB2(m_orthographicBottomLeft, m_orthographicTopRight); }
	void Translate2D(Vec2 const& translation);

	Mat44 GetOrthographicMatrix() const;
	Mat44 GetPerspectiveMatrix() const;
	Mat44 GetProjectionMatrix() const;

	Mode GetMode() const { return m_mode; }
	void SetMode(Mode mode) { m_mode = mode; }
	
	
protected:
	Mode m_mode = eMode_Orthographic;

	Vec3 m_position;
	EulerAngles m_orientation;

	Vec2 m_orthographicBottomLeft;
	Vec2 m_orthographicTopRight;
	float m_orthographicNear = 0.f;
	float m_orthographicFar = 1.f;

	float m_perspectiveAspect = 16.f / 9.f;
	float m_perspectiveFOV = 60.f;
	float m_perspectiveNear = 0.1f;
	float m_perspectiveFar = 100.f;

	Mat44 m_cameraToRenderTransfrom;

};