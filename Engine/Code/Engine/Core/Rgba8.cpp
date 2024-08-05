#include "engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"

//--------------------------------------------------------------------------------------------------
#include <string>

//--------------------------------------------------------------------------------------------------
Rgba8 const Rgba8::WHITE(255, 255, 255);
Rgba8 const Rgba8::BLACK(0, 0, 0);
Rgba8 const Rgba8::RED(255, 0, 0);
Rgba8 const Rgba8::GREEN(0, 255, 0);
Rgba8 const Rgba8::BLUE(0, 0, 255);
Rgba8 const Rgba8::CYAN(0, 255, 255);
Rgba8 const Rgba8::MAGENTA(255, 0, 255);
Rgba8 const Rgba8::YELLOW(255, 255, 0);
Rgba8 const Rgba8::ORANGE(255, 165, 0);
Rgba8 const Rgba8::TURQUOISE(64, 224, 208);
Rgba8 const Rgba8::CRIMSON(220, 20, 60);

Rgba8 const Rgba8::DARK_CYAN(30, 175, 180);
Rgba8 const Rgba8::DARK_GREEN(0, 100, 0);
Rgba8 const Rgba8::DARK_BLUE(20, 20, 40);
Rgba8 const Rgba8::DARK_BLUE_GREY(102, 102, 153);

Rgba8 const Rgba8::LIGHT_BLUE(200, 230, 255);

Rgba8 const Rgba8::DEEP_PINK(255, 20, 147);


//--------------------------------------------------------------------------------------------------
Rgba8::Rgba8(unsigned char r, unsigned char g, unsigned char b, unsigned char a) : r(r), g(g), b(b), a(a)
{
}


//--------------------------------------------------------------------------------------------------
bool Rgba8::operator==(Rgba8 const& compare) const
{
	
	return (r == compare.r) && (g == compare.g) && (b == compare.b) && (a == compare.a);
}


//--------------------------------------------------------------------------------------------------
bool Rgba8::operator!=(Rgba8 const& compare) const
{
	return ((r != compare.r) || (g != compare.g) || (b != compare.b) || (a != compare.a));
}


//--------------------------------------------------------------------------------------------------
void Rgba8::operator*=(float strength)
{
	r = (unsigned char)(r * strength);
	g = (unsigned char)(g * strength);
	b = (unsigned char)(b * strength);
	a = (unsigned char)(a * strength);
}


//--------------------------------------------------------------------------------------------------
Rgba8 Rgba8::operator*(float strength)
{
	Rgba8 result;
	result.r = (unsigned char)(r * strength);
	result.g = (unsigned char)(g * strength);
	result.b = (unsigned char)(b * strength);
	result.a = (unsigned char)(a * strength);
	return result;
}


//--------------------------------------------------------------------------------------------------
void Rgba8::SetFromText(char const* text)
{
	Strings delimitedText = SplitStringOnDelimiter(text, ',');
	r = unsigned char(atoi(delimitedText[0].data()));
	g = unsigned char(atoi(delimitedText[1].data()));
	b = unsigned char(atoi(delimitedText[2].data()));
	if (delimitedText.size() > 3)
	{
		a = unsigned char(atoi(delimitedText[3].data()));
	}
}


//--------------------------------------------------------------------------------------------------
void Rgba8::GetAsFloats(float* colorAsFloats)const
{
	colorAsFloats[0] = NormalizeByte(r);
	colorAsFloats[1] = NormalizeByte(g);
	colorAsFloats[2] = NormalizeByte(b);
	colorAsFloats[3] = NormalizeByte(a);
}