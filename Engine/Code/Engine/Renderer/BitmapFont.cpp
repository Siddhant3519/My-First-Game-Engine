#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Mat44.hpp"

BitmapFont::BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture) :
	m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension),
	m_fontGlyphsSpriteSheet(fontTexture, IntVec2(16, 16))
{
}

Texture& BitmapFont::GetTexture()
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

void BitmapFont::AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect)
{
	// TODO(sid): add support for texts in any orientation
	float cellWidth = cellAspect * cellHeight;
	AABB2 positionBounds;
	positionBounds.m_mins = textMins;
	positionBounds.m_maxs.y = positionBounds.m_mins.y + cellHeight;
	positionBounds.m_maxs.x = positionBounds.m_mins.x + cellWidth;

	for (int textIndex = 0; textIndex < text.size(); ++textIndex)
	{
		int spriteIndex = text[textIndex];
		AABB2 uvbounds = m_fontGlyphsSpriteSheet.GetSpriteUVs(spriteIndex);
		IntVec2 spriteSheetDims = m_fontGlyphsSpriteSheet.GetSpriteSheetDimensions();
		AddVertsForAABB2D(vertexArray, positionBounds, tint, uvbounds);
		positionBounds.m_mins.x += cellWidth;
		positionBounds.m_maxs.x += cellWidth;
	}
}

void BitmapFont::AddVertsForText3D(std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, int maxGlyphsToDraw)
{
	(void)maxGlyphsToDraw;
	std::vector<Vertex_PCU> textVerts;
	AddVertsForText2D(textVerts, textMins, cellHeight, text, tint, cellAspect);

	AABB2 textBounds = GetVertexBounds2D(textVerts);
	Vec2 translation;
	translation.x = abs(textBounds.m_maxs.x - textBounds.m_mins.x);
	translation.y = abs(textBounds.m_maxs.y - textBounds.m_mins.y);

	translation.x *= alignment.x;
	translation.y *= alignment.y;

	Mat44 originAligned;
	originAligned.SetIJK3D(Vec3::Y_AXIS, Vec3::Z_AXIS, Vec3::X_AXIS);
	originAligned.AppendTranslation3D(Vec3(-translation.x, -translation.y));
	TransformVertexArray3D(textVerts, originAligned);
	
	verts.insert(verts.end(), textVerts.begin(), textVerts.end());
}

//--------------------------------------------------------------------------------------------------
void BitmapFont::AddVertsForText3D(std::vector<Vertex_PCU>& verts, Vec3 const& textOrigin, Vec3 const& iBasis, Vec3 const& jBasis, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect)
{
	// Add Verts in 2D first (in identity world position)
	int numVertsBefore	= int(verts.size());
	AddVertsForText2D(verts, Vec2(), cellHeight, text, tint, cellAspect);
	int numVertsAfter	= int(verts.size());
	int numVertsAdded	= numVertsAfter - numVertsBefore;
	
	// Transform just the new verts into their proper 3D position
	int indexWhereNewVertsStart = numVertsBefore;
	Vec3 kBasis	= CrossProduct3D(iBasis, jBasis).GetNormalized();
	Mat44 transform (iBasis, jBasis, kBasis, textOrigin);
	
	TransformVertexArray3D(numVertsAdded, &verts[indexWhereNewVertsStart], transform);
}

