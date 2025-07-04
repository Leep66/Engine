#include "Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

const Rgba8 Rgba8::RED = Rgba8(255, 0, 0, 255);
const Rgba8 Rgba8::GREEN = Rgba8(0, 255, 0, 255);
const Rgba8 Rgba8::BLUE = Rgba8(0, 0, 255, 255);
const Rgba8 Rgba8::WHITE = Rgba8(255, 255, 255, 255);
const Rgba8 Rgba8::BLACK = Rgba8(0, 0, 0, 255);
const Rgba8 Rgba8::YELLOW = Rgba8(255, 255, 0, 255);
const Rgba8 Rgba8::CYAN = Rgba8(0, 255, 255, 255);
const Rgba8 Rgba8::MAGENTA = Rgba8(255, 0, 255, 255);
const Rgba8 Rgba8::GRAY = Rgba8(128, 128, 128, 255);
const Rgba8 Rgba8::ORANGE = Rgba8(255, 165, 0, 255);
const Rgba8 Rgba8::PURPLE = Rgba8(128, 0, 128, 255);
const Rgba8 Rgba8::BROWN = Rgba8(165, 42, 42, 255);
const Rgba8 Rgba8::PINK = Rgba8(255, 192, 203, 255);
const Rgba8 Rgba8::LIGHT_GRAY = Rgba8(211, 211, 211, 255);
const Rgba8 Rgba8::DARK_GRAY = Rgba8(64, 64, 64, 255);
const Rgba8 Rgba8::NAVY_BLUE = Rgba8(0, 127, 127, 255);
Rgba8::Rgba8(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	r = red;
	g = green;
	b = blue;
	a = alpha;
}

Rgba8::Rgba8(unsigned char red, unsigned char green, unsigned char blue)
{
	r = red;
	g = green;
	b = blue;
	a = 255;
}

Rgba8 Rgba8::ScaleColor(Rgba8 color, float rgbFactor, float aFactor)
{
	return Rgba8(
		(unsigned char)GetClamped((float)color.r * rgbFactor, 0.f, 255.f),
		(unsigned char)GetClamped((float)color.g * rgbFactor, 0.f, 255.f),
		(unsigned char)GetClamped((float)color.b * rgbFactor, 0.f, 255.f),
		(unsigned char)GetClamped((float)color.a * aFactor, 0.f, 255.f)
	);
}

Rgba8 Interpolate(Rgba8 startColor, Rgba8 endColor, float fractionOfEnd)
{
	Rgba8 blendedColor;
	float red = Interpolate(NormalizeByte(startColor.r), NormalizeByte(endColor.r), fractionOfEnd);
	float green = Interpolate(NormalizeByte(startColor.g), NormalizeByte(endColor.g), fractionOfEnd);
	float blue = Interpolate(NormalizeByte(startColor.b), NormalizeByte(endColor.b), fractionOfEnd);
	float alpha = Interpolate(NormalizeByte(startColor.a), NormalizeByte(endColor.a), fractionOfEnd);

	blendedColor.r = DenormalizeByte(red);
	blendedColor.g = DenormalizeByte(green);
	blendedColor.b = DenormalizeByte(blue);
	blendedColor.a = DenormalizeByte(alpha);

	return blendedColor;
}

void Rgba8::SetFromText(char const* text)
{
	if (!text) return;

	Strings parts = SplitStringOnDelimiter(text, ',');
	size_t count = parts.size();

	if (count < 3 || count > 4)
	{
		ERROR_AND_DIE(Stringf("Invalid Rgba8 Text"));
	}

	r = static_cast<unsigned char>(atoi(parts[0].c_str()));
	g = static_cast<unsigned char>(atoi(parts[1].c_str()));
	b = static_cast<unsigned char>(atoi(parts[2].c_str()));
	a = (count == 4) ? static_cast<unsigned char>(atoi(parts[3].c_str())) : 255; // Default alpha to 255
}

void Rgba8::GetAsFloats(float* colorAsFloats) const
{
	colorAsFloats[0] = static_cast<float>(r) / 255.0f;
	colorAsFloats[1] = static_cast<float>(g) / 255.0f;
	colorAsFloats[2] = static_cast<float>(b) / 255.0f;
	colorAsFloats[3] = static_cast<float>(a) / 255.0f;
}

bool Rgba8::operator==(Rgba8 const& compare) const
{
	return (this->r == compare.r)
		&& (this->g == compare.g)
		&& (this->b == compare.b)
		&& (this->a == compare.a);
}
