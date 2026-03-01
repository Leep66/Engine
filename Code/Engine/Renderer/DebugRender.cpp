#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

Renderer* s_theRenderer = nullptr;

namespace
{

	std::recursive_mutex s_debugRenderMutex;

	struct DebugObject
	{
		Rgba8 m_startColor = Rgba8::WHITE;
		Rgba8 m_endColor = Rgba8::WHITE;
		float m_duration = -1.f;

		RasterizerMode m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
		DebugRenderMode m_mode = DebugRenderMode::USE_DEPTH;
		Timer* m_timer = nullptr;

		BillboardType m_type = BillboardType::NONE;
		Vec3 m_origin;

		std::vector<Vertex_PCU> m_verts;
		Texture* m_tex = nullptr;

		bool m_singleFrame = false;
	};


	std::string s_fontName;
	BitmapFont* s_theFont = nullptr;

	bool s_isVisible = true;

	std::vector<DebugObject> s_debugObjects;

	std::vector<DebugObject> s_worldDebugTexts;
	std::vector<DebugObject> s_worldBillboardTexts;

	std::vector<DebugObject> s_screenDebugTexts;
	std::vector<DebugObject> s_screenMessages;

	Clock* s_debugRenderClock = new Clock();


}

void DebugRenderSystemStartup(const DebugRenderConfig& config)
{
	s_theRenderer = config.m_renderer;
	s_fontName = config.m_fontName;

	std::string s_fontPath = "Data/Fonts/" + s_fontName;
	s_theFont = s_theRenderer->CreateOrGetBitmapFont(s_fontPath.c_str());
}

void DebugRenderSystemShutdown()
{
	s_theRenderer = nullptr;
	for (DebugObject obj : s_debugObjects)
	{
		obj.m_verts.clear();
		delete obj.m_timer;
		obj.m_timer = nullptr;
	}

	for (DebugObject text : s_worldDebugTexts)
	{
		text.m_verts.clear();
		delete text.m_timer;
		text.m_timer = nullptr;
	}

	for (DebugObject text : s_screenDebugTexts)
	{
		text.m_verts.clear();
		delete text.m_timer;
		text.m_timer = nullptr;
	}

	for (DebugObject message : s_screenMessages)
	{
		message.m_verts.clear();
		delete message.m_timer;
		message.m_timer = nullptr;
	}

	s_debugObjects.clear();
	s_worldDebugTexts.clear();
	s_screenDebugTexts.clear();
	s_screenMessages.clear();

}

void DebugRenderSetVisible()
{
	s_isVisible = true;
}

void DebugRenderSetHidden()
{
	s_isVisible = false;
}

void ToggleDebugRenderMode()
{
	s_isVisible = !s_isVisible;
}

bool IsDebugRender()
{
	return s_isVisible;
}

void DebugRenderClear()
{
	s_debugObjects.clear();
	s_worldDebugTexts.clear();
	s_worldBillboardTexts.clear();
	s_screenDebugTexts.clear();
	s_screenMessages.clear();
}

