#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/Time.hpp"
#include <thread>

static Clock s_theSystemClock;

Clock::Clock()
{
	if (this != &s_theSystemClock)
	{
		m_parent = &s_theSystemClock;
		m_parent->AddChild(this);
	}
}

Clock::~Clock()
{
	if (m_parent) 
	{
		m_parent->RemoveChild(this);
	}

	for (Clock* child : m_children) 
	{
		child->m_parent = nullptr;
	}

	m_children.clear();
}

Clock::Clock(Clock& parent)
	: m_parent(&parent)
{
	if (m_parent) {
		m_parent->AddChild(this);
	}
}

void Clock::Reset()
{
	m_lastUpdateTimeInSeconds = 0.f;
	m_totalSeconds = 0.f;
	m_deltaSeconds = 0.f;
	m_frameCount = 0;

	m_timeScale = 1.f;

	m_isPaused = false;
	m_stepSingleFrame = false;
}

bool Clock::IsPaused() const
{
	return m_isPaused;
}

void Clock::Pause()
{
	m_isPaused = true;
	for (Clock* child : m_children)
	{
		child->Pause();
	}
}

void Clock::Unpause()
{
	m_isPaused = false;
	for (Clock* child : m_children)
	{
		child->Unpause();
	}
}

void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;

	for (int i = 0; i < (int) m_children.size(); i++)
	{
		if (m_children[i])
		{
			m_children[i]->TogglePause();
		}
	}
}

void Clock::StepSingleFrame()
{
	m_stepSingleFrame = true;

	for (int i = 0; i < (int)m_children.size(); i++)
	{
		if (m_children[i])
		{
			m_children[i]->StepSingleFrame();
		}
	}
}



void Clock::SetTimeScale(float timeScale)
{
	m_timeScale = (float)timeScale;
}

float Clock::GetTimeScale() const
{
	return (float)m_timeScale;
}

float Clock::GetDeltaSeconds() const
{
	return (float)m_deltaSeconds;
}

float Clock::GetTotalSeconds() const
{
	return (float)m_totalSeconds;
}

int Clock::GetFrameCount() const
{
	return m_frameCount;
}

Clock& Clock::GetSystemClock()
{
	return s_theSystemClock;
}

void Clock::TickSystemClock()
{
	s_theSystemClock.Tick();
}

void Clock::Tick()
{
	double currentTime = (double)GetCurrentTimeSeconds();
	double deltaTime = currentTime - m_lastUpdateTimeInSeconds;

	m_lastUpdateTimeInSeconds = currentTime; 

	/*while (deltaTime < m_minDeltaSeconds)
	{
		std::this_thread::yield();
		currentTime = GetCurrentTimeSeconds();
		deltaTime = currentTime;
	}*/

	if (deltaTime > m_maxDeltaSeconds)
	{
		deltaTime = m_maxDeltaSeconds;
	}

	if (m_isPaused && !m_stepSingleFrame)
	{
		m_deltaSeconds = 0.0f;
		return;
	}
	else
	{
		deltaTime *= m_timeScale;
		Advance(deltaTime);

		if (m_stepSingleFrame)
		{
			m_stepSingleFrame = false;
			m_isPaused = true;
		}
	}

	for (Clock* child : m_children)
	{
		child->Tick();
	}
}

void Clock::Advance(double deltaTimeSeconds)
{
	m_deltaSeconds = deltaTimeSeconds;
	m_totalSeconds += deltaTimeSeconds;
	m_frameCount++;
}



void Clock::AddChild(Clock* childClock)
{
	m_children.push_back(childClock);
}

void Clock::RemoveChild(Clock* childClock)
{
	auto it = std::find(m_children.begin(), m_children.end(), childClock);

	if (it != m_children.end()) {
		m_children.erase(it);
	}
}





