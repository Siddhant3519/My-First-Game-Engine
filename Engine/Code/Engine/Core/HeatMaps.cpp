#include "Engine/Core/HeatMaps.hpp"

TileHeatMap::TileHeatMap(IntVec2 const& dimensions, float defaultValue) :
	m_dimensions(dimensions)
{
	int numOfTiles = dimensions.x * dimensions.y;
	m_values.resize(numOfTiles);
	SetAllValues(defaultValue);
}

float TileHeatMap::GetHeatValueAt(IntVec2 const& tileCoords) const
{
	int tileIndex = tileCoords.x + (tileCoords.y * m_dimensions.x);
	return m_values[tileIndex];
}

void TileHeatMap::SetAllValues(float resetValue)
{
	for (int tileIndex = 0; tileIndex < (int)m_values.size(); ++tileIndex)
	{
		m_values[tileIndex] = resetValue;
	}
}

void TileHeatMap::SetHeatValueAt(IntVec2 const& tileCoords, float heatValueToSet)
{
	int tileIndex = tileCoords.x + (tileCoords.y * m_dimensions.x);
	m_values[tileIndex] = heatValueToSet;
}

void TileHeatMap::AddHeatValueAt(IntVec2 const& tileCoords, float heatValueToAdd)
{
	int tileIndex = tileCoords.x + (tileCoords.y * m_dimensions.x);
	m_values[tileIndex] += heatValueToAdd;
}