void DebugRenderBeginFrame()
{
	std::scoped_lock lock(s_debugRenderMutex);

	for (DebugObject& obj : s_debugObjects)
	{
		if (obj.m_singleFrame) continue;
		if (!obj.m_timer) continue;

		float elapsedFraction = obj.m_timer->GetElapsedFraction();

		if (obj.m_duration > 0.f)
		{
			Rgba8 newColor;
			newColor.r = (unsigned char)RangeMapClamped(elapsedFraction, 0.f, 1.f, obj.m_startColor.r, obj.m_endColor.r);
			newColor.g = (unsigned char)RangeMapClamped(elapsedFraction, 0.f, 1.f, obj.m_startColor.g, obj.m_endColor.g);
			newColor.b = (unsigned char)RangeMapClamped(elapsedFraction, 0.f, 1.f, obj.m_startColor.b, obj.m_endColor.b);
			newColor.a = (unsigned char)RangeMapClamped(elapsedFraction, 0.f, 1.f, obj.m_startColor.a, obj.m_endColor.a);

			for (Vertex_PCU& vert : obj.m_verts)
			{
				vert.m_color = newColor;
			}
		}
	}

	s_debugObjects.erase(
		std::remove_if(
			s_debugObjects.begin(),
			s_debugObjects.end(),
			[](DebugObject& obj)
			{
				if (obj.m_singleFrame) return true;
				if (obj.m_timer && obj.m_duration > 0.f && obj.m_timer->GetElapsedTime() >= obj.m_duration)
				{
					delete obj.m_timer;
					obj.m_timer = nullptr;
					return true;
				}
				return false;
			}
		),
		s_debugObjects.end()
	);

	for (DebugObject& obj : s_worldBillboardTexts)
	{
		if (obj.m_singleFrame) continue;
		if (!obj.m_timer) continue;

		float elapsedFraction = obj.m_timer->GetElapsedFraction();

		if (obj.m_duration > 0.f)
		{
			Rgba8 newColor;
			newColor.r = (unsigned char)RangeMapClamped(elapsedFraction, 0.f, 1.f, obj.m_startColor.r, obj.m_endColor.r);
			newColor.g = (unsigned char)RangeMapClamped(elapsedFraction, 0.f, 1.f, obj.m_startColor.g, obj.m_endColor.g);
			newColor.b = (unsigned char)RangeMapClamped(elapsedFraction, 0.f, 1.f, obj.m_startColor.b, obj.m_endColor.b);
			newColor.a = (unsigned char)RangeMapClamped(elapsedFraction, 0.f, 1.f, obj.m_startColor.a, obj.m_endColor.a);

			for (Vertex_PCU& vert : obj.m_verts)
			{
				vert.m_color = newColor;
			}
		}
	}

	s_worldBillboardTexts.erase(
		std::remove_if(
			s_worldBillboardTexts.begin(),
			s_worldBillboardTexts.end(),
			[](DebugObject& obj)
			{
				if (obj.m_singleFrame) return true;
				if (obj.m_timer && obj.m_duration > 0.f && obj.m_timer->GetElapsedTime() >= obj.m_duration)
				{
					delete obj.m_timer;
					obj.m_timer = nullptr;
					return true;
				}
				return false;
			}
		),
		s_worldBillboardTexts.end()
	);

	s_screenDebugTexts.clear();

	for (DebugObject& obj : s_screenMessages)
	{
		if (obj.m_singleFrame) continue;
		if (!obj.m_timer) continue;
	}

	s_screenMessages.erase(
		std::remove_if(
			s_screenMessages.begin(),
			s_screenMessages.end(),
			[](DebugObject& obj)
			{
				if (obj.m_singleFrame) return true;
				if (obj.m_timer && obj.m_duration > 0.f && obj.m_timer->GetElapsedTime() >= obj.m_duration)
				{
					delete obj.m_timer;
					obj.m_timer = nullptr;
					return true;
				}
				return false;
			}
		),
		s_screenMessages.end()
	);

	s_worldDebugTexts.erase(
		std::remove_if(
			s_worldDebugTexts.begin(),
			s_worldDebugTexts.end(),
			[](DebugObject& obj)
			{
				if (obj.m_singleFrame) return true;
				if (obj.m_timer && obj.m_duration > 0.f && obj.m_timer->GetElapsedTime() >= obj.m_duration)
				{
					delete obj.m_timer;
					obj.m_timer = nullptr;
					return true;
				}
				return false;
			}
		),
		s_worldDebugTexts.end()
	);
}



void DebugRenderWorld(const Camera& camera)
{
	std::scoped_lock lock(s_debugRenderMutex);

	if (s_isVisible)
	{
		s_theRenderer->BeginCamera(camera);

		DebugRenderWorldObjects();
		DebugRenderWorldTexts();
		DebugRenderWorldBillboardTexts(camera);

		s_theRenderer->EndCamera(camera);
	}
	
}

void DebugRenderScreen(const Camera& camera)
{
	std::scoped_lock lock(s_debugRenderMutex);

	if (s_isVisible)
	{
		s_theRenderer->BeginCamera(camera);

		for (int i = 0; i < (int)s_screenDebugTexts.size(); ++i)
		{
			s_theRenderer->SetDepthMode(DepthMode::DISABLED);
			s_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
			s_theRenderer->BindShader(s_theRenderer->GetShader("Default"));
			s_theRenderer->SetBlendMode(BlendMode::ALPHA);
			s_theRenderer->BindTexture(&s_theFont->GetTexture());
			s_theRenderer->DrawVertexArray(s_screenDebugTexts[i].m_verts);
			s_theRenderer->BindTexture(nullptr);
		}

		const float verticalSpacing = 15.f;
		const float screenHeight = g_gameConfigBlackboard.GetValue("screenHeight", 800.f) * 0.95f;

		float currentY = screenHeight - verticalSpacing;

		for (int i = 0; i < (int)s_screenMessages.size(); ++i)
		{
			std::vector<Vertex_PCU>& verts = s_screenMessages[i].m_verts;

			Vec2 targetPosition = Vec2(0.f, currentY);

			std::vector<Vertex_PCU> tempVerts = verts;

			for (Vertex_PCU& vert : tempVerts)
			{
				vert.m_position += Vec3(targetPosition.x, targetPosition.y, 0.f); 
			}

			s_theRenderer->SetDepthMode(DepthMode::DISABLED);
			s_theRenderer->SetBlendMode(BlendMode::ALPHA);
			s_theRenderer->BindTexture(&s_theFont->GetTexture());

			s_theRenderer->DrawVertexArray(tempVerts);

			s_theRenderer->BindTexture(nullptr);
			currentY -= verticalSpacing;
		}
		
		s_theRenderer->EndCamera(camera);
	}
	
}

