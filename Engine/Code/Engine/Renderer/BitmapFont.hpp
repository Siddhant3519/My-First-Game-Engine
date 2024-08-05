#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/Vec3.hpp"

#include <vector>
#include <string>

class Texture;
struct Vertex_PCU;
struct Vec2;

enum class TextDrawMode
{
	SHRINK,
	OVERRUN,
};

class BitmapFont
{
	friend class Renderer;

private:
	BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture);

public:
	Texture& GetTexture();
	
	void AddVertsForText2D		(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8(255, 255, 255), float cellAspect = 1.f);
	void AddVertsForText3D		(std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f, Vec2 const& alignment = Vec2(0.5f, 0.5f), int maxGlyphsToDraw = 999999999);
	void AddVertsForText3D		(std::vector<Vertex_PCU>& verts, Vec3 const& textOrigin, Vec3 const& iBasis, Vec3 const& jBasis, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect);
	void AddVertsForTextInBox2D	(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8(255, 255, 255), float cellAspect = 1.f,
		Vec2 const& alignment = Vec2(0.5f, 0.5f), TextDrawMode mode = TextDrawMode::SHRINK, int maxGlyphsToDraw = 99999999);

	float GetTextWidth(float cellHeight, std::string const& text, float cellAspect = 1.f);

protected:
	float GetGlyphAspect(int glyphUnicode) const; // NOTE(sid): for now will return 1.f;

protected:
	std::string m_fontFilePathNameWithNoExtension;
	SpriteSheet m_fontGlyphsSpriteSheet;
};