#pragma once
#include <vector>
#include "SpriteDefinition.hpp"
#include "Engine/Math/IntVec2.hpp"

class Texture;
struct AABB2;
struct Vec2;

class SpriteDefinition;

constexpr float SPRITESHEET_TEXEL_RESOLUTION = 1.f / 8.f;
constexpr float TEXTURE_OFFSET = SPRITESHEET_TEXEL_RESOLUTION * (1.f / 128.f);

class SpriteSheet
{
public:
	explicit SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout);

	Texture& GetTexture() const;
	int GetNumSprites() const;
	IntVec2 GetSpriteSheetDimensions() const;
	SpriteDefinition const& GetSpriteDef(int spriteIndex) const;
	void GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const;
	AABB2 GetSpriteUVs(int spriteIndex) const;
	AABB2 GetSpriteUVs(int spriteCoordX, int spriteCoordY) const;
	AABB2 GetSpriteUVs(IntVec2 spriteCoord) const;

protected:
	Texture& m_texture;
	IntVec2 m_spriteSheetDims;
	std::vector<SpriteDefinition> m_spriteDefs;
};