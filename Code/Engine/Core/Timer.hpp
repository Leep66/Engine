#pragma once

class Clock;

class Timer
{
public:
	explicit Timer(float period, const Clock* clock = nullptr);

	void Start();
	void Stop();
	float GetElapsedTime() const;
	float GetElapsedFraction() const;
	bool IsStopped() const;
	bool HasPeriodElapsed() const;
	bool DecrementPeriodIfElapsed();
	void ForceComplete();
	float GetRemainingTime() const;

	const Clock* m_clock = nullptr;

	double m_startTime = 0.f;
	double m_period = 0.f;
};