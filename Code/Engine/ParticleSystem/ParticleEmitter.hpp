#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/ParticleSystem/ParticleForce.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <vector>
#include <string>

class Texture;
class Shader;
class StructuredBuffer;
class ConstantBuffer;
class VertexBuffer;
class IndexBuffer;
struct ID3D11Buffer;
struct ID3D11Query;
class ParticleSystem;

struct alignas(16) Particle
{
	Vec3  position;      float lifetime;

	Vec3  velocity;      float maxLifetime;

	Vec4 color;

	float size;
	float softFactor;
	float emissive;
	float stage;

	float  rotation;
	float  angularVelocity;
	float  p_pad0;
	float  p_pad1;
};

struct ParticleStageConfig
{
	Rgba8 startColor = Rgba8(127, 127, 127, 255);
	Rgba8 endColor = Rgba8(127, 127, 127, 0);
	float startSize = 0.1f;
	float endSize = 1.0f;
	float lifetime = 2.0f;
	float lifetimeVariance = 0.5f;

	Vec3  baseVelocity = Vec3(0, 0, 0);
	Vec3  velocityVariance = Vec3(0, 0, 0);

	float baseAngularVelocity = 0.f;
	float angularVariance = 0.f;

	std::string texPath = "";
	float prob = 1.f;

	float startSoftFactor = 0.0f;
	float endSoftFactor = 0.0f;

	float startEmissive = 0.0f;
	float endEmissive = 0.0f;

	uint32_t billboardType = 0u;
};

struct ParticleEmitterConfig
{
	ParticleSystem* m_owner = nullptr;
	uint32_t maxParticles = 100000;
	std::string name = "Emitter";

	BlendMode blendMode = BlendMode::ALPHA;

	Vec3  position = Vec3::ZERO;
	Vec3  spawnArea = Vec3(0.5f, 0.5f, 0.0f);

	float spawnRate = 500.0f;

	float noiseStrength = 0.f;
	float noiseFrequency = 0.f;

	bool  isLooping = true;
	float duration = 0.0f;
	bool  enabled = true;

	ParticleStageConfig mainStage;

	bool useSubStage = false;
	ParticleStageConfig subStage;
};


struct alignas(16) ParticleSystemConstants
{
	Mat44 modelMatrix;
	float deltaTime;
	float systemTime;
	uint32_t mainBillboardType;
	uint32_t subBillboardType;
};
static_assert(sizeof(ParticleSystemConstants) % 16 == 0, "b0 must be 16B aligned");

struct alignas(16) ParticleEmitterConstants
{
	Vec3 emitterPosition;      float _pad0;

	Vec3 spawnExtent;          float _pad1;

	Vec3 baseVelocity;         float _pad2;
	Vec3 velocityVariance;     float _pad3;

	float baseAngularVelocity; float angularVariance; Vec2 _pad4;

	float particleLifetime;
	float lifetimeVariance;
	float startSize;
	float endSize;

	Vec4  startColor;
	Vec4  endColor;

	float startSoftFactor;
	float endSoftFactor;
	float startEmissive;
	float endEmissive;

	uint32_t spawnBudget;
	uint32_t randomSeed;
	float    noiseStrength;
	float    noiseFrequency;

	Vec4 substageStartColor;
	Vec4 substageEndColor;

	float substageLifetime;
	float substageLifetimeVariance;
	float substageStartSize;
	float substageEndSize;

	float substageStartSoftFactor;
	float substageEndSoftFactor;
	float substageStartEmissive;
	float substageEndEmissive;

	Vec3  substageBaseVelocity;     uint32_t useSubStage;
	Vec3  substageVelocityVariance; float    substageProb;

	float substageAngularVelocity;  float substageAngularVariance; Vec2 _pad5;

	uint32_t mainBillboardType;
	uint32_t substageBillboardType;

	float    _padBillboard[2];
};

static_assert(sizeof(ParticleEmitterConstants) % 16 == 0);


struct alignas(16) SoftParams 
{
	float nearZ;
	float farZ;
	float softRange;
	float softBias;
};
static_assert(sizeof(SoftParams) % 16 == 0, "CBuffer must be 16B-aligned");

struct alignas(16) CullingConstants
{
	Mat44	WorldToCamera;
	Mat44	CameraToRender;
	Mat44	RenderToClip;
	Vec3	CameraPos;   
	float	Padding1;
	Vec2	ViewportSize;
	float	MinPixelPx;
	float	FrustumPad;
	float   LODNearDist;
	float   LODFarDist;
	float   LODMaxSkip;
	float   Padding2;
};
static_assert(sizeof(CullingConstants) % 16 == 0, "CBuffer must be 16B-aligned");

class ParticleEmitter
{
public:
	ParticleEmitter(const ParticleEmitterConfig& config);
	~ParticleEmitter();

	void Startup();
	void Shutdown();

	void Update(float deltaSeconds);
	void Render() const;

	void SimpleRender() const;

	void SetTransform(const Mat44& transform);
	void SetPosition(const Vec3& position);
	void SetEnabled(bool enabled) { m_config.enabled = enabled; }
	void SetPaused(bool paused) { m_isPaused = paused; }
	void Restart();

	bool IsAlive() const;
	bool IsEnabled() const { return m_config.enabled; }
	float GetAge() const { return m_systemAge; }
	const std::string& GetName() const { return m_config.name; }

	const ParticleEmitterConfig& GetConfig() const { return m_config; }
	void UpdateConfig(const ParticleEmitterConfig& config);
	
	uint32_t GetActiveParticleCount() const { return m_activeParticleCount; }
	int  GetActiveCount() const { return m_activeParticleCount; }
	int  GetMaxParticles() const { return m_config.maxParticles; }
	float GetSpawnRate() const { return m_config.spawnRate; }
	double GetLastGpuTimeMs() const { return m_lastGpuTimeMs; }
private:
	void InitializeBuffers();
	void InitializeShaders();
	void SpawnParticles(float deltaSeconds);
	void UpdateGPU(float deltaSeconds);
	void CreateGPUQueries();


public:
	ParticleEmitterConfig m_config;
private:
	StructuredBuffer* m_particleBufferA = nullptr;
	StructuredBuffer* m_particleBufferB = nullptr;

	ConstantBuffer* m_systemCBO = nullptr;
	ConstantBuffer* m_emitterCBO = nullptr;

	StructuredBuffer* m_spawnCounter = nullptr;
	StructuredBuffer* m_aliveCounter = nullptr;

	ID3D11Buffer* m_aliveCounterStaging = nullptr;

	ConstantBuffer* m_softCBO = nullptr;
	ConstantBuffer* m_cullingCBO = nullptr;
	
	Shader* m_updateShader = nullptr;
	Shader* m_renderShader = nullptr;
	Texture* m_particleTexture = nullptr;
	Texture* m_particleTextureStage1 = nullptr;

	float m_systemAge = 0.0f;
	float m_spawnAccumulator = 0.0f;
	uint32_t m_activeParticleCount = 0;
	bool m_isPaused = false;
	bool m_isFirstUpdate = true;

	Mat44 m_transform;

	bool m_useBufferAForUpdate = true;

	ID3D11Query* m_gpuDisjoint = nullptr;
	ID3D11Query* m_gpuTimestampBeg = nullptr;
	ID3D11Query* m_gpuTimestampEnd = nullptr;

	mutable double m_lastGpuTimeMs = 0.0;

};