#include "Engine/Input/NamedStrings.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"

void NamedStrings::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	const XmlAttribute* attribute = element.FirstAttribute();
	while (attribute)
	{
		std::string key = attribute->Name();
		std::string value = attribute->Value();
		m_keyValuePairs[key] = value;
		attribute = attribute->Next();
	}
}

void NamedStrings::SetValue(std::string const& keyName, std::string const& newValue)
{
	m_keyValuePairs[keyName] = newValue;
}

std::string NamedStrings::GetValue(std::string const& keyName, std::string const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	return found->second;
}

bool NamedStrings::GetValue(std::string const& keyName, bool defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);

	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	std::string value = found->second;

	if (value == "true")
	{
		return true;
	}
	else if (value == "false")
	{
		return false;
	}

	return defaultValue;
}

int NamedStrings::GetValue(std::string const& keyName, int defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);

	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	return std::stoi(found->second);
}

float NamedStrings::GetValue(std::string const& keyName, float defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);

	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	return std::stof(found->second);
}

std::string NamedStrings::GetValue(std::string const& keyName, char const* defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);

	if (found == m_keyValuePairs.end())
	{
		return std::string(defaultValue);
	}

	return found->second;
}

Rgba8 NamedStrings::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);

	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	Rgba8 value;
	value.SetFromText(found->second.c_str());

	return value;
}

Vec2 NamedStrings::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);

	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	Vec2 value;
	value.SetFromText(found->second.c_str());

	return value;
}

IntVec2 NamedStrings::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
{
	auto found = m_keyValuePairs.find(keyName);

	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	IntVec2 value;
	value.SetFromText(found->second.c_str());

	return value;
}






