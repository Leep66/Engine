#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/FloatRange.hpp"


int ParseXmlAttribute(XmlElement const& element, char const* attributeName, int defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		return static_cast<int> (atoi(attributeValue)); 
	}

	return defaultValue;
}

char ParseXmlAttribute(XmlElement const& element, char const* attributeName, char defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue && strlen(attributeValue) == 1)
	{
		return attributeValue[0];
	}

	return defaultValue;
}

bool ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool defaultValue)
{
	const char* value = element.Attribute(attributeName);
	if (value)
	{
		return (strcmp(value, "true") == 0);
	}
	return defaultValue;
}

float ParseXmlAttribute(XmlElement const& element, char const* attributeName, float defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		return static_cast<float> ((atof(attributeValue)));
	}

	return defaultValue;
}

Rgba8 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);

	if (attributeValue)
	{
		Strings rgbaStrings = SplitStringOnDelimiter(attributeValue, ',');

		if (rgbaStrings.size() == 3 || rgbaStrings.size() == 4)
		{
			unsigned char r = static_cast<unsigned char>(atoi(rgbaStrings[0].c_str()));
			unsigned char g = static_cast<unsigned char>(atoi(rgbaStrings[1].c_str()));
			unsigned char b = static_cast<unsigned char>(atoi(rgbaStrings[2].c_str()));
			unsigned char a = (rgbaStrings.size() == 4)
				? static_cast<unsigned char>(atoi(rgbaStrings[3].c_str()))
				: 255;

			return Rgba8(r, g, b, a);
		}
		else
		{
			printf("Invalid Rgba8 Xml Attribute: %s\n", attributeValue);
		}
	}

	return defaultValue;
}


Vec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const& defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		Strings vec2String = SplitStringOnDelimiter(attributeValue, ',');

		if (vec2String.size() != 2) {
			printf("Invalid Vec2 Xml Attribute");
		}

		float x = static_cast<float> (atof(vec2String[0].c_str()));
		float y = static_cast<float> (atof(vec2String[1].c_str()));

		return Vec2(x, y);
	}
	return defaultValue;
}

IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		Strings intVec2String = SplitStringOnDelimiter(attributeValue, ',');

		if (intVec2String.size() != 2) {
			printf("Invalid IntVec2 Xml Attribute");
		}

		int intX = static_cast<int> (atoi(intVec2String[0].c_str()));
		int intY = static_cast<int> (atoi(intVec2String[1].c_str()));

		return IntVec2(intX, intY);
	}
	
	return defaultValue;

}

Vec3 ParseXmlAttribute(const XmlElement& element, const char* attributeName, const Vec3& defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		Strings Vec3String = SplitStringOnDelimiter(attributeValue, ',');
		
		float x = static_cast<float> (atof(Vec3String[0].c_str()));
		float y = static_cast<float> (atof(Vec3String[1].c_str()));
		float z = static_cast<float> (atof(Vec3String[2].c_str()));

		return Vec3(x, y, z);
	}

	return defaultValue;
}

EulerAngles ParseXmlAttribute(const XmlElement& element, const char* attributeName, const EulerAngles& defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		Strings eulerString = SplitStringOnDelimiter(attributeValue, ',');

		float yaw = static_cast<float> (atof(eulerString[0].c_str()));
		float pitch = static_cast<float> (atof(eulerString[1].c_str()));
		float roll = static_cast<float> (atof(eulerString[2].c_str()));

		return EulerAngles(yaw, pitch, roll);
	}

	return defaultValue;
}

FloatRange ParseXmlAttribute(const XmlElement& element, const char* attributeName, const FloatRange& defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);
	if (attributeValue)
	{
		Strings floatRangeString = SplitStringOnDelimiter(attributeValue, '~');

		float min = static_cast<float> (atof(floatRangeString[0].c_str()));
		float max = static_cast<float> (atof(floatRangeString[1].c_str()));


		return FloatRange(min, max);
	}

	return defaultValue;
}

std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);

	if (attributeValue) {
		return std::string(attributeValue);
	}

	return defaultValue;
}

std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, char const* defaultValue)
{
	const char* attributeValue = element.Attribute(attributeName);

	if (attributeValue) {
		return std::string(attributeValue); 
	}

	return std::string(defaultValue);
}

Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValues)
{
	const char* attributeValue = element.Attribute(attributeName);

	if (attributeValue) {
		return SplitStringOnDelimiter(attributeValue, ',');
	}

	return defaultValues;
}
