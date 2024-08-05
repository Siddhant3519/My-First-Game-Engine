#pragma once

class Clock;

class Stopwatch
{
public:
	Stopwatch();
	explicit Stopwatch(float duration);
	Stopwatch(Clock const* parentClock);
	Stopwatch(Clock const* clock, float duration);

	void Start();
	void Start(float duration);
	void Restart();
	void Stop();
	
	float GetElapsedTime() const;
	float GetElapsedFraction() const;
	bool IsStopped() const;
	bool HasDurationElapsed() const;
	bool DecrementDurationIfElapsed();

public:
	Clock const* m_clock = nullptr;
	float m_startTime = 0.f;
	float m_duration = 0.f;
	float m_invDuration = 0.f;
};