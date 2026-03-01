#include "ParticleEmitter.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Game.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/DevConsole.hpp"
#include <cstdint>

extern Renderer* g_theRenderer;
extern ParticleSystem* g_theParticleSystem;
extern DevConsole* g_theDevConsole;
extern Window* g_theWindow;

constexpr uint32_t COMPUTE_THREAD_GROUP_SIZE = 64;

ParticleEmitter::ParticleEmitter(const ParticleEmitterConfig& config)
	: m_config(config)
{
	Startup();
}

ParticleEmitter::~ParticleEmitter()
{
	Shutdown();
}

void ParticleEmitter::Startup()
{
	InitializeBuffers();
	InitializeShaders();
	CreateGPUQueries();

	m_particleTexture = g_theRenderer->CreateTextureFromFile(m_config.mainStage.texPath.c_str());
	
	if (m_config.useSubStage && !m_config.subStage.texPath.empty())
	{
		m_particleTextureStage1 = g_theRenderer->CreateTextureFromFile(m_config.subStage.texPath.c_str());
	}
	
	
	if (!m_config.m_owner)
	{
		m_config.m_owner = g_theParticleSystem;
	}
}
void ParticleEmitter::Shutdown()
{
	SAFE_DELETE(m_particleBufferA);
	SAFE_DELETE(m_particleBufferB);
	SAFE_DELETE(m_spawnCounter);
	SAFE_DELETE(m_aliveCounter);
	SAFE_DELETE(m_systemCBO);
	SAFE_DELETE(m_emitterCBO);
	SAFE_DELETE(m_softCBO);
	SAFE_DELETE(m_cullingCBO);
	DX_SAFE_RELEASE(m_aliveCounterStaging);
	DX_SAFE_RELEASE(m_gpuDisjoint);
	DX_SAFE_RELEASE(m_gpuTimestampBeg);
	DX_SAFE_RELEASE(m_gpuTimestampEnd);
}

void ParticleEmitter::Update(float deltaSeconds)
{
	if (!m_config.enabled)
	{
		return;
	}

	m_systemAge += deltaSeconds;

	if (m_config.duration > 0.0f && m_systemAge >= m_config.duration)
	{
		if (m_config.isLooping)
		{
			Restart();
		}
		else
		{
			m_config.enabled = false;
			return;
		}
	}

	//SpawnParticles(deltaSeconds);
	UpdateGPU(deltaSeconds);


}

