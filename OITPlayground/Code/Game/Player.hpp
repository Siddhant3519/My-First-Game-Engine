#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Renderer/Camera.hpp"


//--------------------------------------------------------------------------------------------------
class Game;
class Clock;

//--------------------------------------------------------------------------------------------------
class Player
{
public:
	Player(Game* owner, Vec3 initialPosition, float clientAspect);
	~Player();

	void Update();
	void Render() const;

	void InputUpdate();

public:
	Camera			m_camera;
	Game*			m_owner = nullptr;
	Clock*			m_clock = nullptr;
	float			m_speed = 5.f;
};