void DebugRenderEndFrame()
{
	std::scoped_lock lock(s_debugRenderMutex);
}

void DebugRenderWorldObjects()
{
	std::scoped_lock lock(s_debugRenderMutex);
	for (int i = 0; i < (int)s_debugObjects.size(); ++i)
	{
		DebugObject& obj = s_debugObjects[i];

		if (obj.m_mode == DebugRenderMode::ALWAYS)
		{
			s_theRenderer->SetDepthMode(DepthMode::DISABLED);
		}
		else if (obj.m_mode == DebugRenderMode::USE_DEPTH)
		{
			s_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		}
		else if (obj.m_mode == DebugRenderMode::X_RAY)
		{
			Rgba8 brightColor = obj.m_startColor;
			brightColor.r = (unsigned char)GetClamped((float)brightColor.r + 50, 0, 255);
			brightColor.g = (unsigned char)GetClamped((float)brightColor.g + 50, 0, 255);
			brightColor.b = (unsigned char)GetClamped((float)brightColor.b + 50, 0, 255);
			brightColor.a = 128;

			for (Vertex_PCU& vert : s_debugObjects[i].m_verts)
			{
				vert.m_color = brightColor;
			}

			s_theRenderer->SetBlendMode(BlendMode::ALPHA);
			s_theRenderer->SetDepthMode(DepthMode::READ_ONLY_ALWAYS);
			s_theRenderer->BindTexture(nullptr);
			s_theRenderer->DrawVertexArray(s_debugObjects[i].m_verts);

			for (Vertex_PCU& vert : s_debugObjects[i].m_verts)
			{
				vert.m_color = obj.m_startColor;
			}
			s_theRenderer->SetBlendMode(BlendMode::Blend_OPAQUE);
			s_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		}
		s_theRenderer->SetRasterizerMode(obj.m_rasterizerMode);

		s_theRenderer->BindTexture(obj.m_tex);
		s_theRenderer->DrawVertexArray(s_debugObjects[i].m_verts);
	}
}

void DebugRenderWorldTexts()
{
	std::scoped_lock lock(s_debugRenderMutex);
	for (int i = 0; i < (int)s_worldDebugTexts.size(); ++i)
	{
		DebugObject& text = s_worldDebugTexts[i];

		if (text.m_mode == DebugRenderMode::ALWAYS)
		{
			s_theRenderer->SetDepthMode(DepthMode::DISABLED);
		}
		else if (text.m_mode == DebugRenderMode::USE_DEPTH)
		{
			s_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		}
		else if (text.m_mode == DebugRenderMode::X_RAY)
		{
			Rgba8 brightColor = text.m_startColor;
			brightColor.r = (unsigned char)GetClamped((float)brightColor.r + 50, 0, 255);
			brightColor.g = (unsigned char)GetClamped((float)brightColor.g + 50, 0, 255);
			brightColor.b = (unsigned char)GetClamped((float)brightColor.b + 50, 0, 255);
			brightColor.a = 128;

			for (Vertex_PCU& vert : s_worldDebugTexts[i].m_verts)
			{
				vert.m_color = brightColor;
			}

			s_theRenderer->SetBlendMode(BlendMode::ALPHA);
			s_theRenderer->SetDepthMode(DepthMode::READ_ONLY_ALWAYS);
			s_theRenderer->BindTexture(nullptr);
			s_theRenderer->DrawVertexArray(s_worldDebugTexts[i].m_verts);

			for (Vertex_PCU& vert : s_worldDebugTexts[i].m_verts)
			{
				vert.m_color = text.m_startColor;
			}

			s_theRenderer->SetBlendMode(BlendMode::Blend_OPAQUE);
			s_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		}
		s_theRenderer->SetRasterizerMode(text.m_rasterizerMode);
		s_theRenderer->BindTexture(&s_theFont->GetTexture());
		s_theRenderer->DrawVertexArray(s_worldDebugTexts[i].m_verts);
		s_theRenderer->BindTexture(nullptr);
	}

}