void ParticleEmitter::UpdateGPU(float deltaSeconds)
{
	if (!m_updateShader || !m_updateShader->m_computeShader)
		return;

	ID3D11DeviceContext* ctx = g_theRenderer->GetDeviceContext();

	if (m_config.m_owner)
	{
		m_config.m_owner->UploadForcesIfDirty();
	}
	

	ID3D11VertexShader* oldVS = nullptr;
	ID3D11PixelShader* oldPS = nullptr;
	ID3D11InputLayout* oldIL = nullptr;
	ctx->VSGetShader(&oldVS, nullptr, 0);
	ctx->PSGetShader(&oldPS, nullptr, 0);
	ctx->IAGetInputLayout(&oldIL);

	g_theRenderer->BindComputeShader(m_updateShader);

	ParticleSystemConstants sys{};
	sys.modelMatrix = m_transform;
	sys.deltaTime = deltaSeconds;
	sys.systemTime = m_systemAge;
	g_theRenderer->UpdateConstantBuffer(m_systemCBO, &sys, sizeof(sys));
	g_theRenderer->BindConstantBufferCS(0, m_systemCBO);

	ParticleEmitterConstants emitter{};
	emitter.emitterPosition = m_config.position;
	emitter.spawnExtent = m_config.spawnArea * 0.5f;
	emitter.baseVelocity = m_config.mainStage.baseVelocity;
	emitter.velocityVariance = m_config.mainStage.velocityVariance;
	emitter.baseAngularVelocity = m_config.mainStage.baseAngularVelocity;
	emitter.angularVariance = m_config.mainStage.angularVariance;
	emitter.substageAngularVelocity = m_config.subStage.baseAngularVelocity;
	emitter.substageAngularVariance = m_config.subStage.angularVariance;
	emitter.particleLifetime = m_config.mainStage.lifetime;
	emitter.lifetimeVariance = m_config.mainStage.lifetimeVariance;
	emitter.startSize = m_config.mainStage.startSize;
	emitter.endSize = m_config.mainStage.endSize;
	emitter.startColor = m_config.mainStage.startColor.ToVec4();
	emitter.endColor = m_config.mainStage.endColor.ToVec4();
	emitter.startSoftFactor = m_config.mainStage.startSoftFactor;
	emitter.endSoftFactor = m_config.mainStage.endSoftFactor;
	emitter.startEmissive = m_config.mainStage.startEmissive;
	emitter.endEmissive = m_config.mainStage.endEmissive;
	emitter.substageStartColor = m_config.subStage.startColor.ToVec4();
	emitter.substageEndColor = m_config.subStage.endColor.ToVec4();
	emitter.substageLifetime = m_config.subStage.lifetime;
	emitter.substageLifetimeVariance = m_config.subStage.lifetimeVariance;
	emitter.substageStartSize = m_config.subStage.startSize;
	emitter.substageEndSize = m_config.subStage.endSize;
	emitter.substageBaseVelocity = m_config.subStage.baseVelocity;
	emitter.substageVelocityVariance = m_config.subStage.velocityVariance;
	emitter.useSubStage = m_config.useSubStage ? 1u : 0u;
	emitter.substageProb = m_config.subStage.prob;
	emitter.substageStartSoftFactor = m_config.subStage.startSoftFactor;
	emitter.substageEndSoftFactor = m_config.subStage.endSoftFactor;
	emitter.substageStartEmissive = m_config.subStage.startEmissive;
	emitter.substageEndEmissive = m_config.subStage.endEmissive;
	
	emitter.mainBillboardType = m_config.mainStage.billboardType;
	emitter.substageBillboardType = m_config.subStage.billboardType;

	const uint32_t B = 1u;
	const float dt = Max(deltaSeconds, 1e-6f);

	const float minRateToMeetB = float(B) / dt;
	const float effectiveSpawnRate = Max(m_config.spawnRate, minRateToMeetB);

	m_spawnAccumulator += effectiveSpawnRate * dt;

	uint32_t spawnBudget = (uint32_t)m_spawnAccumulator;
	if (spawnBudget > 0)
	{
		m_spawnAccumulator -= float(spawnBudget);

		uint32_t available = m_config.maxParticles - m_activeParticleCount;
		spawnBudget = Min(spawnBudget, available);
	}

	emitter.spawnBudget = spawnBudget;

	emitter.randomSeed = static_cast<std::uint32_t>(m_systemAge * 1000.0f);

	g_theRenderer->UpdateConstantBuffer(m_emitterCBO, &emitter, sizeof(emitter));
	g_theRenderer->BindConstantBufferCS(1, m_emitterCBO);

	CullingConstants pvc{};
	pvc.WorldToCamera = g_theParticleSystem->m_camera.GetWorldToCameraTransform();
	pvc.CameraToRender = g_theParticleSystem->m_camera.GetCameraToRenderTransform();
	pvc.RenderToClip = g_theParticleSystem->m_camera.GetRenderToClipTransform();
	pvc.CameraPos = g_theParticleSystem->m_camera.GetPosition();
	pvc.ViewportSize = Vec2((float)g_theWindow->GetClientDimensions().x,
		(float)g_theWindow->GetClientDimensions().y);
	pvc.MinPixelPx = 1.5f;
	pvc.FrustumPad = 0.05f;
	pvc.LODNearDist = 10.0f;
	pvc.LODFarDist = 100.0f;
	pvc.LODMaxSkip = 4.0f;

	g_theRenderer->UpdateConstantBuffer(m_cullingCBO, &pvc, sizeof(pvc));
	g_theRenderer->BindConstantBufferCS(5, m_cullingCBO);

	std::uint32_t zero = 0;
	if (m_spawnCounter) m_spawnCounter->Update(&zero, 1);
	if (m_aliveCounter) m_aliveCounter->Update(&zero, 1);

	StructuredBuffer* inputBuffer = m_useBufferAForUpdate ? m_particleBufferA : m_particleBufferB;
	StructuredBuffer* outputBuffer = m_useBufferAForUpdate ? m_particleBufferB : m_particleBufferA;

	auto EarlyRestoreAndReturn = [&](void) {
		ID3D11Buffer* nullCB[2] = { nullptr, nullptr };
		ctx->CSSetConstantBuffers(0, 2, nullCB);

		ID3D11ShaderResourceView* nullSRV[8] = {};
		ctx->CSSetShaderResources(0, 8, nullSRV);

		ID3D11UnorderedAccessView* nullUAV[4] = {};
		UINT zeros[4] = {};
		ctx->CSSetUnorderedAccessViews(0, 4, nullUAV, zeros);

		ID3D11SamplerState* nullSamp[4] = {};
		ctx->CSSetSamplers(0, 4, nullSamp);

		ctx->CSSetShader(nullptr, nullptr, 0);

		ctx->VSSetShader(oldVS, nullptr, 0);
		ctx->PSSetShader(oldPS, nullptr, 0);
		ctx->IASetInputLayout(oldIL);
		DX_SAFE_RELEASE(oldVS);
		DX_SAFE_RELEASE(oldPS);
		DX_SAFE_RELEASE(oldIL);
		};

	if (!inputBuffer || !outputBuffer)
	{
		EarlyRestoreAndReturn();
		m_useBufferAForUpdate = !m_useBufferAForUpdate;
		return;
	}

	inputBuffer->BindAsSRV(0);               // t0
	outputBuffer->BindAsUAV(0);              // u0
	if (m_spawnCounter) m_spawnCounter->BindAsUAV(1); // u1
	if (m_aliveCounter) m_aliveCounter->BindAsUAV(2); // u2
	if (m_config.m_owner && m_config.m_owner->GetForceBuffer())
	{
		m_config.m_owner->GetForceBuffer()->BindAsSRV(1); // t1
		
	}

	if (m_gpuDisjoint && m_gpuTimestampBeg && m_gpuTimestampEnd)
	{
		ctx->Begin(m_gpuDisjoint);
		ctx->End(m_gpuTimestampBeg);
	}

	{
		const std::uint32_t THREADS = COMPUTE_THREAD_GROUP_SIZE;
		std::uint32_t groupCount = (static_cast<std::uint32_t>(m_config.maxParticles) + THREADS - 1U) / THREADS;
		ctx->Dispatch(groupCount, 1, 1);
	}

	if (m_gpuDisjoint && m_gpuTimestampBeg && m_gpuTimestampEnd)
	{
		ctx->End(m_gpuTimestampEnd);
		ctx->End(m_gpuDisjoint);
	}


	inputBuffer->UnbindAsSRV(0);
	outputBuffer->UnbindAsUAV(0);
	if (m_spawnCounter) m_spawnCounter->UnbindAsUAV(1);
	if (m_aliveCounter) m_aliveCounter->UnbindAsUAV(2);
	if (m_config.m_owner && m_config.m_owner->GetForceBuffer()) {
		m_config.m_owner->GetForceBuffer()->UnbindAsSRV(1);
	}

	{
		ID3D11Buffer* nullCBs[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		ctx->CSSetConstantBuffers(0, 6, nullCBs);
	}

	{
		ID3D11ShaderResourceView* nullSRV[8] = {};
		ctx->CSSetShaderResources(0, 8, nullSRV);

		ID3D11UnorderedAccessView* nullUAV[4] = {};
		UINT zeros[4] = {};
		ctx->CSSetUnorderedAccessViews(0, 4, nullUAV, zeros);

		ID3D11SamplerState* nullSamp[4] = {};
		ctx->CSSetSamplers(0, 4, nullSamp);
	}

	ctx->CSSetShader(nullptr, nullptr, 0);

	m_useBufferAForUpdate = !m_useBufferAForUpdate;

	m_activeParticleCount = 0;
	if (m_aliveCounter && m_aliveCounterStaging) {
		ID3D11Buffer* gpuBuf = m_aliveCounter->GetBuffer();
		if (gpuBuf) {
			ctx->CopyResource(m_aliveCounterStaging, gpuBuf);
			D3D11_MAPPED_SUBRESOURCE mapped{};
			HRESULT hr = ctx->Map(m_aliveCounterStaging, 0, D3D11_MAP_READ, 0, &mapped);
			if (SUCCEEDED(hr)) {
				const std::uint32_t* data = reinterpret_cast<const std::uint32_t*>(mapped.pData);
				if (data) m_activeParticleCount = static_cast<int>(*data);
				ctx->Unmap(m_aliveCounterStaging, 0);
			}
			else {
				m_activeParticleCount = 0;
			}
		}
	}

	if (m_gpuDisjoint && m_gpuTimestampBeg && m_gpuTimestampEnd)
	{
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint{};
		HRESULT hrDisjoint = ctx->GetData(
			m_gpuDisjoint,
			&disjoint,
			sizeof(disjoint),
			0);

		if (SUCCEEDED(hrDisjoint) && !disjoint.Disjoint)
		{
			UINT64 tsBegin = 0;
			UINT64 tsEnd = 0;

			HRESULT hrBegin = ctx->GetData(
				m_gpuTimestampBeg,
				&tsBegin,
				sizeof(tsBegin),
				0);

			HRESULT hrEnd = ctx->GetData(
				m_gpuTimestampEnd,
				&tsEnd,
				sizeof(tsEnd),
				0);

			if (SUCCEEDED(hrBegin) && SUCCEEDED(hrEnd))
			{
				double deltaTicks = double(tsEnd - tsBegin);
				double freq = double(disjoint.Frequency);
				double timeSec = deltaTicks / freq;
				m_lastGpuTimeMs = timeSec * 1000.0;
			}
		}
	}


	ctx->VSSetShader(oldVS, nullptr, 0);
	ctx->PSSetShader(oldPS, nullptr, 0);
	ctx->IASetInputLayout(oldIL);
	DX_SAFE_RELEASE(oldVS);
	DX_SAFE_RELEASE(oldPS);
	DX_SAFE_RELEASE(oldIL);


}

void ParticleEmitter::Render() const
{
	if (!m_config.enabled || m_activeParticleCount == 0 || !m_renderShader)
		return;



	Shader* prevShader = g_theRenderer->GetCurrentShader();
	g_theRenderer->BindShader(m_renderShader);

	g_theRenderer->BindConstantBufferVS(2, g_theRenderer->m_cameraCBO);
	g_theRenderer->BindConstantBufferPS(2, g_theRenderer->m_cameraCBO);
	g_theRenderer->BindConstantBufferVS(3, g_theRenderer->m_modelCBO);
	g_theRenderer->BindConstantBufferPS(3, g_theRenderer->m_modelCBO);

	if (g_theRenderer->m_lightCBO)
	{
		g_theRenderer->BindConstantBufferVS(4, g_theRenderer->m_lightCBO);
		g_theRenderer->BindConstantBufferPS(4, g_theRenderer->m_lightCBO);
	}

	if (m_particleTexture)
	{
		g_theRenderer->BindTexture(m_particleTexture, 1);
	}
	if (m_particleTextureStage1)
	{
		g_theRenderer->BindTexture(m_particleTextureStage1, 6);
	}

	{
		ID3D11SamplerState* samp = g_theRenderer->GetSamplerState();
		g_theRenderer->GetDeviceContext()->PSSetSamplers(0, 1, &samp);
	}

	ID3D11ShaderResourceView* depthSRV = g_theRenderer->GetSceneDepthSRV();
	if (depthSRV)
	{
		auto* ctx = g_theRenderer->GetDeviceContext();
		ctx->PSSetShaderResources(5, 1, &depthSRV);
		ID3D11SamplerState* samp = g_theRenderer->GetSamplerState();
		ctx->PSSetSamplers(5, 1, &samp);

		struct SoftParams { float nearZ, farZ, softRange, softBias; };
		SoftParams sp{};
		sp.nearZ = g_theRenderer->GetCurrentCamera()->GetPerspectiveNearAndFar().x;
		sp.farZ = g_theRenderer->GetCurrentCamera()->GetPerspectiveNearAndFar().y;
		sp.softRange = 5.0f;
		sp.softBias = 1.0f;
		g_theRenderer->UpdateConstantBuffer(m_softCBO, &sp, sizeof(sp));
		g_theRenderer->BindConstantBufferPS(9, m_softCBO);
	}

	ParticleSystemConstants cb{};
	cb.modelMatrix = m_transform;
	cb.deltaTime = 0;
	cb.systemTime = m_systemAge;
	cb.mainBillboardType = m_config.mainStage.billboardType;
	cb.subBillboardType = m_config.subStage.billboardType;
	g_theRenderer->UpdateConstantBuffer(m_systemCBO, &cb, sizeof(cb));
	g_theRenderer->BindConstantBufferVS(8, m_systemCBO);
	g_theRenderer->BindConstantBufferPS(8, m_systemCBO);

	const StructuredBuffer* sim = (m_useBufferAForUpdate ? m_particleBufferB : m_particleBufferA);
	if (!sim)
	{
		if (prevShader) g_theRenderer->BindShader(prevShader);
		return;
	}
	sim->BindAsSRV(0);

	g_theRenderer->SetBlendMode(m_config.blendMode);
	g_theRenderer->SetDepthMode(DepthMode::READ_ONLY_LESS_EQUAL);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);


	g_theRenderer->SetStatesIfChanged();

	auto* ctx = g_theRenderer->GetDeviceContext();
	ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	ctx->IASetInputLayout(nullptr);
	ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11RenderTargetView* rtv = g_theRenderer->GetMainRTV();
	ID3D11DepthStencilView* dsv = g_theRenderer->GetMainDSV_ReadOnly();
	ctx->OMSetRenderTargets(1, &rtv, dsv);

	ctx->DrawInstanced(6, m_activeParticleCount, 0, 0);

	sim->UnbindAsSRV(0);

	if (prevShader)
		g_theRenderer->BindShader(prevShader);
}

