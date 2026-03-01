#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>
#include <string>
#include <vector>


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

Strings SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn)
{
	Strings splitStrings;
	std::string currentString;

	for (char character : originalString)
	{
		if (character == delimiterToSplitOn)
		{
			if (!currentString.empty())
			{
				splitStrings.push_back(currentString);
				currentString.clear();
			}
		}
		else
		{
			currentString += character;
		}
	}

	if (!currentString.empty())
	{
		splitStrings.push_back(currentString);
	}

	return splitStrings;
}

std::string ToLower(const std::string& str) 
{
	std::string result = str;
	for (char& c : result) {
		c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
	}
	return result;
}

std::string CleanupString(const std::string& input)
{
	std::string output;
	for (char c : input) {
		if (c != '\n' && c != '\r') {
			output.push_back(c);
		}
	}
	return output;
}


