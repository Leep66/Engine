#pragma once
#include "Engine/ParticleSystem/ParticleEmitter.hpp"
#include "Engine/Renderer/Camera.hpp"
#include <vector>
#include <string>

struct StructedBuffer;
class ParticleForce;
class Game;

struct ParticleSystemConfig
{
	int m_maxParticles = 10000000;
	int m_maxForces = 10;
	Game* m_game = nullptr;
};

class ParticleSystem
{
public:
	ParticleSystem(ParticleSystemConfig const& config);
	~ParticleSystem();

	void Update(float deltaSeconds);
	void Render() const;

	ParticleEmitter* CreateEmitter(const ParticleEmitterConfig& config);
	std::vector<ParticleEmitter*> GetEmitters() const { return m_emitters; }
	void DestroyEmitter(const std::string& name);
	void DestroyEmitter(ParticleEmitter* emitter);

	ParticleEmitter* GetEmitter(const std::string& name);
	void SetEmitterEnabled(const std::string& name, bool enabled);

	void RestartAllEmitters();
	void PauseAllEmitters(bool pause);

	void Shutdown();
	int GetAllParticlesCount() const;

	void UploadForces(const std::vector<ParticleForce>& forces);

	uint32_t AddForce(const ParticleForce& f);

	ParticleForce* GetForce(uint32_t handle);
	void SetForce(uint32_t handle, const ParticleForce& f);

	bool RemoveForce(uint32_t idx);
	void ClearForces();                             
	void ReplaceAllForces(const std::vector<ParticleForce>& forces);

	
	void EnableForce(uint32_t handle, bool on);
	void ToggleForce(uint32_t handle);
	void UploadForcesIfDirty();
	StructuredBuffer* GetForceBuffer() const { return m_forceBuffer; }
	std::vector<ParticleForce> GetForces() const { return m_forces; }

	

	Camera m_camera;
	ParticleSystemConfig m_config;
	bool m_forcesDirty = false;

private:
	
	std::vector<ParticleEmitter*> m_emitters;

	std::vector<ParticleForce> m_forces;

	StructuredBuffer* m_forceBuffer = nullptr;

};