void ParticleEmitter::SimpleRender() const
{
	if (!m_config.enabled || m_activeParticleCount == 0 || !m_renderShader)
		return;

	Shader* previousShader = g_theRenderer->GetCurrentShader();
	g_theRenderer->BindShader(m_renderShader);

	g_theRenderer->BindConstantBufferVS(2, g_theRenderer->m_cameraCBO);
	g_theRenderer->BindConstantBufferPS(2, g_theRenderer->m_cameraCBO);

	const StructuredBuffer* sim =
		(m_useBufferAForUpdate ? m_particleBufferB : m_particleBufferA);

	if (!sim)
	{
		if (previousShader) g_theRenderer->BindShader(previousShader);
		return;
	}

	sim->BindAsSRV(0);

	if (m_particleTexture)
	{
		g_theRenderer->BindTexture(m_particleTexture, 1);
	}

	{
		ID3D11SamplerState* samp = g_theRenderer->GetSamplerState();
		g_theRenderer->GetDeviceContext()->PSSetSamplers(0, 1, &samp);
	}

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::READ_ONLY_LESS_EQUAL);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetStatesIfChanged();

	auto* ctx = g_theRenderer->GetDeviceContext();
	ctx->IASetInputLayout(nullptr);
	ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11RenderTargetView* rtv = g_theRenderer->GetMainRTV();
	ID3D11DepthStencilView* dsv = g_theRenderer->GetMainDSV_ReadOnly();

	ctx->OMSetRenderTargets(1, &rtv, dsv);

	ctx->DrawInstanced(6, m_activeParticleCount, 0, 0);

	sim->UnbindAsSRV(0);

	if (previousShader)
		g_theRenderer->BindShader(previousShader);
}



