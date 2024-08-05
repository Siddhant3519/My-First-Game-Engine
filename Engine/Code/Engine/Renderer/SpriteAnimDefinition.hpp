#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec3.hpp"

struct IntVec2;
struct SpawnInfo;

class Shader;
class Image;
class Texture;

enum class SpriteAnimPlaybackType
{
	ONCE, 
	LOOP,
	PINGPONG,
};

class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex, 
						 float framesPerSecond = 20.f, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP);
	SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex,
		float framesPerSecond = 20.f, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP, float duration = -1.f, Vec3 direction = Vec3(-1.f, -1.f, -1.f));

	SpriteDefinition const& GetSpriteDefAtTime(float seconds) const;

private:
	SpriteSheet const&				m_spriteSheet;
	int								m_startSpriteIndex = -1;
	int								m_endSpriteIndex = -1;
	float							m_secondsPerFrame = 0.05f;
	SpriteAnimPlaybackType			m_playbackType = SpriteAnimPlaybackType::LOOP;
public:
	float							m_duration = -1.f;
	Vec3							m_direction = Vec3(-1.f, -1.f, -1.f);

	// PLACEHOLDER
	bool firstPass = true;
};

struct SpriteAnimGroupDefinition
{
	SpriteAnimGroupDefinition(XmlElement const& mapDefElement, SpriteSheet spriteSheet);
	SpriteAnimGroupDefinition(XmlElement const& mapDefElement, Texture* texture, IntVec2 const& cellCount);
	~SpriteAnimGroupDefinition();
	void SetPlayBackTypeByText(std::string const& text);
	SpriteDefinition const* GetSpriteDefAtTime(float timeInSeconds, Vec3 const& direction) const;
	std::string							m_name = "UNNAMED ANIMATION DEFINITION";
	SpriteAnimPlaybackType				m_playbackType = SpriteAnimPlaybackType::LOOP;
	bool								m_scaledBySpeed = false;
	float								m_secondsPerFrame = 0.05f;
	// float							m_duration = -1.f;
	SpriteSheet const*					m_spriteSheet = nullptr;;
	std::vector<SpriteAnimDefinition>	m_spriteAnimDefs;
	// Vec3								m_direction;
};