void DebugRenderWorld2D(const Camera& camera)
{
	std::scoped_lock lock(s_debugRenderMutex);

	s_theRenderer->BeginCamera(camera);

	s_theRenderer->SetDepthMode(DepthMode::DISABLED);
	s_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	s_theRenderer->SetBlendMode(BlendMode::ALPHA);

	DebugRenderWorldObjects();

	if (s_theFont)
	{
		s_theRenderer->BindTexture(&s_theFont->GetTexture());

		for (const DebugObject& text : s_worldDebugTexts)
		{
			s_theRenderer->DrawVertexArray((int)text.m_verts.size(), text.m_verts.data());
		}

		s_theRenderer->BindTexture(nullptr);
	}

	s_theRenderer->EndCamera(camera);

	s_worldDebugTexts.clear();
}

void DebugRenderWorldBillboardTexts(const Camera& camera)
{
	std::scoped_lock lock(s_debugRenderMutex);
	Mat44 cameraMatrix = camera.GetCameraToWorldTransform();

	for (int i = 0; i < (int)s_worldBillboardTexts.size(); ++i)
	{
		DebugObject& text = s_worldBillboardTexts[i];
		std::vector<Vertex_PCU>& textVerts = s_worldBillboardTexts[i].m_verts;

		Mat44 billboardMatrix = GetBillboardMatrix(text.m_type, cameraMatrix, text.m_origin);

		std::vector<Vertex_PCU> transformedVerts = textVerts;
		for (Vertex_PCU& vert : transformedVerts)
		{
			Vec3 transformedPos = billboardMatrix.TransformPosition3D(vert.m_position);
			vert.m_position = transformedPos;
		}

		if (text.m_mode == DebugRenderMode::ALWAYS)
		{
			s_theRenderer->SetDepthMode(DepthMode::DISABLED);
		}
		else if (text.m_mode == DebugRenderMode::USE_DEPTH)
		{
			s_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		}
		else if (text.m_mode == DebugRenderMode::X_RAY)
		{
			Rgba8 brightColor = text.m_startColor;
			brightColor.r = (unsigned char)GetClamped((float)brightColor.r + 50, 0, 255);
			brightColor.g = (unsigned char)GetClamped((float)brightColor.g + 50, 0, 255);
			brightColor.b = (unsigned char)GetClamped((float)brightColor.b + 50, 0, 255);
			brightColor.a = 128;

			for (Vertex_PCU& vert : transformedVerts)
			{
				vert.m_color = brightColor;
			}

			s_theRenderer->SetBlendMode(BlendMode::ALPHA);
			s_theRenderer->SetDepthMode(DepthMode::READ_ONLY_ALWAYS);
			s_theRenderer->BindTexture(nullptr);
			s_theRenderer->DrawVertexArray(transformedVerts);

			for (Vertex_PCU& vert : transformedVerts)
			{
				vert.m_color = text.m_startColor;
			}

			s_theRenderer->SetBlendMode(BlendMode::Blend_OPAQUE);
			s_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
		}

		s_theRenderer->SetRasterizerMode(text.m_rasterizerMode);
		s_theRenderer->BindTexture(&s_theFont->GetTexture());

		s_theRenderer->DrawVertexArray(transformedVerts);

		s_theRenderer->BindTexture(nullptr);
	}
}

void DebugAddRaycastResult(RaycastResult3D result, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::scoped_lock lock(s_debugRenderMutex);
	Vec3 resultEnd = result.m_rayStart + result.m_rayForwardNormal * result.m_rayMaxLength;
	DebugAddWorldLine(result.m_rayStart, resultEnd, radius, duration, startColor, endColor, mode);

	if (result.m_didImpact)
	{
		DebugAddWorldPoint(result.m_impactPos, 0.06f, duration);

		Vec3 arrowEnd = result.m_impactPos + result.m_impactNormal * 0.3f;

		DebugAddWorldArrow(result.m_impactPos, arrowEnd, 0.03f, duration, Rgba8::BLUE, Rgba8::BLUE);
	}
}


