#pragma once

struct Rgba8
{
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;
									
	static const Rgba8 RED;
	static const Rgba8 GREEN;
	static const Rgba8 BLUE;
	static const Rgba8 WHITE;
	static const Rgba8 BLACK;
	static const Rgba8 YELLOW;
	static const Rgba8 CYAN;
	static const Rgba8 MAGENTA;
	static const Rgba8 GRAY;
	static const Rgba8 ORANGE;
	static const Rgba8 PURPLE;
	static const Rgba8 BROWN;
	static const Rgba8 PINK	;
	static const Rgba8 LIGHT_GRAY;
	static const Rgba8 DARK_GRAY;
	static const Rgba8 NAVY_BLUE;



	Rgba8() = default;

	Rgba8(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
	Rgba8(unsigned char red, unsigned char green, unsigned char blue);

	static Rgba8 ScaleColor(Rgba8 color, float percent, float aFactor=1.f);

	void SetFromText(char const* text);

	void GetAsFloats(float* colorAsFloats) const;
	bool operator==(Rgba8 const& compare) const;
};

Rgba8 Interpolate(Rgba8 startColor, Rgba8 endColor, float fractionOfEnd);