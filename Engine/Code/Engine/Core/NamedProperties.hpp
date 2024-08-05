#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/HashedCaseInsensitiveString.hpp"


//--------------------------------------------------------------------------------------------------
#include <map>
#include <string>


//--------------------------------------------------------------------------------------------------
class NamedPropertyBase
{
public:
	NamedPropertyBase()				=	default;
	virtual ~NamedPropertyBase()	=	default;

protected:

};


//--------------------------------------------------------------------------------------------------
template<typename T>
class NamedPropertyofType : public NamedPropertyBase
{
	friend class NamedProperties;

protected:
	NamedPropertyofType(T value) : m_value(value) {}
	T m_value;
};


//--------------------------------------------------------------------------------------------------
class NamedProperties
{
public:
	template<typename T>
	void SetValue(std::string const& keyName, T const& value);
	template<typename T>
	T GetValue(std::string const& keyName, T const& defaultValue);
	void SetValue(std::string const& keyName, char const* value);
	std::string GetValue(std::string const& keyName, char const* defaultValue);

	template<typename T>
	void SetValue(char const* keyName, T const& value);
	template<typename T>
	T			GetValue(char const* keyName, T const& defaultName);
	void		SetValue(char const* keyName, char const* defaultValue);
	std::string GetValue(char const* keyName, char const* defaultValue);

private:
	std::map<HCISString, NamedPropertyBase*> m_keyValuePairs;
};


//--------------------------------------------------------------------------------------------------
template<typename T>
inline void NamedProperties::SetValue(std::string const& keyName, T const& value)
{
	HCISString key(keyName);
	m_keyValuePairs[key] = new NamedPropertyofType<T>(value);
}


//--------------------------------------------------------------------------------------------------
template<typename T>
inline T NamedProperties::GetValue(std::string const& keyName, T const& defaultValue)
{
	HCISString key(keyName);
	std::map<HCISString, NamedPropertyBase*>::iterator found = m_keyValuePairs.find(key);
	if (found == m_keyValuePairs.end());
	{
		return defaultValue;
	}

	NamedPropertyBase* property = found->second;
	NamedPropertyofType<T>* typedProperty = dynamic_cast<NamedPropertyofType<T>*>(property);
	if (typedProperty == nullptr)
	{
		ERROR_RECOVERABLE("Asked for value of the incorrect type!");
		return defaultValue;
	}

	return typedProperty->m_value;
}


//--------------------------------------------------------------------------------------------------
void NamedProperties::SetValue(std::string const& keyName, char const* value)
{
	HCISString key(keyName);
	m_keyValuePairs[key] = new NamedPropertyofType<std::string>(value);
}


//--------------------------------------------------------------------------------------------------
inline std::string NamedProperties::GetValue(std::string const& keyName, char const* defaultValue)
{
	HCISString key(keyName);
	std::map<HCISString, NamedPropertyBase*>::iterator found = m_keyValuePairs.find(key);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	NamedPropertyBase* property = found->second;
	NamedPropertyofType<std::string>* typedProperty = dynamic_cast<NamedPropertyofType<std::string>*>(property);
	if (typedProperty == nullptr)
	{
		ERROR_RECOVERABLE("Asked for value of the incorrect type");
		return defaultValue;
	}
	return typedProperty->m_value;
}


//--------------------------------------------------------------------------------------------------
template<typename T>
inline void NamedProperties::SetValue(char const* keyName, T const& value)
{
	HCISString key(keyName);
	m_keyValuePairs[key] = new NamedPropertyofType<T>(value);
}


//--------------------------------------------------------------------------------------------------
template<typename T>
inline T NamedProperties::GetValue(char const* keyName, T const& defaultValue)
{
	HCISString key(keyName);
	std::map<HCISString, NamedPropertyBase*>::iterator found = m_keyValuePairs.find(key);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}

	NamedPropertyBase* property = found->second;
	NamedPropertyofType<T>* typedProperty = dynamic_cast<NamedPropertyofType<T>*>(property);
	if (typedProperty == nullptr)
	{
		ERROR_RECOVERABLE("Asked for value of the incorrect type!");
		return defaultValue;
	}

	return typedProperty->m_value;
}


//--------------------------------------------------------------------------------------------------
inline void NamedProperties::SetValue(char const* keyName, char const* defaultValue)
{
	HCISString key(keyName);
	m_keyValuePairs[key] = new NamedPropertyofType<std::string>(defaultValue);
}


//--------------------------------------------------------------------------------------------------
inline std::string NamedProperties::GetValue(char const* keyName, char const* defaultValue)
{
	HCISString key(keyName);
	std::map<HCISString, NamedPropertyBase*>::iterator found = m_keyValuePairs.find(key);
	if (found == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	NamedPropertyBase* property = found->second;
	NamedPropertyofType<std::string>* typedProperty = dynamic_cast<NamedPropertyofType<std::string>*>(property);
	if (typedProperty == nullptr)
	{
		ERROR_RECOVERABLE("Asked for value of the incorrect type");
		return defaultValue;
	}
	return typedProperty->m_value;
}