void DebugAddWorldWireAABB3(AABB3 box, float duration,
	const Rgba8& startColor, const Rgba8& endColor,
	DebugRenderMode mode)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> aabbVerts;

	AddVertsForAABBWireframe3D(aabbVerts, box, 0.01f, startColor);

	DebugObject aabb;
	aabb.m_startColor = startColor;
	aabb.m_endColor = endColor;
	aabb.m_mode = mode;
	aabb.m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	aabb.m_verts = aabbVerts;

	if (duration == 0.f)
	{
		aabb.m_singleFrame = true;
		aabb.m_duration = 0.f;
		aabb.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		aabb.m_singleFrame = false;
		aabb.m_duration = duration;
		aabb.m_timer = new Timer(duration, s_debugRenderClock);
		aabb.m_timer->Start();
	}
	else
	{
		aabb.m_singleFrame = false;
		aabb.m_duration = -1.f;
		aabb.m_timer = nullptr;
	}

	s_debugObjects.push_back(aabb);
}


void DebugAddWorldPoint(const Vec3& pos, float radius, float duration,
	const Rgba8& startColor, const Rgba8& endColor,
	DebugRenderMode mode)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> pointVerts;

	AddVertsForSphere3D(pointVerts, pos, radius, startColor);

	DebugObject point;
	point.m_startColor = startColor;
	point.m_endColor = endColor;
	point.m_mode = mode;
	point.m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	point.m_verts = pointVerts;

	if (duration == 0.f)
	{
		point.m_singleFrame = true;
		point.m_duration = 0.f;
		point.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		point.m_singleFrame = false;
		point.m_duration = duration;
		point.m_timer = new Timer(duration, s_debugRenderClock);
		point.m_timer->Start();
	}
	else
	{
		point.m_singleFrame = false;
		point.m_duration = -1.f;
		point.m_timer = nullptr;
	}

	s_debugObjects.push_back(point);
}


void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration,
	const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> lineVerts;

	AddVertsForCylinder3D(lineVerts, start, end, radius, startColor, AABB2::ZERO_TO_ONE, 16);

	DebugObject line;
	line.m_startColor = startColor;
	line.m_endColor = endColor;
	line.m_mode = mode;
	line.m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	line.m_verts = lineVerts;

	if (duration == 0.f)
	{
		line.m_singleFrame = true;
		line.m_duration = 0.f;
		line.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		line.m_singleFrame = false;
		line.m_duration = duration;
		line.m_timer = new Timer(duration, s_debugRenderClock);
		line.m_timer->Start();
	}
	else
	{
		line.m_singleFrame = false;
		line.m_duration = -1.f;
		line.m_timer = nullptr;
	}

	s_debugObjects.push_back(line);
}


void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration,
	const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> cylinderVerts;

	AddVertsForCylinder3D(cylinderVerts, base, top, radius, startColor);

	DebugObject cylinder;
	cylinder.m_startColor = startColor;
	cylinder.m_endColor = endColor;
	cylinder.m_mode = mode;
	cylinder.m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	cylinder.m_verts = cylinderVerts;

	if (duration == 0.f)
	{
		cylinder.m_singleFrame = true;
		cylinder.m_duration = 0.f;
		cylinder.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		cylinder.m_singleFrame = false;
		cylinder.m_duration = duration;
		cylinder.m_timer = new Timer(duration, s_debugRenderClock);
		cylinder.m_timer->Start();
	}
	else
	{
		cylinder.m_singleFrame = false;
		cylinder.m_duration = -1.f;
		cylinder.m_timer = nullptr;
	}

	s_debugObjects.push_back(cylinder);
}


void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration,
	const Rgba8& startColor, const Rgba8& endColor,
	DebugRenderMode mode)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> sphereVerts;

	AddVertsForUVSphereZWireframe3D(sphereVerts, center, radius, 8, 0.01f, startColor);

	DebugObject sphere;
	sphere.m_startColor = startColor;
	sphere.m_endColor = endColor;
	sphere.m_mode = mode;
	sphere.m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	sphere.m_verts = sphereVerts;

	if (duration == 0.f)
	{
		sphere.m_singleFrame = true;
		sphere.m_duration = 0.f;
		sphere.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		sphere.m_singleFrame = false;
		sphere.m_duration = duration;
		sphere.m_timer = new Timer(duration, s_debugRenderClock);
		sphere.m_timer->Start();
	}
	else
	{
		sphere.m_singleFrame = false;
		sphere.m_duration = -1.f;
		sphere.m_timer = nullptr;
	}

	s_debugObjects.push_back(sphere);
}


