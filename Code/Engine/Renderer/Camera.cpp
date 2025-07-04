#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void Camera::SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float zNear, float zFar)
{
	m_orthographicBottomLeft = bottomLeft;
	m_orthographicTopRight = topRight;
	m_orthographicNear = zNear;
	m_orthographicFar = zFar;
}

void Camera::SetPerspectiveView(float aspect, float fov, float zNear, float zFar)
{
	m_perspectiveAspect = aspect;
	m_perspectiveFOV = fov;
	m_perspectiveNear = zNear;
	m_perspectiveFar = zFar;
}

void Camera::SetPositionAndOrientation(const Vec3& position, const EulerAngles& orientation)
{
	m_position = position;
	m_orientation = orientation;
}

void Camera::SetPosition(const Vec3& position)
{
	m_position = position;
}

Vec3 Camera::GetPosition() const
{
	return m_position;
}

void Camera::SetOrientation(const EulerAngles& orientation)
{
	m_orientation = orientation;
}

EulerAngles Camera::GetOrientation() const
{
	return m_orientation;
}

Mat44 Camera::GetCameraToWorldTransform() const
{
	Vec3 forward, left, up;
	m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

	Vec3 position = m_position;

	Mat44 cameraToWorld;
	cameraToWorld.SetIJKT3D(forward, left, up, position);

	return cameraToWorld;
}


Mat44 Camera::GetWorldToCameraTransform() const
{
	Mat44 cameraTransform = GetCameraToWorldTransform();
	return cameraTransform.GetOrthonormalInverse();
}

void Camera::SetCameraToRenderTransform(const Mat44& m)
{
	m_cameraToRenderTransfrom = m;
}

Mat44 Camera::GetCameraToRenderTransform() const
{
	return m_cameraToRenderTransfrom;
}

Mat44 Camera::GetRenderToClipTransform() const
{
	return GetProjectionMatrix();
}

Vec2 Camera::GetOrthographicBottomLeft() const
{
	return m_orthographicBottomLeft;
}

Vec2 Camera::GetOrthographicTopRight() const
{
	return m_orthographicTopRight;
}

void Camera::Translate2D(Vec2 const& translation)
{
	m_position.x += translation.x;
	m_position.y += translation.y;
}

Mat44 Camera::GetOrthographicMatrix() const
{
	float left = m_orthographicBottomLeft.x;
	float right = m_orthographicTopRight.x;
	float bottom = m_orthographicBottomLeft.y;
	float top = m_orthographicTopRight.y;
	float zNear = m_orthographicNear;
	float zFar = m_orthographicFar;

	return Mat44::MakeOrthoProjection(left, right, bottom, top, zNear, zFar);
}

Mat44 Camera::GetPerspectiveMatrix() const
{
	float fovYDegrees = m_perspectiveFOV;
	float aspect = m_perspectiveAspect;
	float zNear = m_perspectiveNear;
	float zFar = m_perspectiveFar;

	return Mat44::MakePerspectiveProjection(fovYDegrees, aspect, zNear, zFar);
}

Mat44 Camera::GetProjectionMatrix() const
{
	Mat44 projection;

	if (m_mode == eMode_Orthographic)
	{
		projection = GetOrthographicMatrix();
	}
	else if (m_mode == eMode_Perspective)
	{

		projection = GetPerspectiveMatrix();
	}

	return projection;
}	


