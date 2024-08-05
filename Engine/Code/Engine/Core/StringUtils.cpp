#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


//--------------------------------------------------------------------------------------------------
Strings SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn)
{
	Strings result;

	size_t startPos = 0;
	size_t delimiterPos = originalString.find_first_of(delimiterToSplitOn, startPos);
	while (delimiterPos != std::string::npos)
	{
		std::string substring = std::string(originalString, startPos, delimiterPos - startPos);
		result.push_back(substring);
		startPos = delimiterPos + 1;
		delimiterPos = originalString.find_first_of(delimiterToSplitOn, startPos);
	}

	std::string substring = std::string(originalString, startPos);
	result.push_back(substring);
	return result;
}


//--------------------------------------------------------------------------------------------------
Strings SplitStringOnDelimiterWithQuotes(std::string const& originalString, char delimiterToSplitOn)
{
	Strings result;

	bool isInQuotes = false;
	size_t delimiterPos = 0;
	for (size_t stringIndex = 0; stringIndex < originalString.size(); stringIndex++)
	{
		if (originalString[stringIndex] == '"')
		{
			isInQuotes = !isInQuotes;
		}

		if (originalString[stringIndex] == delimiterToSplitOn)
		{
			if (isInQuotes)
			{
				continue;
			}

			std::string subString = std::string(originalString, delimiterPos, stringIndex - delimiterPos);
			result.push_back(subString);
			delimiterPos = stringIndex + 1;
		}
	}

	std::string subString = std::string(originalString, delimiterPos);
	result.push_back(subString);
	return result;
}


//--------------------------------------------------------------------------------------------------
void TrimString(std::string& originalString, char delimiterToTrim)
{
	// std::string trimmedString;
	// for (int stringIndex = 0; stringIndex < (int)originalString.size(); ++stringIndex)
	// {
	// 	if (originalString[stringIndex] == delimiterToTrim)
	// 	{
	// 		continue;
	// 	}
	// 	trimmedString += originalString[stringIndex];
	// }
	// originalString = trimmedString;

	size_t stringSize = originalString.size();
	if (stringSize == 0)
	{
		return;
	}

	for (size_t stringIndex = 0; stringIndex < stringSize; ++stringIndex)
	{
		if (originalString[stringIndex] == delimiterToTrim)
		{
			std::string firstHalfOfString	= std::string(originalString, 0, stringIndex);

			if (stringIndex != stringSize - 1)
			{
				std::string secondHalfOfString	= std::string(originalString, stringIndex + 1, stringSize);
				originalString					= firstHalfOfString + secondHalfOfString;
				break;
			}
			else
			{
				originalString = firstHalfOfString;
				return;
			}
		}
	}

	stringSize = originalString.size();
	if (stringSize == 0)
	{
		return;
	}

	for (size_t stringIndex = stringSize - 1; stringIndex > 0; --stringIndex)
	{
		if (originalString[stringIndex] == delimiterToTrim)
		{
			std::string firstHalfOfString = std::string(originalString, 0, stringIndex);

			if (stringIndex != stringSize - 1)
			{
				std::string secondHalfOfString	= std::string(originalString, stringIndex + 1, stringSize);
				originalString					= firstHalfOfString + secondHalfOfString;
				break;
			}
			else
			{
				originalString = firstHalfOfString;
				return;
			}
		}
		

		// Should never happen in theory
		if (stringIndex == 1)
		{
			if (originalString[0] == delimiterToTrim)
			{
				originalString = std::string(originalString, 1, stringSize - 1);
				break;
			}
		}
	}
}





