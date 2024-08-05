#pragma once

class RandomNumberGenerator
{
public:

	void SetSeed(unsigned int newSeed);

	int RollRandomIntLessThan(int maxNotInclusive);
	int RollRandomIntInRange(int minExclusive, int maxExclusive);
	float RollRandomFloatZeroToOne();
	float RollRandomFloatInRange(float minInclusive, float maxInclusive);

public:
	unsigned int	m_seed = 0;
	int				m_position = 0;
};