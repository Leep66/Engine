#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>


Mat44::Mat44()
{
	m_values[Ix] = 1.0f;
	m_values[Iy] = 0.0f;
	m_values[Iz] = 0.0f;
	m_values[Iw] = 0.0f;

	m_values[Jx] = 0.0f;
	m_values[Jy] = 1.0f;
	m_values[Jz] = 0.0f;
	m_values[Jw] = 0.0f;

	m_values[Kx] = 0.0f;
	m_values[Ky] = 0.0f;
	m_values[Kz] = 1.0f;
	m_values[Kw] = 0.0f;

	m_values[Tx] = 0.0f;
	m_values[Ty] = 0.0f;
	m_values[Tz] = 0.0f;
	m_values[Tw] = 1.0f;
}

Mat44::Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.0f;
	m_values[Iw] = 0.0f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.0f;
	m_values[Jw] = 0.0f;

	m_values[Kx] = 0.0f;
	m_values[Ky] = 0.0f;
	m_values[Kz] = 1.0f;
	m_values[Kw] = 0.0f;

	m_values[Tx] = translation2D.x;
	m_values[Ty] = translation2D.y;
	m_values[Tz] = 0.0f;
	m_values[Tw] = 1.0f;
}

Mat44::Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.0f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.0f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.0f;

	m_values[Tx] = translation3D.x;
	m_values[Ty] = translation3D.y;
	m_values[Tz] = translation3D.z;
	m_values[Tw] = 1.0f;
}

Mat44::Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

Mat44::Mat44(float const* sixteenValuesBasisMajor)
{
	for (int i = 0; i < 16; ++i) {
		m_values[i] = sixteenValuesBasisMajor[i];
	}
}

Mat44 const Mat44::MakeTranslation2D(Vec2 const& translationXY)
{
	Mat44 result;

	result.m_values[Tx] = translationXY.x;
	result.m_values[Ty] = translationXY.y;

	return result;
}


Mat44 const Mat44::MakeTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 result;

	result.m_values[Tx] = translationXYZ.x;
	result.m_values[Ty] = translationXYZ.y;
	result.m_values[Tz] = translationXYZ.z;

	return result;
}

Mat44 const Mat44::MakeUniformScale2D(float uniformScaleXY)
{
	Mat44 result;

	result.m_values[Ix] = uniformScaleXY;
	result.m_values[Jy] = uniformScaleXY;

	return result;
}

Mat44 const Mat44::MakeUniformScale3D(float uniformScaleXYZ)
{
	Mat44 result;

	result.m_values[Ix] = uniformScaleXYZ;
	result.m_values[Jy] = uniformScaleXYZ;
	result.m_values[Kz] = uniformScaleXYZ;

	return result;
}


Mat44 const Mat44::MakeNonUniformScale2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 result;

	result.m_values[Ix] = nonUniformScaleXY.x;
	result.m_values[Jy] = nonUniformScaleXY.y;

	return result;
}

Mat44 const Mat44::MakeNonUniformScale3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 result;

	result.m_values[Ix] = nonUniformScaleXYZ.x;
	result.m_values[Jy] = nonUniformScaleXYZ.y;
	result.m_values[Kz] = nonUniformScaleXYZ.z;

	return result;
}


Mat44 const Mat44::MakeZRotationDegrees(float rotationDegreesAboutZ)
{
	float rotationRadians = ConvertDegreesToRadians(rotationDegreesAboutZ);

	Mat44 result;

	result.m_values[Ix] = cosf(rotationRadians);
	result.m_values[Iy] = sinf(rotationRadians);

	result.m_values[Jx] = -sinf(rotationRadians);
	result.m_values[Jy] = cosf(rotationRadians);

	return result;
}




Mat44 const Mat44::MakeYRotationDegrees(float rotationDegreesAboutY)
{
	float rotationRadians = ConvertDegreesToRadians(rotationDegreesAboutY);

	Mat44 result;

	result.m_values[Ix] = cosf(rotationRadians);
	result.m_values[Iz] = -sinf(rotationRadians);

	result.m_values[Kx] = sinf(rotationRadians);
	result.m_values[Kz] = cosf(rotationRadians);

	return result;
}



Mat44 const Mat44::MakeXRotationDegrees(float rotationDegreesAboutX)
{
	float rotationRadians = ConvertDegreesToRadians(rotationDegreesAboutX);

	Mat44 result;

	result.m_values[Jy] = cosf(rotationRadians);
	result.m_values[Jz] = sinf(rotationRadians);
	
	result.m_values[Ky] = -sinf(rotationRadians);
	result.m_values[Kz] = cosf(rotationRadians);


	return result;
}

