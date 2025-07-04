#include "Engine/Core/Timer.hpp"
#include "Engine/Core/Clock.hpp"

Timer::Timer(float period, const Clock* clock)
	: m_clock(clock)
	, m_period(period)
{
}

void Timer::Start()
{
	if (m_clock != nullptr) 
	{
		m_startTime = m_clock->GetTotalSeconds();
	}
}

void Timer::Stop()
{
	m_startTime = 0.f;
}

float Timer::GetElapsedTime() const
{
	if (IsStopped()) {
		return 0.f;
	}
	return (float)(m_clock->GetTotalSeconds() - m_startTime);
}

float Timer::GetElapsedFraction() const
{
	if (m_period == 0.0f) {
		return 0.0f;
	}

	float elapsedTime = GetElapsedTime();
	return  elapsedTime / (float) m_period;
}

bool Timer::IsStopped() const
{
	return m_startTime == 0.0;
}

bool Timer::HasPeriodElapsed() const
{
	return GetElapsedTime() > m_period && !IsStopped();
}

bool Timer::DecrementPeriodIfElapsed()
{
	if (HasPeriodElapsed()) {
		m_startTime += m_period;
		return true;
	}

	return false;
}

void Timer::ForceComplete()
{
	if (m_clock != nullptr)
	{
		m_startTime = m_clock->GetTotalSeconds() - m_period;
	}
}

float Timer::GetRemainingTime() const
{
	if (IsStopped()) return 0.f;
	return (float)m_period - GetElapsedTime();
}
