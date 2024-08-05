
This is some sample code that loads an image file (32-bit .PNG preferred) and creates an OpenGL Texture from it

// 1. Create Engine/Core/Image.cpp,hpp

// 2. Create Engine/Renderer/Texture.cpp,hpp

// 3. At the top of Engine/Core/Image.cpp, do this:
#define STB_IMAGE_IMPLEMENTATION // Exactly one .CPP (this Image.cpp) should #define this before #including stb_image.h
#include "ThirdParty/stb/stb_image.h"

// 4. At the top of Renderer.cpp, add this:
#include "ThirdParty/stb/stb_image.h"

// 5. Test imaging loading in ProtogameCmd or similar; load named image from a path starting with "Data/" and
//	print out the image's width and height, bytes-per-pixel (3 for RGB or 4 for RGBA), and the Rgba8 byte colors
//	of the first four texels in the image data (and also the last texel).

// 6. Create a Texture class (using the example below) in Renderer/Texture.hpp and .cpp

// 7. Write Renderer::CreateTextureFromData (using the example below), ignoring (commenting out) the m_loadedTextures.push_back for now

// 8. Write Renderer::BindTexture (using the example below)

// 9. In Protogame2D (or TestGame2D), draw a simple large square AABB2 without texturing in Attract mode.  Make sure it works.

// 10. In Protogame2D's Game::RenderAttract, create a "fake" 4x4 image in temp stack local memory with very specific Rgba8 colors in it;
//	For example:  top row, left to right: Cyan, Magenta, Yellow, Black; then two rows of checkerboard black and brown (200,100,50,255);
//	then the bottom row, left to right: Red, Green, Blue, White.

// 11. Use g_theRenderer->CreateTextureFromData function to create a 4x4 texture (x 4-channel RGBA per texel) from the dummy image data

// 12. Change your RenderAttract square to use textured rendering (see the SomeGameFunction usage example below).

// 13. Write Renderer::CreateTextureFromFile (see example), which uses STB_image to get the texture data, then calls CreateTextureFromData.

// 14. Try to get your test square to load the Test_StbiFlippedAndOpenGL.png test image and draw it textured.

// 15. Create a "registry" in the Renderer which stores a record of all textures previously created.  Whenever any texture is created by
//	CreateTextureFromData (which will be 100% of the time), add it to the registry.

// 16. Write Renderer::CreateOrGetTextureFromFile, which first checks the registry to see if a texture of that name already exists;
//	do this by calling GetTextureForFileName, which will just return the existing Texture* (or nullptr if no such texture exists).

// 17. Make sure everything works.

// 18. Place breakpoints in a Debug build on CreateTextureFromData and CreateTextureFromFile.  Call CreateOrGetTextureFromFile every
//	frame; make sure that they are each called exactly once -- the first time CreateOrGetTextureFromFile is called; after that, they
//	should not get called again (since the texture should be in the Renderer's registry of already-loaded textures, and just get
//	returned by GetTextureForFileName directly).

// 19. Profit.


//-----------------------------------------------------------------------------------------------
// Sample code for loading an image from disk and creating an OpenGL texture from its data.
// 
// Game code calls Renderer::CreateOrGetTextureFromFile(), which in turn will
//	check that name amongst the registry of already-loaded textures (by name).  If that image
//	has already been loaded, the renderer simply returns the Texture* it already has.  If the
//	image has not been loaded before, CreateTextureFromFile() gets called internally, which in
//	turn calls CreateTextureFromData.  The new Texture* is then added to the registry of
//	already-loaded textures, and then returned.
//------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateOrGetTextureFromFile( char const* imageFilePath )
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureForFileName( imageFilePath );
	if( existingTexture )
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile( imageFilePath );
	return newTexture;
}


//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromFile( char const* imageFilePath )
{
	IntVec2 dimensions = IntVec2::ZERO;		// This will be filled in for us to indicate image width & height
	int bytesPerTexel = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load( 1 ); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* texelData = stbi_load( imageFilePath, &dimensions.x, &dimensions.y, &bytesPerTexel, numComponentsRequested );

	// Check if the load was successful
	GUARANTEE_OR_DIE( texelData, Stringf( "Failed to load image \"%s\"", imageFilePath ) );

	Texture* newTexture = CreateTextureFromData( imageFilePath, dimensions, bytesPerTexel, texelData );

	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free( texelData );

	return newTexture;
}