Mat44 const Mat44::MakeOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Mat44 orthoMat;
	orthoMat.m_values[Ix] = 2.f / (right - left);
	orthoMat.m_values[Jy] = 2.f / (top - bottom);
	orthoMat.m_values[Kz] = 1.f / (zFar - zNear);

	orthoMat.m_values[Tx] = -(right + left) / (right - left);
	orthoMat.m_values[Ty] = -(top + bottom) / (top - bottom);
	orthoMat.m_values[Tz] = -zNear / (zFar - zNear);

	orthoMat.m_values[Tw] = 1.f;

	return orthoMat;
}

Mat44 const Mat44::MakePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar)
{
	Mat44 result;

	float c = CosDegrees(fovYDegrees * 0.5f);
	float s = SinDegrees(fovYDegrees * 0.5f);
	float scaleY = c / s;
	float scaleX = scaleY / aspect;
	float scaleZ = zFar / (zFar - zNear);
	float translateZ = (zNear * zFar) / (zNear - zFar);

	result.m_values[Ix] = scaleX;
	result.m_values[Jy] = scaleY;
	result.m_values[Kz] = scaleZ;
	result.m_values[Kw] = 1.0f;
	result.m_values[Tz] = translateZ;
	result.m_values[Tw] = 0.0f;

	return result;
}


Vec2 const Mat44::TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const
{
	float x = vectorQuantityXY.x;
	float y = vectorQuantityXY.y;

	float transformedX = m_values[Ix] * x + m_values[Jx] * y;
	float transformedY = m_values[Iy] * x + m_values[Jy] * y;

	return Vec2(transformedX, transformedY);
}


Vec3 const Mat44::TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const
{
	float x = vectorQuantityXYZ.x;
	float y = vectorQuantityXYZ.y;
	float z = vectorQuantityXYZ.z;

	float transformedX = m_values[Ix] * x + m_values[Jx] * y + m_values[Kx] * z;
	float transformedY = m_values[Iy] * x + m_values[Jy] * y + m_values[Ky] * z;
	float transformedZ = m_values[Iz] * x + m_values[Jz] * y + m_values[Kz] * z;

	return Vec3(transformedX, transformedY, transformedZ);
}

Vec2 const Mat44::TransformPosition2D(Vec2 const& positionXY) const
{
	float x = positionXY.x;
	float y = positionXY.y;

	float transformedX = m_values[Ix] * x + m_values[Jx] * y + m_values[Tx];
	float transformedY = m_values[Iy] * x + m_values[Jy] * y + m_values[Ty];

	return Vec2(transformedX, transformedY);
}


Vec3 const Mat44::TransformPosition3D(Vec3 const& position3D) const
{
	float x = position3D.x;
	float y = position3D.y;
	float z = position3D.z;

	float transformedX = m_values[Ix] * x + m_values[Jx] * y + m_values[Kx] * z + m_values[Tx];
	float transformedY = m_values[Iy] * x + m_values[Jy] * y + m_values[Ky] * z + m_values[Ty];
	float transformedZ = m_values[Iz] * x + m_values[Jz] * y + m_values[Kz] * z + m_values[Tz];

	return Vec3(transformedX, transformedY, transformedZ);
}


Vec4 const Mat44::TransformHomogeneous3D(Vec4 const& homogeneousPoint3D) const
{
	float x = homogeneousPoint3D.x;
	float y = homogeneousPoint3D.y;
	float z = homogeneousPoint3D.z;
	float w = homogeneousPoint3D.w;

	float transformedX = m_values[Ix] * x + m_values[Jx] * y + m_values[Kx] * z + m_values[Tx] * w;
	float transformedY = m_values[Iy] * x + m_values[Jy] * y + m_values[Ky] * z + m_values[Ty] * w;
	float transformedZ = m_values[Iz] * x + m_values[Jz] * y + m_values[Kz] * z + m_values[Tz] * w;
	float transformedW = m_values[Iw] * x + m_values[Jw] * y + m_values[Kw] * z + m_values[Tw] * w;

	return Vec4(transformedX, transformedY, transformedZ, transformedW);
}


float* Mat44::GetAsFloatArray()
{
	return &m_values[0];
}

float const* Mat44::GetAsFloatArray() const
{
	return m_values;
}

Vec2 const Mat44::GetIBasis2D() const
{
	return Vec2(m_values[Ix], m_values[Iy]);
}


Vec2 const Mat44::GetJBasis2D() const
{
	return Vec2(m_values[Jx], m_values[Jy]);
}


Vec2 const Mat44::GetTranslation2D() const
{
	return Vec2(m_values[Tx], m_values[Ty]);
}


