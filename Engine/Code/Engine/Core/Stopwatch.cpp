#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Clock.hpp"

Stopwatch::Stopwatch()
	: m_clock(&Clock::GetSystemClock())
{
}

Stopwatch::Stopwatch(float duration) :
	m_clock(&Clock::GetSystemClock()), 
	m_duration(duration)
{
	if (duration == 0.f)
	{
		m_invDuration = -2.f;
	}
	else
	{
		m_invDuration = 1.f / m_duration;
	}
}

Stopwatch::Stopwatch(Clock const* parentClock) :
	m_clock(parentClock)
{
}

Stopwatch::Stopwatch(Clock const* clock, float duration) :
	m_clock(clock),
	m_duration(duration)
{
	m_invDuration = 1.f / m_duration;
}

void Stopwatch::Start()
{
	m_startTime = m_clock->GetTotalSeconds();
}

void Stopwatch::Start(float duration)
{
	m_duration = duration;
	m_invDuration = 1.f / m_duration;
	m_startTime = m_clock->GetTotalSeconds();
}

void Stopwatch::Restart()
{
	if (!IsStopped())
	{
		Start();
	}
}

void Stopwatch::Stop()
{
	m_startTime = 0;
}

float Stopwatch::GetElapsedTime() const
{
	if (IsStopped())
	{
		return 0.f;
	}
	float timeElapsed = m_clock->GetTotalSeconds() - m_startTime;
	return timeElapsed;
}

float Stopwatch::GetElapsedFraction() const
{
	float elapsedTime = GetElapsedTime();
	if (elapsedTime == 0.f && m_duration == 0.f)
	{
		return 0.f;
	}
	float noOfDurations = elapsedTime * m_invDuration;
	return noOfDurations;
}

bool Stopwatch::IsStopped() const
{
	return m_startTime == 0.f;
}

bool Stopwatch::HasDurationElapsed() const
{
	float elapsedTime = GetElapsedTime();
	return elapsedTime >= m_duration;
}

bool Stopwatch::DecrementDurationIfElapsed()
{
	if (HasDurationElapsed() && !IsStopped())
	{
		m_startTime += m_duration;
		return true;
	}
	return false;
}