void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration,
	const Rgba8& startColor, const Rgba8& endColor,
	DebugRenderMode mode)
{
	std::scoped_lock lock(s_debugRenderMutex);
	Vec3  direction = (end - start).GetNormalized();
	float arrowLength = (end - start).GetLength();

	float shaftLength = arrowLength * 0.7f;
	Vec3  shaftEnd = start + direction * shaftLength;
	Vec3  headEnd = end;

	float shaftRadius = radius;
	float headRadius = radius * 1.5f;

	std::vector<Vertex_PCU> shaftVerts;
	AddVertsForCylinder3D(shaftVerts, start, shaftEnd, shaftRadius, startColor,
		AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f)), 16);

	std::vector<Vertex_PCU> headVerts;
	AddVertsForCone3D(headVerts, shaftEnd, headEnd, headRadius, endColor,
		AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f)), 16);

	std::vector<Vertex_PCU> arrowVerts;
	arrowVerts.insert(arrowVerts.end(), shaftVerts.begin(), shaftVerts.end());
	arrowVerts.insert(arrowVerts.end(), headVerts.begin(), headVerts.end());

	DebugObject arrow;
	arrow.m_startColor = startColor;
	arrow.m_endColor = endColor;
	arrow.m_mode = mode;
	arrow.m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	arrow.m_verts = arrowVerts;

	if (duration == 0.f)
	{
		arrow.m_singleFrame = true;
		arrow.m_duration = 0.f;
		arrow.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		arrow.m_singleFrame = false;
		arrow.m_duration = duration;
		arrow.m_timer = new Timer(duration, s_debugRenderClock);
		arrow.m_timer->Start();
	}
	else
	{
		arrow.m_singleFrame = false;
		arrow.m_duration = -1.f;
		arrow.m_timer = nullptr;
	}

	s_debugObjects.push_back(arrow);
}


void DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight,
	float alignment, float duration, const Rgba8& startColor,
	const Rgba8& endColor, DebugRenderMode mode)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> textVerts;
	s_theFont->AddVertsForText3DAtOriginXForward(textVerts, textHeight, text,
		startColor, 1.f, Vec2(alignment, alignment));

	for (Vertex_PCU& vertex : textVerts)
		vertex.m_position = transform.TransformPosition3D(vertex.m_position);

	DebugObject obj;
	obj.m_startColor = startColor;
	obj.m_endColor = endColor;
	obj.m_mode = mode;
	obj.m_rasterizerMode = RasterizerMode::SOLID_CULL_NONE;
	obj.m_verts = textVerts;

	if (duration == 0.f)
	{
		obj.m_singleFrame = true;
		obj.m_duration = 0.f;
		obj.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		obj.m_singleFrame = false;
		obj.m_duration = duration;
		obj.m_timer = new Timer(duration, s_debugRenderClock);
		obj.m_timer->Start();
	}
	else
	{
		obj.m_singleFrame = false;
		obj.m_duration = -1.f;
		obj.m_timer = nullptr;
	}

	s_worldDebugTexts.push_back(obj);
}


void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin,
	float textHeight, const Vec2& alignment,
	float duration, const Rgba8& startColor,
	const Rgba8& endColor, DebugRenderMode mode,
	BillboardType type)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> textVerts;
	s_theFont->AddVertsForText3DAtOriginXForward(textVerts, textHeight, text,
		startColor, 1.f, alignment);

	DebugObject obj;
	obj.m_startColor = startColor;
	obj.m_endColor = endColor;
	obj.m_mode = mode;
	obj.m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	obj.m_origin = origin;
	obj.m_type = type;
	obj.m_verts = textVerts;

	if (duration == 0.f)
	{
		obj.m_singleFrame = true;
		obj.m_duration = 0.f;
		obj.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		obj.m_singleFrame = false;
		obj.m_duration = duration;
		obj.m_timer = new Timer(duration, s_debugRenderClock);
		obj.m_timer->Start();
	}
	else
	{
		obj.m_singleFrame = false;
		obj.m_duration = -1.f;
		obj.m_timer = nullptr;
	}

	s_worldBillboardTexts.push_back(obj);
}



void DebugAddWorldBasis(const Mat44& transform, float duration, DebugRenderMode mode)
{
	Vec3 pos = transform.GetTranslation3D();
	Vec3 fwd = transform.GetIBasis3D();
	Vec3 left = transform.GetJBasis3D();
	Vec3 up = transform.GetKBasis3D();

	Vec3 xEnd = pos + fwd;
	Vec3 yEnd = pos + left;
	Vec3 zEnd = pos + up;

	DebugAddWorldArrow(pos, xEnd, 0.1f, duration, Rgba8::RED, Rgba8::RED, mode);
	DebugAddWorldArrow(pos, yEnd, 0.1f, duration, Rgba8::GREEN, Rgba8::GREEN, mode);
	DebugAddWorldArrow(pos, zEnd, 0.1f, duration, Rgba8::BLUE, Rgba8::BLUE, mode);
}