Vec3 const Mat44::GetIBasis3D() const
{
	return Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
}

Vec3 const Mat44::GetJBasis3D() const
{
	return Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
}

Vec3 const Mat44::GetKBasis3D() const
{
	return Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
}

Vec3 const Mat44::GetTranslation3D() const
{
	return Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);
}

Vec4 const Mat44::GetIBasis4D() const
{
	return Vec4(m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw]);
}

Vec4 const Mat44::GetJBasis4D() const
{
	return Vec4(m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw]);
}

Vec4 const Mat44::GetKBasis4D() const
{
	return Vec4(m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw]);
}

Vec4 const Mat44::GetTranslation4D() const
{
	return Vec4(m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw]);
}

Mat44 const Mat44::GetOrthonormalInverse() const
{
	Mat44 result;

	result.m_values[Ix] = m_values[Ix];
	result.m_values[Iy] = m_values[Jx];
	result.m_values[Iz] = m_values[Kx];
	result.m_values[Iw] = 0.0f;

	result.m_values[Jx] = m_values[Iy];
	result.m_values[Jy] = m_values[Jy];
	result.m_values[Jz] = m_values[Ky];
	result.m_values[Jw] = 0.0f;

	result.m_values[Kx] = m_values[Iz];
	result.m_values[Ky] = m_values[Jz];
	result.m_values[Kz] = m_values[Kz];
	result.m_values[Kw] = 0.0f;

	result.m_values[Tx] = -(result.m_values[Ix] * m_values[Tx] + result.m_values[Jx] * m_values[Ty] + result.m_values[Kx] * m_values[Tz]);
	result.m_values[Ty] = -(result.m_values[Iy] * m_values[Tx] + result.m_values[Jy] * m_values[Ty] + result.m_values[Ky] * m_values[Tz]);
	result.m_values[Tz] = -(result.m_values[Iz] * m_values[Tx] + result.m_values[Jz] * m_values[Ty] + result.m_values[Kz] * m_values[Tz]);
	result.m_values[Tw] = 1.0f;

	return result;
}

void Mat44::SetTranslation2D(Vec2 const& translationXY)
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.0f;
	m_values[Tw] = 1.0f;
}

void Mat44::SetTranslation3D(Vec3 const& translationXYZ)
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.0f;
}

void Mat44::SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.0f;
	m_values[Iw] = 0.0f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.0f;
	m_values[Jw] = 0.0f;
}

void Mat44::SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY)
{
	SetIJ2D(iBasis2D, jBasis2D);
	SetTranslation2D(translationXY);
}

void Mat44::SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.0f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.0f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.0f;
}

void Mat44::SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ)
{
	SetIJK3D(iBasis3D, jBasis3D, kBasis3D);
	SetTranslation3D(translationXYZ);
}

void Mat44::SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

void Mat44::Transpose()
{
	float transposedValues[16] = 
	{	m_values[Ix],m_values[Jx],m_values[Kx],m_values[Tx],
		m_values[Iy],m_values[Jy],m_values[Ky],m_values[Ty],			
		m_values[Iz],m_values[Jz],m_values[Kz],m_values[Tz],			
		m_values[Iw],m_values[Jw],m_values[Kw],m_values[Tw] };

	for (int i = 0; i < 16; ++i) {
		m_values[i] = transposedValues[i];
	}

}

void Mat44::Orthonormalize_IFwd_JLeft_KUp()
{
	Vec3 i(m_values[Ix], m_values[Iy], m_values[Iz]);
	Vec3 j(m_values[Jx], m_values[Jy], m_values[Jz]);
	Vec3 k(m_values[Kx], m_values[Ky], m_values[Kz]);

	i = i.GetNormalized();

	float projectionJI = DotProduct3D(j, i);
	j -= i * projectionJI;

	j = j.GetNormalized();

	float projectionKI = DotProduct3D(k, i);
	float projectionKJ = DotProduct3D(k, j);
	k -= i * projectionKI + j * projectionKJ;

	k = k.GetNormalized();

	m_values[Ix] = i.x; m_values[Iy] = i.y; m_values[Iz] = i.z;
	m_values[Jx] = j.x; m_values[Jy] = j.y; m_values[Jz] = j.z;
	m_values[Kx] = k.x; m_values[Ky] = k.y; m_values[Kz] = k.z;
}


