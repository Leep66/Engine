#include "ParticleSystem.hpp"
#include "Engine/ParticleSystem/ParticleForce.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

ParticleSystem::ParticleSystem(ParticleSystemConfig const& config)
	: m_config(config)
{
	m_forceBuffer = g_theRenderer->CreateStructuredBuffer(
		sizeof(ParticleForce), m_config.m_maxForces, /*srv=*/true, /*uav=*/false);
	std::vector<ParticleForce> zeroes(m_config.m_maxForces, ParticleForce{});

	if (m_forceBuffer) m_forceBuffer->Update(zeroes.data(), (uint32_t)zeroes.size());
}

ParticleSystem::~ParticleSystem()
{
	Shutdown();
}

void ParticleSystem::Shutdown()
{
	for (auto* emitter : m_emitters) {
		if (emitter) 
		{
			delete emitter;
		}
	}
	m_emitters.clear();

	SAFE_DELETE(m_forceBuffer);
}

void ParticleSystem::Update(float /*deltaSeconds*/)
{
	/*for (auto* emitter : m_emitters) {
		if (emitter)
		{
			emitter->Update(deltaSeconds);
		}
	}*/
}

void ParticleSystem::Render() const
{
	/*for (auto* emitter : m_emitters) {
		if (emitter)
		{
			emitter->Render();
		}
	}*/
}

ParticleEmitter* ParticleSystem::CreateEmitter(const ParticleEmitterConfig& config)
{
	ParticleEmitter* emitter = new ParticleEmitter(config);
	m_emitters.push_back(emitter);
	return emitter;
}

void ParticleSystem::DestroyEmitter(const std::string& name)
{
	for (auto it = m_emitters.begin(); it != m_emitters.end();)
	{
		if ((*it)->GetName() == name)
		{
			(*it)->Shutdown();
			delete* it;
			it = m_emitters.erase(it); 
		}
		else
		{
			++it;
		}
	}
}

void ParticleSystem::DestroyEmitter(ParticleEmitter* emitter)
{
	for (auto it = m_emitters.begin(); it != m_emitters.end();)
	{
		if (*it == emitter)
		{
			(*it)->Shutdown();
			delete* it;
			it = m_emitters.erase(it); 
		}
		else
		{
			++it;
		}
	}
}

ParticleEmitter* ParticleSystem::GetEmitter(const std::string& name)
{
	for (ParticleEmitter* emitter : m_emitters)
	{
		if (emitter->GetName() == name)
		{
			return emitter;
		}
	}
	return nullptr;
}

void ParticleSystem::SetEmitterEnabled(const std::string& name, bool enabled)
{
	ParticleEmitter* emitter = GetEmitter(name);
	if (emitter)
	{
		emitter->SetEnabled(enabled);
	}
}

void ParticleSystem::RestartAllEmitters()
{
	for (ParticleEmitter* emitter : m_emitters)
	{
		emitter->Restart();
	}
}

void ParticleSystem::PauseAllEmitters(bool pause)
{
	for (ParticleEmitter* emitter : m_emitters)
	{
		emitter->SetPaused(pause);
	}
}



int ParticleSystem::GetAllParticlesCount() const
{
	int amount = 0;
	for (ParticleEmitter* emitter : m_emitters)
	{
		amount += emitter->GetActiveParticleCount();
	}
	return amount;
}

void ParticleSystem::UploadForces(const std::vector<ParticleForce>& forces)
{
	const uint32_t maxForces = m_config.m_maxForces;
	const uint32_t count = Min(static_cast<uint32_t>(forces.size()), maxForces);

	std::vector<ParticleForce> upload(maxForces);

	std::fill(upload.begin(), upload.end(), ParticleForce{});

	for (uint32_t i = 0; i < count; ++i)
	{
		upload[i] = forces[i];

		if (upload[i].m_forceType == FORCE_GRAVITY ||
			upload[i].m_forceType == FORCE_DIRECTION ||
			upload[i].m_forceType == FORCE_FLOWCOLUMN)
		{
			Vec3 dir(
				upload[i].m_direction[0],
				upload[i].m_direction[1],
				upload[i].m_direction[2]
			);

			float lenSq = dir.GetLengthSquared();
			if (lenSq > 1e-6f)
			{
				Vec3 n = dir / Sqrt(lenSq);
				upload[i].m_direction[0] = n.x;
				upload[i].m_direction[1] = n.y;
				upload[i].m_direction[2] = n.z;
			}
			else
			{
				if (upload[i].m_forceType == FORCE_FLOWCOLUMN)
				{
					upload[i].m_direction[0] = 0.f;
					upload[i].m_direction[1] = 0.f;
					upload[i].m_direction[2] = 1.f;
				}
				else
				{
					upload[i].m_direction[0] = 0.f;
					upload[i].m_direction[1] = 0.f;
					upload[i].m_direction[2] = -1.f;
				}
			}
		}
	}

	if (m_forceBuffer)
	{
		m_forceBuffer->Update(upload.data(), maxForces);
	}
}



uint32_t ParticleSystem::AddForce(const ParticleForce& f)
{
	m_forces.push_back(f);
	m_forcesDirty = true;
	return uint32_t(m_forces.size() - 1);
}

ParticleForce* ParticleSystem::GetForce(uint32_t h)
{
	if (h >= m_forces.size()) return nullptr;
	return &m_forces[h];
}

void ParticleSystem::SetForce(uint32_t h, const ParticleForce& f)
{
	if (h >= m_forces.size()) return;
	m_forces[h] = f;
	m_forcesDirty = true;
}

bool ParticleSystem::RemoveForce(uint32_t idx) 
{
	if (idx >= m_forces.size()) return false;
	m_forces.erase(m_forces.begin() + idx);
	m_forcesDirty = true;
	return true;
}

void ParticleSystem::ClearForces()
{
	m_forces.clear();
	m_forcesDirty = true;
}

void ParticleSystem::ReplaceAllForces(const std::vector<ParticleForce>& forces) 
{
	m_forces = forces; 
	m_forcesDirty = true;
}


void ParticleSystem::EnableForce(uint32_t h, bool on)
{
	if (auto* pf = GetForce(h)) { pf->SetEnabled(on); m_forcesDirty = true; }
}

void ParticleSystem::ToggleForce(uint32_t h)
{
	if (auto* pf = GetForce(h)) { pf->ToggleEnabled(); m_forcesDirty = true; }
}

void ParticleSystem::UploadForcesIfDirty()
{
 	if (!m_forcesDirty) return;
	UploadForces(m_forces);
	m_forcesDirty = false;
}
