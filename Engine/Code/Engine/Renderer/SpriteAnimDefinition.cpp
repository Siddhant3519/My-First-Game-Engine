#include "SpriteAnimDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


SpriteAnimDefinition::SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex, float framesPerSecond, SpriteAnimPlaybackType playbackType) :
	m_spriteSheet(sheet), m_startSpriteIndex(startSpriteIndex), m_endSpriteIndex(endSpriteIndex), m_secondsPerFrame(1.f / framesPerSecond), m_playbackType(playbackType)
{
}

SpriteAnimDefinition::SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex, float secondsPerFrame, SpriteAnimPlaybackType playbackType, float duration, Vec3 direction) :
	m_spriteSheet(sheet), m_startSpriteIndex(startSpriteIndex), m_endSpriteIndex(endSpriteIndex), m_secondsPerFrame(secondsPerFrame), m_playbackType(playbackType),
	m_duration(duration), m_direction(direction)
{
}

SpriteDefinition const& SpriteAnimDefinition::GetSpriteDefAtTime(float seconds) const
{
	int frameIndexAtGivenSeconds = RoundDownToInt(seconds / m_secondsPerFrame);
	int numOfFrames = ABS_INT(m_endSpriteIndex - m_startSpriteIndex) + 1;
	float animationDuration = m_secondsPerFrame * numOfFrames;

	int spriteAnimIndex = m_startSpriteIndex;

	// TODO(sid): add checks for when start and end sprite index are the same
	// TODO(sid): add checks for when the endIndex is smaller than the start index
	// TODO(sid): add checks for rolling back animations
	
	switch (m_playbackType)
	{
		case SpriteAnimPlaybackType::ONCE:
		{
			if (seconds > animationDuration)
			{
				spriteAnimIndex = m_endSpriteIndex;
			}
			else
			{
				spriteAnimIndex += frameIndexAtGivenSeconds;
			}
			break;
		}
		case SpriteAnimPlaybackType::LOOP:
			if (seconds > animationDuration)
			{
				spriteAnimIndex += frameIndexAtGivenSeconds % numOfFrames;
			}
			else if (seconds < 0)
			{
				spriteAnimIndex = m_startSpriteIndex;
			}
			else
			{
				spriteAnimIndex += frameIndexAtGivenSeconds;
			}
			break;
		case SpriteAnimPlaybackType::PINGPONG:
		{
			// TODO(sid): remove redundant code
			// TODO(sid): add checks for -ve seconds
			// TODO(sid): refactor, make it cleaner and more chunk-able
			// int loopsElapsedSec = RoundDownToInt(seconds / animationDuration);

			int animPlaybackCount = frameIndexAtGivenSeconds / (numOfFrames - 1);
			int animPlaybackRemainder = RoundDownToInt(seconds) % (numOfFrames - 1);

			int firstOrLastPos = frameIndexAtGivenSeconds / (numOfFrames - 1);
			if (firstOrLastPos % 2 == 0 && animPlaybackRemainder == 0)
			{
				spriteAnimIndex = m_startSpriteIndex;
			}
			else if (firstOrLastPos % 2 == 1 && animPlaybackRemainder == 0)
			{
				spriteAnimIndex = m_endSpriteIndex;
			}
			else if (firstOrLastPos % 2 == 1 && animPlaybackRemainder == (numOfFrames - 2))
			{
				spriteAnimIndex = m_startSpriteIndex + 1;
			}
			else if (animPlaybackCount % 2 == 0)
			{
				spriteAnimIndex += (frameIndexAtGivenSeconds % (numOfFrames - 1));
			}
			else
			{
				spriteAnimIndex = (m_endSpriteIndex) - (frameIndexAtGivenSeconds % (numOfFrames - 1));
			}
			break;
		}
		default:
			break;
	}

	SpriteDefinition const& spriteDefAtTime = m_spriteSheet.GetSpriteDef(spriteAnimIndex);
	return spriteDefAtTime;
}

