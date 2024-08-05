#include "Engine/Math/RandomNumberGenerator.hpp"
#include "ThirdParty/Squirrel/RawNoise.hpp"

//--------------------------------------------------------------------------------------------------
constexpr unsigned int MAX_RANDOM_UINT = 0xFF'FF'FF'FF;
constexpr double ONE_OVER_MAX_RANDOM_UINT = 1.0 / double(MAX_RANDOM_UINT);


//--------------------------------------------------------------------------------------------------
void RandomNumberGenerator::SetSeed(unsigned int newSeed)
{
	m_seed = newSeed;
}


//--------------------------------------------------------------------------------------------------
int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive)
{
	// return rand() % maxNotInclusive;
	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_seed);
	return randomUInt % maxNotInclusive;
}


//--------------------------------------------------------------------------------------------------
int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive)
{
	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_seed);
	int range = 1 + maxInclusive - minInclusive;
	return minInclusive + (randomUInt % range);
	// return ((randomUInt % (maxExclusive - minExclusive + 1)) + minExclusive);
}


//--------------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_seed);
	return float((double)randomUInt * ONE_OVER_MAX_RANDOM_UINT);
}


//--------------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive)
{
	// unsigned int randomUInt = Get1dNoiseUint(m_position++, m_seed);
	// return float(double(randomUInt) * double((maxInclusive - minInclusive)) * ONE_OVER_MAX_RANDOM_UINT);
	float range = maxInclusive - minInclusive;
	return minInclusive + (RollRandomFloatZeroToOne() * range);
}
