#include "SpriteSheet.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Texture.hpp"

SpriteSheet::SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout) :
	m_texture(texture), m_spriteSheetDims(simpleGridLayout)
{
	float uvScalerX = 1.f / simpleGridLayout.x;
	float uvScalerY = 1.f / simpleGridLayout.y;
	constexpr float OFFSET_CONSTANT = 128.f;
	IntVec2 textureDimensions = m_texture.GetDimensions();
	float textureOffsetX = 1.f / ((float)textureDimensions.x * OFFSET_CONSTANT);
	float textureOffsetY = 1.f / ((float)textureDimensions.y * OFFSET_CONSTANT);

	int spriteIndex = 0;
	for (int spriteYCoord = 0; spriteYCoord < simpleGridLayout.y; ++spriteYCoord)
	{
		for (int spriteXCoord = 0; spriteXCoord < simpleGridLayout.x; ++spriteXCoord)
		{
			// Calculate the UVs for a sprite
			float uvAtMinsX = ((float)spriteXCoord * uvScalerX) + textureOffsetX;
			float uvAtMinsY = (1.f - (float(spriteYCoord + 1) * uvScalerY)) + textureOffsetY;
			float uvAtMaxsX = (float(spriteXCoord + 1) * uvScalerX) - textureOffsetX;
			float uvAtMaxsY = (1.f - (float(spriteYCoord) * uvScalerY)) - textureOffsetY;
		
			m_spriteDefs.push_back(SpriteDefinition(*this, spriteIndex, Vec2(uvAtMinsX, uvAtMinsY), Vec2(uvAtMaxsX, uvAtMaxsY)));

			++spriteIndex;
		}
	}
}

Texture& SpriteSheet::GetTexture() const
{
	return m_texture;
}

int SpriteSheet::GetNumSprites() const
{
	return (int)m_spriteDefs.size();
}

IntVec2 SpriteSheet::GetSpriteSheetDimensions() const
{
	return m_spriteSheetDims;
}

SpriteDefinition const& SpriteSheet::GetSpriteDef(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}

void SpriteSheet::GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const
{
	m_spriteDefs[spriteIndex].GetUVs(out_uvAtMins, out_uvAtMaxs);
}

AABB2 SpriteSheet::GetSpriteUVs(int spriteCoordX, int spriteCoordY) const
{
	int spriteIndex = (spriteCoordY * m_spriteSheetDims.x) + spriteCoordX;
	return GetSpriteUVs(spriteIndex);
}

AABB2 SpriteSheet::GetSpriteUVs(IntVec2 spriteCoord) const
{
	int spriteIndex = (spriteCoord.y * m_spriteSheetDims.x) + spriteCoord.x;
	return GetSpriteUVs(spriteIndex);
}

AABB2 SpriteSheet::GetSpriteUVs(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex].GetUVs();
}
