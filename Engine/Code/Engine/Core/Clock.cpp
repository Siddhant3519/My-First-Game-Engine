#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"

static Clock g_theSystemClock;

Clock::Clock()
{
	if (this != &g_theSystemClock)
	{
		m_parent = &g_theSystemClock;
		g_theSystemClock.AddChild(this);
	}
}

Clock::Clock(Clock& parent)
{
	m_parent = &parent;
	parent.AddChild(this);
}

void Clock::Reset()
{
	m_totalSeconds = 0.f;
	m_deltaSeconds = 0.f;
	m_frameCount = 0;
	m_lastUpdateTimeInSeconds = (float)GetCurrentTimeSeconds();
}

bool Clock::IsPaused() const
{
	return m_isPaused;
}

void Clock::Pause()
{
	m_isPaused = true;
}

void Clock::Unpause()
{
	m_isPaused = false;
}

void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
}

void Clock::StepSingleFrame()
{
	m_stepSingleFrame = !m_stepSingleFrame;
	if (m_stepSingleFrame)
	{
		m_isPaused = false;
	}
}

void Clock::SetTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

float Clock::GetTimeScale() const
{
	return m_timeScale;
}

float Clock::GetDeltaSeconds() const
{
	return m_deltaSeconds;
}

float Clock::GetTotalSeconds() const
{
	return m_totalSeconds;
}

size_t Clock::GetFrameCount() const
{
	return size_t(m_frameCount);
}

Clock& Clock::GetSystemClock()
{
	return g_theSystemClock;
}

void Clock::TickSystemClock()
{
	g_theSystemClock.Tick();
}

void Clock::Tick()
{
	// constexpr float MAX_DELTA_SECONDS = 0.1f;

	static double s_timeLastFrame = GetCurrentTimeSeconds();
	double timeThisFrame = GetCurrentTimeSeconds();

	float deltaSeconds = static_cast<float>(timeThisFrame - s_timeLastFrame);
	s_timeLastFrame = timeThisFrame;

	if (deltaSeconds > m_maxDeltaSeconds)
	{
		deltaSeconds = m_maxDeltaSeconds;
	}

	Advance(deltaSeconds);
}

void Clock::Advance(float deltaTimeSeconds)
{
	if (m_isPaused)
	{
		m_deltaSeconds = 0;
		for (int childIndex = 0; childIndex < m_children.size(); ++childIndex)
		{
			m_children[childIndex]->Advance(m_deltaSeconds);
		}
	}
	else
	{
		m_deltaSeconds = deltaTimeSeconds * m_timeScale;
		m_lastUpdateTimeInSeconds = (float)GetCurrentTimeSeconds();
		m_totalSeconds += m_deltaSeconds;
		for (int childIndex = 0; childIndex < m_children.size(); ++childIndex)
		{
			m_children[childIndex]->Advance(m_deltaSeconds);
		}
	}
	++m_frameCount;
	if (m_stepSingleFrame)
	{
		m_isPaused = true;
		m_stepSingleFrame = false;
	}
}

void Clock::AddChild(Clock* childClock)
{
	m_children.push_back(childClock);
}

void Clock::RemoveChild(Clock* childClock)
{
	for (int clockIndex = 0; clockIndex < m_children.size(); ++clockIndex)
	{
		if (m_children[clockIndex] == childClock)
		{
			m_children[clockIndex] = m_children[m_children.size() - 1];
			m_children.pop_back();
		}
	}
}

Clock::~Clock()
{
	if (m_parent)
	{
		m_parent->RemoveChild(this);
		for (int childIndex = 0; childIndex < m_children.size(); ++childIndex)
		{
			if (m_children[childIndex])
			{
				m_parent->AddChild(m_children[childIndex]);
				m_children[childIndex]->m_parent = m_parent;
			}
		}
	}
}