//SpriteAnimGroupDefinition::SpriteAnimGroupDefinition(XmlElement const& spriteAnimGroupDefElement, SpriteSheet spriteSheet)
//{
//	m_name = ParseXmlAttribute(spriteAnimGroupDefElement, "name", m_name);
//	m_scaledBySpeed = ParseXmlAttribute(spriteAnimGroupDefElement, "scaleBySpeed", false);
//	m_secondsPerFrame = ParseXmlAttribute(spriteAnimGroupDefElement, "secondsPerFrame", -1.f);
//	std::string playbackTypeName = ParseXmlAttribute(spriteAnimGroupDefElement, "playbackMode", "INVALID");
//	if (playbackTypeName == "INVALID")
//	{
//		ERROR_AND_DIE("Provide a valid animation playback type");
//	}
//	SetPlayBackTypeByText(playbackTypeName);
//
//	XmlElement const* currentSpriteAnimElement = spriteAnimGroupDefElement.FirstChildElement("Direction");
//
//	while (currentSpriteAnimElement)
//	{
//		// m_direction = ParseXmlAttribute(*currentSpriteAnimElement, "vector", Vec3::ZERO);
//		Vec3 direction = ParseXmlAttribute(*currentSpriteAnimElement, "vector", Vec3::ZERO);
//		direction.Normalize();
//
//		XmlElement const* animationInfoElement = currentSpriteAnimElement->FirstChildElement("Animation");
//		int startFrame = -1;
//		int endFrame = -1;
//		if (animationInfoElement)
//		{
//			startFrame = ParseXmlAttribute(*animationInfoElement, "startFrame", -1);
//			endFrame = ParseXmlAttribute(*animationInfoElement, "endFrame", -1);
//		}
//		int numOfFrames = ABS_INT(endFrame - startFrame) + 1;
//		float duration = numOfFrames * m_secondsPerFrame;
//		// float framesPerSecond = numOfFrames * m_secondsPerFrame;
//		SpriteAnimDefinition currentSpriteAnimDef(spriteSheet, startFrame, endFrame, m_secondsPerFrame, m_playbackType, duration, direction);
//		m_spriteAnimDefs.push_back(currentSpriteAnimDef);
//
//		currentSpriteAnimElement = currentSpriteAnimElement->NextSiblingElement();
//	}
//}

SpriteAnimGroupDefinition::SpriteAnimGroupDefinition(XmlElement const& spriteAnimGroupDefElement, Texture* texture, IntVec2 const& cellCount)
{
	m_name = ParseXmlAttribute(spriteAnimGroupDefElement, "name", m_name);
	m_scaledBySpeed = ParseXmlAttribute(spriteAnimGroupDefElement, "scaleBySpeed", false);
	m_secondsPerFrame = ParseXmlAttribute(spriteAnimGroupDefElement, "secondsPerFrame", -1.f);
	std::string playbackTypeName = ParseXmlAttribute(spriteAnimGroupDefElement, "playbackMode", "INVALID");
	if (playbackTypeName == "INVALID")
	{
		ERROR_AND_DIE("Provide a valid animation playback type");
	}
	SetPlayBackTypeByText(playbackTypeName);

	m_spriteSheet = new SpriteSheet(*texture, cellCount);

	XmlElement const* currentSpriteAnimElement = spriteAnimGroupDefElement.FirstChildElement("Direction");

	while (currentSpriteAnimElement)
	{
		// m_direction = ParseXmlAttribute(*currentSpriteAnimElement, "vector", Vec3::ZERO);
		Vec3 direction = ParseXmlAttribute(*currentSpriteAnimElement, "vector", Vec3::ZERO);
		direction.Normalize();

		XmlElement const* animationInfoElement = currentSpriteAnimElement->FirstChildElement("Animation");
		int startFrame = -1;
		int endFrame = -1;
		if (animationInfoElement)
		{
			startFrame = ParseXmlAttribute(*animationInfoElement, "startFrame", -1);
			endFrame = ParseXmlAttribute(*animationInfoElement, "endFrame", -1);
		}
		int numOfFrames = ABS_INT(endFrame - startFrame) + 1;
		float duration = numOfFrames * m_secondsPerFrame;
		// float framesPerSecond = numOfFrames * m_secondsPerFrame;
		SpriteAnimDefinition* currentSpriteAnimDef = new SpriteAnimDefinition(*m_spriteSheet, startFrame, endFrame, m_secondsPerFrame, m_playbackType, duration, direction);
		m_spriteAnimDefs.push_back(*currentSpriteAnimDef);

		currentSpriteAnimElement = currentSpriteAnimElement->NextSiblingElement();
	}
}

SpriteAnimGroupDefinition::~SpriteAnimGroupDefinition()
{
}

void SpriteAnimGroupDefinition::SetPlayBackTypeByText(std::string const& text)
{
	if (text == "Loop")
	{
		m_playbackType = SpriteAnimPlaybackType::LOOP;
	}
	else if (text == "Once")
	{
		m_playbackType = SpriteAnimPlaybackType::ONCE;
	}
	else if (text == "PingPong")
	{
		m_playbackType = SpriteAnimPlaybackType::PINGPONG;
	}
}

SpriteDefinition const* SpriteAnimGroupDefinition::GetSpriteDefAtTime(float timeInSeconds, Vec3 const& direction) const
{
	float mostSimilarAnimDot = -2.f;
	int animToPlayIndex = -1;
	for (int spriteAnimDefIndex = 0; spriteAnimDefIndex < m_spriteAnimDefs.size(); ++spriteAnimDefIndex)
	{
		float currentSimilarity = DotProduct3D(m_spriteAnimDefs[spriteAnimDefIndex].m_direction, direction);
		if (currentSimilarity > mostSimilarAnimDot)
		{
			mostSimilarAnimDot = currentSimilarity;
			animToPlayIndex = spriteAnimDefIndex;
		}
	}
	if (animToPlayIndex != -1)
	{
		return &(m_spriteAnimDefs[animToPlayIndex].GetSpriteDefAtTime(timeInSeconds));
	}

	return nullptr;
}