void ParticleEmitter::SetTransform(const Mat44& transform)
{
	m_transform = transform;
}

void ParticleEmitter::SetPosition(const Vec3& position)
{
	m_config.position = position;
}

void ParticleEmitter::Restart()
{
	Shutdown();

	Startup();
}

bool ParticleEmitter::IsAlive() const
{
	if (!m_config.enabled)
		return false;

	if (m_config.isLooping)
		return true;

	if (m_config.duration <= 0.0f)
		return true;

	return m_systemAge < m_config.duration || m_activeParticleCount > 0;
}

void ParticleEmitter::UpdateConfig(const ParticleEmitterConfig& config)
{
	m_config = config;
}




void ParticleEmitter::InitializeBuffers()
{
	m_particleBufferA = g_theRenderer->CreateStructuredBuffer(
		sizeof(Particle), m_config.maxParticles, /*SRV*/true, /*UAV*/true);

	m_particleBufferB = g_theRenderer->CreateStructuredBuffer(
		sizeof(Particle), m_config.maxParticles, /*SRV*/true, /*UAV*/true);


	m_systemCBO = g_theRenderer->CreateConstantBuffer(sizeof(ParticleSystemConstants));
	m_emitterCBO = g_theRenderer->CreateConstantBuffer(sizeof(ParticleEmitterConstants));

	m_spawnCounter = g_theRenderer->CreateStructuredBuffer(sizeof(uint32_t), 1, /*SRV*/true, /*UAV*/true);

	m_aliveCounter = g_theRenderer->CreateStructuredBuffer(sizeof(uint32_t), 1, /*srv=*/true, /*uav=*/true);

	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(uint32_t);
	bd.Usage = D3D11_USAGE_STAGING;
	bd.BindFlags = 0;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bd.MiscFlags = 0;

	ID3D11Device* dev = g_theRenderer->GetDevice();
	if (dev) 
	{
		dev->CreateBuffer(&bd, nullptr, &m_aliveCounterStaging);
	}

	uint32_t zero = 0;
	m_spawnCounter->Update(&zero, 1);
	m_aliveCounter->Update(&zero, 1);

	m_softCBO = g_theRenderer->CreateConstantBuffer(sizeof(SoftParams));
	m_cullingCBO = g_theRenderer->CreateConstantBuffer(sizeof(CullingConstants));
}


