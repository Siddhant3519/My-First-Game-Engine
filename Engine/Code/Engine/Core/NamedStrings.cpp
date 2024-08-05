#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"

void NamedStrings::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	XmlAttribute const* currentAttribute = element.FirstAttribute();

	while (currentAttribute != nullptr)
	{
		std::string const currentKeyName =	std::string(currentAttribute->Name());
		std::string const currentKeyValue = std::string(currentAttribute->Value());
		SetValue(currentKeyName, currentKeyValue);
		currentAttribute = currentAttribute->Next();
	}
}

void NamedStrings::SetValue(std::string const& keyName, std::string const& newValue)
{
	m_keyValuePairs[keyName] = newValue;
}

std::string NamedStrings::GetValue(std::string const& keyName, std::string const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iteratorElement = m_keyValuePairs.find(keyName);
	// TODO(sid): verify if this works
	if (iteratorElement != m_keyValuePairs.end())
	{
		return iteratorElement->second;
	}
	
	return defaultValue;
}

bool NamedStrings::GetValue(std::string const& keyName, bool defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iteratorElement = m_keyValuePairs.find(keyName);
	if (iteratorElement != m_keyValuePairs.end())
	{
		std::string const& textVal = iteratorElement->second;

		if (textVal[0] == 't' || textVal[0] == 'T')
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return defaultValue;
}

int NamedStrings::GetValue(std::string const& keyName, int defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iteratorElement = m_keyValuePairs.find(keyName);
	if (iteratorElement != m_keyValuePairs.end())
	{
		int value = atoi(iteratorElement->second.c_str());
		return value;
	}
	return defaultValue;
}

float NamedStrings::GetValue(std::string const& keyName, float defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iteratorElement = m_keyValuePairs.find(keyName);
	if (iteratorElement != m_keyValuePairs.end())
	{
		float value = (float)(atof(iteratorElement->second.c_str()));
		return value;
	}
	return defaultValue;
}

std::string NamedStrings::GetValue(std::string const& keyName, char const* defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iteratorElement = m_keyValuePairs.find(keyName);
	if (iteratorElement != m_keyValuePairs.end())
	{
		return iteratorElement->second;
	}
	return defaultValue;
}

Rgba8 NamedStrings::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iteratorElement = m_keyValuePairs.find(keyName);
	if (iteratorElement != m_keyValuePairs.end())
	{
		Rgba8 value;
		value.SetFromText(iteratorElement->second.c_str());
		return value;
	}
	return defaultValue;
}


//--------------------------------------------------------------------------------------------------
Vec2 NamedStrings::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iteratorElement = m_keyValuePairs.find(keyName);
	if (iteratorElement != m_keyValuePairs.end())
	{
		Vec2 value;
		value.SetFromText(iteratorElement->second.c_str());
		return value;
	}
	return defaultValue;
}


//--------------------------------------------------------------------------------------------------
Vec3 NamedStrings::GetValue(std::string const& keyName, Vec3 const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iteratorElement = m_keyValuePairs.find(keyName);
	if (iteratorElement != m_keyValuePairs.end())
	{
		Vec3 value;
		value.SetFromText(iteratorElement->second.c_str());
		return value;
	}
	return defaultValue;
}


//--------------------------------------------------------------------------------------------------
IntVec2 NamedStrings::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iteratorElement = m_keyValuePairs.find(keyName);
	if (iteratorElement != m_keyValuePairs.end())
	{
		IntVec2 value;
		value.SetFromText(iteratorElement->second.c_str());
		return value;
	}
	return defaultValue;
}


//--------------------------------------------------------------------------------------------------
EulerAngles NamedStrings::GetValue(std::string const& keyName, EulerAngles const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator iteratorElement = m_keyValuePairs.find(keyName);
	if (iteratorElement != m_keyValuePairs.end())
	{
		EulerAngles value;
		value.SetFromText(iteratorElement->second.c_str());
		return value;
	}
	return defaultValue;
}
