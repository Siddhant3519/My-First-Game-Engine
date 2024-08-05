#include "Engine/Core/HashedCaseInsensitiveString.hpp"


//--------------------------------------------------------------------------------------------------
HashedCaseInsensitiveString::HashedCaseInsensitiveString(char const* originalText) :
	m_caseIntactText(originalText),
	m_lowerCaseHash(CalculateHashForText(originalText))
{

}


//--------------------------------------------------------------------------------------------------
HashedCaseInsensitiveString::HashedCaseInsensitiveString(std::string const& originalText) :
	m_caseIntactText(originalText),
	m_lowerCaseHash(CalculateHashForText(originalText))
{

}


//--------------------------------------------------------------------------------------------------
unsigned int HashedCaseInsensitiveString::GetHash() const
{
	return m_lowerCaseHash;
}


//--------------------------------------------------------------------------------------------------
std::string const& HashedCaseInsensitiveString::GetOriginalString() const
{
	return m_caseIntactText;
}


//--------------------------------------------------------------------------------------------------
char const* HashedCaseInsensitiveString::c_str() const
{
	return m_caseIntactText.c_str();
}


//--------------------------------------------------------------------------------------------------
unsigned int HashedCaseInsensitiveString::CalculateHashForText(char const* text)
{
	unsigned int hash		=	0;
	char const* readPos		=	text;
	while (*readPos != '\0')
	{
		hash *= 31;
		hash += (unsigned int)tolower(*readPos);
		readPos++;
	}

	return hash;
}


//--------------------------------------------------------------------------------------------------
unsigned int HashedCaseInsensitiveString::CalculateHashForText(std::string const& text)
{
	return CalculateHashForText(text.c_str());
}


//--------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator<(HashedCaseInsensitiveString const& compareHCIS) const
{
	if (m_lowerCaseHash < compareHCIS.m_lowerCaseHash)
	{
		return true;
	}
	else if (m_lowerCaseHash > compareHCIS.m_lowerCaseHash)
	{
		return false;
	}
	else
	{
		return _stricmp(m_caseIntactText.c_str(), compareHCIS.m_caseIntactText.c_str()) < 0;
	}
}


//--------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator>(HashedCaseInsensitiveString const& compareHCIS) const
{
	if (m_lowerCaseHash > compareHCIS.m_lowerCaseHash)
	{
		return true;
	}
	else if (m_lowerCaseHash < compareHCIS.m_lowerCaseHash)
	{
		return false;
	}
	else
	{
		return _stricmp(m_caseIntactText.c_str(), compareHCIS.m_caseIntactText.c_str()) > 0;
	}
}


//--------------------------------------------------------------------------------------------------
void HashedCaseInsensitiveString::operator=(char const* text)
{
	m_caseIntactText	=	text;
	m_lowerCaseHash		=	CalculateHashForText(m_caseIntactText);
}


//--------------------------------------------------------------------------------------------------
void HashedCaseInsensitiveString::operator=(std::string const& text)
{
	m_caseIntactText	=	text;
	m_lowerCaseHash		=	CalculateHashForText(m_caseIntactText);
}


//--------------------------------------------------------------------------------------------------
void HashedCaseInsensitiveString::operator=(HashedCaseInsensitiveString const& assignFrom)
{
	m_caseIntactText	=	assignFrom.m_caseIntactText;
	m_lowerCaseHash		=	assignFrom.m_lowerCaseHash;
}


//--------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator!=(char const* compareText) const
{
	return _stricmp(m_caseIntactText.c_str(), compareText) != 0;
}


//--------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator==(char const* compareText) const
{
	return _stricmp(m_caseIntactText.c_str(), compareText) == 0;
}


//--------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator!=(std::string const& compareText) const
{
	return _stricmp(m_caseIntactText.c_str(), compareText.c_str()) != 0;
}


//--------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator==(std::string const& compareText) const
{
	return _stricmp(m_caseIntactText.c_str(), compareText.c_str()) == 0;
}


//--------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator!=(HashedCaseInsensitiveString const& compareHCIS) const
{
	if (m_lowerCaseHash != compareHCIS.m_lowerCaseHash)
	{
		return true;
	}
	return _stricmp(m_caseIntactText.c_str(), compareHCIS.m_caseIntactText.c_str()) != 0;
}


//--------------------------------------------------------------------------------------------------
bool HashedCaseInsensitiveString::operator==(HashedCaseInsensitiveString const& compareHCIS) const
{
	if (m_lowerCaseHash != compareHCIS.m_lowerCaseHash)
	{
		return false;
	}
	else
	{
		return _stricmp(m_caseIntactText.c_str(), compareHCIS.m_caseIntactText.c_str()) == 0;
	}
}