void ParticleEmitter::InitializeShaders()
{
	m_updateShader = g_theRenderer->CreateComputeShader("Data/Shaders/ParticleUpdate.hlsl", "CSMain");

	m_renderShader = g_theRenderer->CreateShader("Data/Shaders/ParticleRender", VertexType::Vertex_None, "ParticleVertexMain");
}

void ParticleEmitter::SpawnParticles(float deltaSeconds)
{
	if (m_config.maxParticles == 0)
		return;

	m_spawnAccumulator += deltaSeconds;

	float particlesToSpawn = m_spawnAccumulator * m_config.spawnRate;
	uint32_t numToSpawn = static_cast<uint32_t>(particlesToSpawn);

	if (numToSpawn > 0)
	{
		m_spawnAccumulator -= numToSpawn / m_config.spawnRate;
		numToSpawn = Min(numToSpawn, m_config.maxParticles - m_activeParticleCount);

		if (numToSpawn > 0)
		{
			m_activeParticleCount += numToSpawn;
		}
	}
}


void ParticleEmitter::CreateGPUQueries()
{
	ID3D11Device* dev = g_theRenderer->GetDevice();

	D3D11_QUERY_DESC desc{};
	desc.MiscFlags = 0;

	desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	dev->CreateQuery(&desc, &m_gpuDisjoint);

	desc.Query = D3D11_QUERY_TIMESTAMP;
	dev->CreateQuery(&desc, &m_gpuTimestampBeg);

	desc.Query = D3D11_QUERY_TIMESTAMP;
	dev->CreateQuery(&desc, &m_gpuTimestampEnd);
}



