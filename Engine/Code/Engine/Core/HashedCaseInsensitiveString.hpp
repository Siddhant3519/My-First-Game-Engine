#pragma once


//--------------------------------------------------------------------------------------------------
#include <string>


//--------------------------------------------------------------------------------------------------
class HashedCaseInsensitiveString
{
public:
	HashedCaseInsensitiveString()												=	default;
	HashedCaseInsensitiveString( HashedCaseInsensitiveString const& copyFrom )	=	default;
	HashedCaseInsensitiveString( char const* originalText );
	HashedCaseInsensitiveString( std::string const& originalText );

	unsigned int		GetHash()				const;
	std::string const&	GetOriginalString()		const;
	char const*			c_str()					const;

	static unsigned int CalculateHashForText(char const* text);
	static unsigned int CalculateHashForText(std::string const& text);

	bool operator<(HashedCaseInsensitiveString const& compareHCIS)	const;
	bool operator>(HashedCaseInsensitiveString const& compareHCIS)	const;
	bool operator==(HashedCaseInsensitiveString const& compareHCIS) const;
	bool operator!=(HashedCaseInsensitiveString const& compareHCIS) const;
	bool operator==(std::string const& compareText)					const;
	bool operator!=(std::string const& compareText)					const;
	bool operator==(char const* compareText)						const;
	bool operator!=(char const* compareText)						const;
	void operator=(HashedCaseInsensitiveString const& assignFrom);
	void operator=(std::string const& text);
	void operator=(char const* text);

private:
	std::string		m_caseIntactText	=	"";
	unsigned int	m_lowerCaseHash		=	0;
};

typedef HashedCaseInsensitiveString HCISString;