//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromData( char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData )
{
	// Check if the load was successful
	GUARANTEE_OR_DIE( texelData, Stringf( "CreateTextureFromData failed for \"%s\" - texelData was null!", name ) );
	GUARANTEE_OR_DIE( bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf( "CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel ) );
	GUARANTEE_OR_DIE( dimensions.x > 0 && dimensions.y > 0, Stringf( "CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y ) );

	Texture* newTexture = new Texture();
	newTexture->m_name = name; // NOTE: m_name must be a std::string, otherwise it may point to temporary data!
	newTexture->m_dimensions = dimensions;

	// Enable OpenGL texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*) &newTexture->m_textureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, newTexture->m_textureID );

	// Set texture clamp vs. wrap (repeat) default settings
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // GL_CLAMP or GL_REPEAT
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // GL_CLAMP or GL_REPEAT

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	// Pick the appropriate OpenGL format (RGB or RGBA) for this texel data
	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if( bytesPerTexel == 3 )
	{
		bufferFormat = GL_RGB;
	}
	GLenum internalFormat = bufferFormat; // the format we want the texture to be on the card; technically allows us to translate into a different texture format as we upload to OpenGL

	// Upload the image texel data (raw pixels bytes) to OpenGL under this textureID
	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		dimensions.x,		// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,11], and B is the border thickness [0,1]
		dimensions.y,		// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,11], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1, recommend 0)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color channel/component)
		texelData );		// Address of the actual pixel data bytes/buffer in system memory

	m_loadedTextures.push_back( newTexture );
	return newTexture;
}


//-----------------------------------------------------------------------------------------------
void Renderer::BindTexture( Texture* texture )
{
	if( texture )
	{
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, texture->m_openglTextureID );
	}
	else
	{
		glDisable( GL_TEXTURE_2D );
	}
}


//------------------------------------------------------------------------------------------------
class Texture
{
	friend class Renderer; // Only the Renderer can create new Texture objects!

private:
	Texture(); // can't instantiate directly; must ask Renderer to do it for you
	Texture( Texture const& copy ) = delete; // No copying allowed!  This represents GPU memory.
	~Texture();

public:
	IntVec2				GetDimensions() const		{ return m_dimensions; }
	std::string const&	GetImageFilePath() const	{ return m_name; }

protected:
	std::string			m_name;
	IntVec2				m_dimensions;

	// #ToDo: generalize/replace this for D3D11 support!
	unsigned int		m_openglTextureID				= 0xFFFFFFFF;
};
	

//-----------------------------------------------------------------------------------------------
// Rough sample code for textured rendering (i.e. in Libra game code)
//
// NOTE: this is a temporary solution; later, we will do away with untextured drawing completely
//	(and simply bind a plain white texture when we want to draw "untextured").
//
// Sample usage:
//-----------------------------------------------------------------------------------------------
void SomeGameFunction()
{
	/// ...assume we already have a tank local bounds and tint color, etc.

	// Draw tank base (textured)
	Texture* tankBaseTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TankBase.png" );
	std::vector<Vertex_PCU> tankBaseVerts;
	AppendVertsForAABB2D( tankBaseVerts, tankLocalBounds, color );
	g_theRenderer->BindTexture( tankBaseTexture );
	g_theRenderer->DrawVertexArray( (int) tankBaseVerts.size(), tankBaseVerts.data() );

	// Draw pink radius ring (untextured)
	std::vector<Vertex_PCU> ringVerts;
	AppendVertsForRing2D( ringVerts, ringCenter, ringRadius, ringThickness, Rgba8( 255, 0, 255 ) );
	g_theRenderer->BindTexture( nullptr ); // disables texturing in OpenGL (for now)
	g_theRenderer->DrawVertexArray( (int) ringVerts.size(), ringVerts.data() );
}