//--------------------------------------------------------------------------------------------------
void BitmapFont::AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect,
	Vec2 const& alignment, TextDrawMode mode, int maxGlyphsToDraw)
{
	Strings newLineDelimitedTexts = SplitStringOnDelimiter(text, '\n');
	int numOfLines = (int)newLineDelimitedTexts.size();
	float paragraphHeight = cellHeight * numOfLines;

	float maxLength = -1.f;
	for (int arrayIndex = 0; arrayIndex < numOfLines; ++arrayIndex)
	{
		float currentTextWidth = GetTextWidth(cellHeight, newLineDelimitedTexts[arrayIndex], cellAspect);
		if (maxLength < currentTextWidth)
		{
			maxLength = currentTextWidth;
		}
	}

	// TODO(sid): remove redundant code
	Vec2 const& boxDims = box.GetDimensions();
	float paragraphWidth = maxLength;
	float unusedBoxSpaceX = boxDims.x - paragraphWidth;
	float unusedBoxSpaceY = boxDims.y - paragraphHeight;
	
	float localParagraphMinsX = alignment.x * unusedBoxSpaceX;
	float localParagraphMinsY = alignment.y * unusedBoxSpaceY;

	if (mode == TextDrawMode::SHRINK)
	{
		// TODO(sid): replace the following code with a simple check, where we find the shrink factor
		// clamp it irrespective of overflow to be between 0-1
		if (unusedBoxSpaceX < 0 && unusedBoxSpaceX < unusedBoxSpaceY)
		{
			float ratio = boxDims.x / paragraphWidth;
			cellHeight *= ratio;
			paragraphWidth = boxDims.x;
			paragraphHeight *= ratio;
		
			unusedBoxSpaceX = boxDims.x - paragraphWidth;
			unusedBoxSpaceY = boxDims.y - paragraphHeight;
		
			localParagraphMinsX = alignment.x * unusedBoxSpaceX;
			localParagraphMinsY = alignment.y * unusedBoxSpaceY;
		}

		else if (unusedBoxSpaceY < 0 && unusedBoxSpaceY < unusedBoxSpaceX)
		{
			float ratio = boxDims.y / paragraphHeight;
			cellHeight *= ratio;
			paragraphWidth *= ratio;
			paragraphHeight = boxDims.y;
		
			unusedBoxSpaceX = boxDims.x - paragraphWidth;
			unusedBoxSpaceY = boxDims.y - paragraphHeight;
		
			localParagraphMinsX = alignment.x * unusedBoxSpaceX;
			localParagraphMinsY = alignment.y * unusedBoxSpaceY;
		}
	}

	Vec2 paragraphMins;
	paragraphMins.x = box.m_mins.x + localParagraphMinsX;
	paragraphMins.y = box.m_mins.y + localParagraphMinsY;

	Vec2 textMins;
	textMins.y = paragraphMins.y + ((numOfLines - 1) * cellHeight);
	int glyphsToBeDisplayed = maxGlyphsToDraw;
	for (int stringIndex = 0; stringIndex < numOfLines; ++stringIndex)
	{
		int currentStringLength = (int)newLineDelimitedTexts[stringIndex].size();
		float currentTextWidth = GetTextWidth(cellHeight, newLineDelimitedTexts[stringIndex], cellAspect);
		float unusedParaSpaceX = paragraphWidth - currentTextWidth;
		float localTextMinsX = alignment.x * unusedParaSpaceX;
		textMins.x = paragraphMins.x + localTextMinsX;
		if (glyphsToBeDisplayed >= 0)
		{
			if (glyphsToBeDisplayed > currentStringLength)
			{
				AddVertsForText2D(vertexArray, textMins, cellHeight, newLineDelimitedTexts[stringIndex], tint, cellAspect);
				glyphsToBeDisplayed -= currentStringLength;
			}
			else
			{
				std::string const& glyphsToRender = std::string(newLineDelimitedTexts[stringIndex], 0, glyphsToBeDisplayed);
				AddVertsForText2D(vertexArray, textMins, cellHeight, glyphsToRender, tint, cellAspect);
				glyphsToBeDisplayed = 0;
			}
		}
		else
		{
			break;
		}
		
		textMins.y -= cellHeight;
	}
}

float BitmapFont::GetTextWidth(float cellHeight, std::string const& text, float cellAspect)
{
	float textWidth = 0.f;
	float cellWidth = cellAspect * cellHeight;
	for (int textIndex = 0; textIndex < (int)text.size(); ++textIndex)
	{
		textWidth += cellWidth * GetGlyphAspect(text[textIndex]);
	}
	return textWidth;
}

float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	(void)glyphUnicode;
	// TODO(sid): fill in the right logic
	return 1.0f;
}