void Mat44::Append(Mat44 const& appendThis)
{
	Mat44 copyOfThis = *this;
	float const* left = &copyOfThis.m_values[0];
	float const* right = &appendThis.m_values[0];

	m_values[Ix] = (left[Ix] * right[Ix]) + (left[Jx] * right[Iy]) + (left[Kx] * right[Iz]) + (left[Tx] * right[Iw]);
	m_values[Iy] = (left[Iy] * right[Ix]) + (left[Jy] * right[Iy]) + (left[Ky] * right[Iz]) + (left[Ty] * right[Iw]);
	m_values[Iz] = (left[Iz] * right[Ix]) + (left[Jz] * right[Iy]) + (left[Kz] * right[Iz]) + (left[Tz] * right[Iw]);
	m_values[Iw] = (left[Iw] * right[Ix]) + (left[Jw] * right[Iy]) + (left[Kw] * right[Iz]) + (left[Tw] * right[Iw]);

	m_values[Jx] = (left[Ix] * right[Jx]) + (left[Jx] * right[Jy]) + (left[Kx] * right[Jz]) + (left[Tx] * right[Jw]);
	m_values[Jy] = (left[Iy] * right[Jx]) + (left[Jy] * right[Jy]) + (left[Ky] * right[Jz]) + (left[Ty] * right[Jw]);
	m_values[Jz] = (left[Iz] * right[Jx]) + (left[Jz] * right[Jy]) + (left[Kz] * right[Jz]) + (left[Tz] * right[Jw]);
	m_values[Jw] = (left[Iw] * right[Jx]) + (left[Jw] * right[Jy]) + (left[Kw] * right[Jz]) + (left[Tw] * right[Jw]);

	m_values[Kx] = (left[Ix] * right[Kx]) + (left[Jx] * right[Ky]) + (left[Kx] * right[Kz]) + (left[Tx] * right[Kw]);
	m_values[Ky] = (left[Iy] * right[Kx]) + (left[Jy] * right[Ky]) + (left[Ky] * right[Kz]) + (left[Ty] * right[Kw]);
	m_values[Kz] = (left[Iz] * right[Kx]) + (left[Jz] * right[Ky]) + (left[Kz] * right[Kz]) + (left[Tz] * right[Kw]);
	m_values[Kw] = (left[Iw] * right[Kx]) + (left[Jw] * right[Ky]) + (left[Kw] * right[Kz]) + (left[Tw] * right[Kw]);

	m_values[Tx] = (left[Ix] * right[Tx]) + (left[Jx] * right[Ty]) + (left[Kx] * right[Tz]) + (left[Tx] * right[Tw]);
	m_values[Ty] = (left[Iy] * right[Tx]) + (left[Jy] * right[Ty]) + (left[Ky] * right[Tz]) + (left[Ty] * right[Tw]);
	m_values[Tz] = (left[Iz] * right[Tx]) + (left[Jz] * right[Ty]) + (left[Kz] * right[Tz]) + (left[Tz] * right[Tw]);
	m_values[Tw] = (left[Iw] * right[Tx]) + (left[Jw] * right[Ty]) + (left[Kw] * right[Tz]) + (left[Tw] * right[Tw]);
}




void Mat44::AppendZRotation(float degreesRotationAboutZ)
{
	Mat44 zRotatedMatrix = MakeZRotationDegrees(degreesRotationAboutZ);

	Append(zRotatedMatrix);
}



void Mat44::AppendYRotation(float degreesRotationAboutY)
{
	Mat44 yRotatedMatrix = MakeYRotationDegrees(degreesRotationAboutY);

	Append(yRotatedMatrix);
}


void Mat44::AppendXRotation(float degreesRotationAboutX)
{
	Mat44 xRotatedMatrix = MakeXRotationDegrees(degreesRotationAboutX);

	Append(xRotatedMatrix);
}


void Mat44::AppendTranslation2D(Vec2 const& translationXY)
{
	Mat44 translatedMatrix = MakeTranslation2D(translationXY);

	Append(translatedMatrix);
}


void Mat44::AppendTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 translatedMatrix = MakeTranslation3D(translationXYZ);

	Append(translatedMatrix);
}


void Mat44::AppendScaleUniform2D(float uniformScaleXY)
{
	Mat44 scaledMatrix = MakeUniformScale2D(uniformScaleXY);

	Append(scaledMatrix);
}


void Mat44::AppendScaleUniform3D(float uniformScaleXYZ)
{
	Mat44 scaledMatrix = MakeUniformScale3D(uniformScaleXYZ);

	Append(scaledMatrix);
}


void Mat44::AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 scaledMatrix = MakeNonUniformScale2D(nonUniformScaleXY);

	Append(scaledMatrix);
}


void Mat44::AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 scaledMatrix = MakeNonUniformScale3D(nonUniformScaleXYZ);

	Append(scaledMatrix);
}