void DebugAddWorldCone(const Vec3& discCenter, const Vec3& tipPos, float radius,
	float duration, const Rgba8& startColor, const Rgba8& endColor,
	DebugRenderMode mode)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> coneVerts;

	AddVertsForCone3D(coneVerts, discCenter, tipPos, radius, startColor);

	DebugObject obj;
	obj.m_startColor = startColor;
	obj.m_endColor = endColor;
	obj.m_mode = mode;
	obj.m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	obj.m_verts = coneVerts;

	if (duration == 0.f)
	{
		obj.m_singleFrame = true;
		obj.m_duration = 0.f;
		obj.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		obj.m_singleFrame = false;
		obj.m_duration = duration;
		obj.m_timer = new Timer(duration, s_debugRenderClock);
		obj.m_timer->Start();
	}
	else
	{
		obj.m_singleFrame = false;
		obj.m_duration = -1.f;
		obj.m_timer = nullptr;
	}

	s_debugObjects.push_back(obj);
}


void DebugAddWorldWireCone(const Vec3& discCenter, const Vec3& tipPos, float radius,
	float duration, const Rgba8& startColor, const Rgba8& endColor,
	DebugRenderMode mode)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> coneVerts;

	AddVertsForCone3D(coneVerts, discCenter, tipPos, radius, startColor);

	DebugObject obj;
	obj.m_startColor = startColor;
	obj.m_endColor = endColor;
	obj.m_mode = mode;
	obj.m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	obj.m_verts = coneVerts;

	if (duration == 0.f)
	{
		obj.m_singleFrame = true;
		obj.m_duration = 0.f;
		obj.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		obj.m_singleFrame = false;
		obj.m_duration = duration;
		obj.m_timer = new Timer(duration, s_debugRenderClock);
		obj.m_timer->Start();
	}
	else
	{
		obj.m_singleFrame = false;
		obj.m_duration = -1.f;
		obj.m_timer = nullptr;
	}

	s_debugObjects.push_back(obj);
}


void DebugAddScreenText(const std::string& text, const AABB2& box, float cellHeight,
	const Vec2& alignment, float duration,
	const Rgba8& startColor, const Rgba8& endColor)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> verts;
	s_theFont->AddVertsForTextInBox2D(verts, text, box, cellHeight,
		startColor, 1.f, alignment, TextBoxMode::OVERRUN);

	DebugObject obj;
	obj.m_startColor = startColor;
	obj.m_endColor = endColor;
	obj.m_verts = verts;

	if (duration == 0.f)
	{
		obj.m_singleFrame = true;
		obj.m_duration = 0.f;
		obj.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		obj.m_singleFrame = false;
		obj.m_duration = duration;
		obj.m_timer = new Timer(duration, s_debugRenderClock);
		obj.m_timer->Start();
	}
	else
	{
		obj.m_singleFrame = false;
		obj.m_duration = -1.f;
		obj.m_timer = nullptr;
	}

	s_screenDebugTexts.push_back(obj);
}


void DebugAddScreenText(const std::string& text, const AABB2& box, float cellHeight, 
	float cellAspectScale, const Vec2& alignment, float duration, 
	const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> verts;
	s_theFont->AddVertsForTextInBox2D(verts, text, box, cellHeight,
		startColor, cellAspectScale, alignment, TextBoxMode::OVERRUN);

	DebugObject obj;
	obj.m_startColor = startColor;
	obj.m_endColor = endColor;
	obj.m_verts = verts;

	if (duration == 0.f)
	{
		obj.m_singleFrame = true;
		obj.m_duration = 0.f;
		obj.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		obj.m_singleFrame = false;
		obj.m_duration = duration;
		obj.m_timer = new Timer(duration, s_debugRenderClock);
		obj.m_timer->Start();
	}
	else
	{
		obj.m_singleFrame = false;
		obj.m_duration = -1.f;
		obj.m_timer = nullptr;
	}

	s_screenDebugTexts.push_back(obj);
}

