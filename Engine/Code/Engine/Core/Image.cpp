#include "Engine/Core/Image.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"

Image::Image(char const* imageFilePath) :
	m_imageFilePath(imageFilePath)
{
	int bytesPerTexel = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* texelData = stbi_load(imageFilePath, &m_dimensions.x, &m_dimensions.y, &bytesPerTexel, numComponentsRequested);
	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("Failed to load image \"%s\"", imageFilePath));
	int numOfPixels = m_dimensions.x * m_dimensions.y;
	m_texelRgba8Data.reserve(numOfPixels);
	Rgba8 currentTexelData;
	for (int texelIndex = 0; texelIndex < numOfPixels; ++texelIndex)
	{
		int dataByteStartIndex = texelIndex * bytesPerTexel;
		currentTexelData.r = *(texelData + dataByteStartIndex);
		if (bytesPerTexel == 1)
		{
			currentTexelData.g = currentTexelData.r;
			currentTexelData.b = currentTexelData.r;
		}
		else if (bytesPerTexel == 3)
		{
			currentTexelData.g = *(texelData + dataByteStartIndex + 1);
			currentTexelData.b = *(texelData + dataByteStartIndex + 2);
		}
		else if (bytesPerTexel == 4)
		{
			currentTexelData.g = *(texelData + dataByteStartIndex + 1);
			currentTexelData.b = *(texelData + dataByteStartIndex + 2);
			currentTexelData.a = *(texelData + dataByteStartIndex + 3);
		}
		m_texelRgba8Data.push_back(currentTexelData);
	}
	stbi_image_free(texelData);
}

Image::Image(IntVec2 size, Rgba8 color) : 
	m_dimensions(size)
{
	int numOfPixels = size.x * size.y;
	m_texelRgba8Data.resize(numOfPixels);

	for (int texelIndex = 0; texelIndex < numOfPixels; ++texelIndex)
	{
		m_texelRgba8Data[texelIndex] = color;
		// m_texelRgba8Data.push_back(color);
	}
}

IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}

Rgba8 Image::GetTexelColor(IntVec2 const& texelCoords) const
{
	int texelIndex = (texelCoords.y * m_dimensions.x) + texelCoords.x;
	return m_texelRgba8Data[texelIndex];
}

void Image::SetTexelColor(IntVec2 const& texelCoords, Rgba8 const& newColor)
{
	(void)texelCoords;
	(void)newColor;
}

std::string const& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}

void const* Image::GetRawData() const
{
	return m_texelRgba8Data.data();
}
