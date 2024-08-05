#pragma once

//--------------------------------------------------------------------------------------------------
struct Rgba8
{
public:
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;

	static Rgba8 const WHITE;
	static Rgba8 const BLACK;
	static Rgba8 const RED;
	static Rgba8 const GREEN;
	static Rgba8 const BLUE;
	static Rgba8 const CYAN;
	static Rgba8 const MAGENTA;
	static Rgba8 const YELLOW;
	static Rgba8 const ORANGE;
	static Rgba8 const TURQUOISE;
	static Rgba8 const CRIMSON;

	static Rgba8 const DARK_CYAN;
	static Rgba8 const DARK_GREEN;
	static Rgba8 const DARK_BLUE;
	static Rgba8 const DARK_BLUE_GREY;

	static Rgba8 const LIGHT_BLUE;

	static Rgba8 const DEEP_PINK;

public:
	Rgba8() {}

	explicit Rgba8(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	bool		operator==(Rgba8 const& compare) const;
	bool		operator!=(Rgba8 const& compare) const;
	void		operator*=(float strength);
	Rgba8		operator*(float strength);
	void SetFromText(char const* text);
	void GetAsFloats(float* colorAsFloats)const;
};