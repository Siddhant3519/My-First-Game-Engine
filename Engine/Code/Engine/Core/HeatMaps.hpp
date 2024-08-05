#pragma once
#include "Engine/Math/IntVec2.hpp"

#include <vector>

struct IntVec2;

class TileHeatMap
{
public:
	TileHeatMap(IntVec2 const& dimensions, float defaultValue = 0);
	
	float GetHeatValueAt(IntVec2 const& tileCoords) const;

	void SetAllValues(float resetValue);
	void SetHeatValueAt(IntVec2 const& tileCoords, float heatValueToSet);
	void AddHeatValueAt(IntVec2 const& tileCoords, float heatValueToAdd);
public:
	std::vector<float>m_values;
	IntVec2 m_dimensions;
};
