#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/XmlUtils.hpp"


//--------------------------------------------------------------------------------------------------
int ParseXmlAttribute(XmlElement const& element, char const* attributeName, int defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	int attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		attributeActualValue = int(atoi(attributeTextValue));
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
char ParseXmlAttribute(XmlElement const& element, char const* attributeName, char defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	char attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		attributeActualValue = attributeTextValue[0];
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
bool ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	bool attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		if (attributeTextValue[0] == 't' || attributeTextValue[0] == 'T')
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
float ParseXmlAttribute(XmlElement const& element, char const* attributeName, float defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	float attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		attributeActualValue = float(atof(attributeTextValue));
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
Rgba8 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	Rgba8 attributeActualValue;
	if (!attributeTextValue)
	{
		attributeActualValue = defaultValue;
	}
	else
	{
		attributeActualValue.SetFromText(attributeTextValue);
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
Vec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	Vec2 attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		attributeActualValue.SetFromText(attributeTextValue);
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
Vec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec3 const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	Vec3 attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		attributeActualValue.SetFromText(attributeTextValue);
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	IntVec2 attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		attributeActualValue.SetFromText(attributeTextValue);
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
FloatRange ParseXmlAttribute(XmlElement const& element, char const* attributeName, FloatRange const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	FloatRange attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		attributeActualValue.SetFromText(attributeTextValue);
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	std::string attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		attributeActualValue = attributeTextValue;
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	Strings attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		attributeActualValue = SplitStringOnDelimiter(attributeTextValue, ',');
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, char const* defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	std::string attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		attributeActualValue = attributeTextValue;
	}
	return attributeActualValue;
}


//--------------------------------------------------------------------------------------------------
EulerAngles ParseXmlAttribute(XmlElement const& element, char const* attributeName, EulerAngles const& defaultValue)
{
	char const* attributeTextValue = element.Attribute(attributeName);
	EulerAngles attributeActualValue = defaultValue;
	if (attributeTextValue)
	{
		attributeActualValue.SetFromText(attributeTextValue);
	}
	return attributeActualValue;
}
