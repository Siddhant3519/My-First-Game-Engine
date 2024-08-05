#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.hpp"
#include <string>
#include <vector>


//--------------------------------------------------------------------------------------------------
int		FileReadToBuffer(std::vector<uint8_t>& out_Buffer, std::string const& fileName);
int		FileReadToString(std::string& outString, std::string const& fileName);
void	FileWriteFromBuffer(std::vector<uint8_t> const& inBuffer, std::string const& fileName);
bool	DoesFileExist(std::string const& filePath);