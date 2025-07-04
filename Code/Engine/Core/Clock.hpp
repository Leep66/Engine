#pragma once
#include <vector>

class Clock
{
public:
	Clock();

	explicit Clock(Clock& parent);

	~Clock();
	Clock(const Clock& copy) = delete;

	void Reset();

	bool IsPaused() const;
	void Pause();
	void Unpause();
	void TogglePause();

	void StepSingleFrame();

	void SetTimeScale(float timeScale);
	float GetTimeScale() const;

	float GetDeltaSeconds() const;
	float GetTotalSeconds() const;
	int GetFrameCount() const;

public:
	static Clock& GetSystemClock();
	static void TickSystemClock();


protected:
	void Tick();
	void Advance(double deltaTimeSeconds);
	void AddChild(Clock* childClock);
	void RemoveChild(Clock* childClock);

protected:
	Clock* m_parent = nullptr;

	std::vector<Clock*> m_children;

	double m_lastUpdateTimeInSeconds = 0.f;
	double m_totalSeconds = 0.f;
	double m_deltaSeconds = 0.f;
	int m_frameCount = 0;

	double m_timeScale = 1.f;

	bool m_isPaused = false;

	float m_maxDeltaSeconds = 0.1f;
	float m_minDeltaSeconds = 1.f / 120.f;

	bool m_stepSingleFrame = false;
};