void DebugAddMessage(const std::string& text, float duration,
	const Rgba8& startColor, const Rgba8& endColor)
{
	std::scoped_lock lock(s_debugRenderMutex);
	std::vector<Vertex_PCU> verts;

	AABB2 box(Vec2(0.f, 0.f), Vec2(300.f, 10.f));
	s_theFont->AddVertsForTextInBox2D(verts, text, box, 15.f, startColor,
		0.8f, Vec2(0.f, 0.5f), TextBoxMode::OVERRUN);

	DebugObject obj;
	obj.m_startColor = startColor;
	obj.m_endColor = endColor;
	obj.m_verts = verts;

	if (duration == 0.f)
	{
		obj.m_singleFrame = true;
		obj.m_duration = 0.f;
		obj.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		obj.m_singleFrame = false;
		obj.m_duration = duration;
		obj.m_timer = new Timer(duration, s_debugRenderClock);
		obj.m_timer->Start();
	}
	else 
	{
		obj.m_singleFrame = false;
		obj.m_duration = -1.f;
		obj.m_timer = nullptr;
	}


	s_screenMessages.push_back(obj);
}



void DebugAddWorldText2D(const std::string& text, const Vec2& worldPos, float cellHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor)
{
	std::scoped_lock lock(s_debugRenderMutex);

	std::vector<Vertex_PCU> verts;
	AABB2 dummyBox(Vec2::ZERO, Vec2(1000.f, cellHeight * 2.f));
	s_theFont->AddVertsForTextInBox2D(verts, text, dummyBox, cellHeight, startColor, 1.f, alignment, TextBoxMode::SHRINK_TO_FIT);

	Vec2 textBoundsMin = Vec2(FLT_MAX, FLT_MAX);
	Vec2 textBoundsMax = Vec2(-FLT_MAX, -FLT_MAX);
	for (const Vertex_PCU& v : verts) {
		textBoundsMin.x = std::min(textBoundsMin.x, v.m_position.x);
		textBoundsMin.y = std::min(textBoundsMin.y, v.m_position.y);
		textBoundsMax.x = std::max(textBoundsMax.x, v.m_position.x);
		textBoundsMax.y = std::max(textBoundsMax.y, v.m_position.y);
	}
	Vec2 textCenter = (textBoundsMin + textBoundsMax) * 0.5f;
	Vec2 offset = worldPos - textCenter;

	for (Vertex_PCU& v : verts) {
		v.m_position += Vec3(offset.x, offset.y, 0.f);
	}

	DebugObject obj;
	obj.m_startColor = startColor;
	obj.m_endColor = endColor;
	obj.m_verts = verts;

	if (duration == 0.f) {
		obj.m_singleFrame = true;
		obj.m_duration = 0.f;
		obj.m_timer = nullptr;
	}
	else if (duration > 0.f) {
		obj.m_singleFrame = false;
		obj.m_duration = duration;
		obj.m_timer = new Timer(duration, s_debugRenderClock);
		obj.m_timer->Start();
	}
	else {
		obj.m_singleFrame = false;
		obj.m_duration = -1.f;
		obj.m_timer = nullptr;
	}

	s_worldDebugTexts.push_back(obj);
}

void DebugAddWorldAABB2(const AABB2& bounds, Texture* tex, const Rgba8& tint, float duration, DebugRenderMode mode)
{
	std::scoped_lock lock(s_debugRenderMutex);

	std::vector<Vertex_PCU> verts;

	AddVertsForAABB2D(
		verts,
		bounds,
		tint,
		Vec2(0.0f, 0.0f),  // UV min
		Vec2(1.0f, 1.0f)   // UV max
	);

	DebugObject obj;
	obj.m_startColor = tint;
	obj.m_endColor = tint;
	obj.m_mode = mode;
	obj.m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	obj.m_verts = verts;

	if (duration == 0.f)
	{
		obj.m_singleFrame = true;
		obj.m_duration = 0.f;
		obj.m_timer = nullptr;
	}
	else if (duration > 0.f)
	{
		obj.m_singleFrame = false;
		obj.m_duration = duration;
		obj.m_timer = new Timer(duration, s_debugRenderClock);
		obj.m_timer->Start();
	}
	else
	{
		obj.m_singleFrame = false;
		obj.m_duration = -1.f;
		obj.m_timer = nullptr;
	}

	obj.m_tex = tex;

	s_debugObjects.push_back(obj);
}
bool Command_DebugRenderClear(EventArgs& args)
{
	UNUSED(args);

	DebugRenderClear();
	return true;
}

bool Command_DebugRenderToggle(EventArgs& args)
{
	UNUSED(args);

	s_isVisible = !s_isVisible;

	return true;
}
