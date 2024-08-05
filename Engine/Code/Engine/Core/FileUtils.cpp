#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//--------------------------------------------------------------------------------------------------
int FileReadToBuffer(std::vector<uint8_t>& out_Buffer, std::string const& fileName)
{
	size_t numOfItemsRead = 0;
	FILE* fileStreamPtr = nullptr;
	errno_t fOpenErr = fopen_s(&fileStreamPtr, fileName.c_str(), "rb");
	if (fOpenErr)
	{
		ERROR_AND_DIE("Could not open the specified file" + fOpenErr);
	}

	if (fileStreamPtr)
	{
		fseek(fileStreamPtr, 0, SEEK_END);
		size_t fileSize = ftell(fileStreamPtr);
		fseek(fileStreamPtr, 0, SEEK_SET);
		out_Buffer.resize(fileSize);
		numOfItemsRead = fread(out_Buffer.data(), sizeof(uint8_t), fileSize, fileStreamPtr);
		if (numOfItemsRead != fileSize)
		{
			ERROR_AND_DIE("There was a problem in reading parsing the file" + fileName);
		}
		fclose(fileStreamPtr);
	}


	return (int)numOfItemsRead;
}


//--------------------------------------------------------------------------------------------------
int FileReadToString(std::string& outString, std::string const& fileName)
{
	std::vector<uint8_t> outputBuffer;
	int numOfItemsRead = FileReadToBuffer(outputBuffer, fileName);
	outputBuffer.push_back('\0');
	outString.resize(outputBuffer.size());
	
	memcpy(outString.data(), outputBuffer.data(), outputBuffer.size());
	if ((int)outString.size() != (numOfItemsRead + 1))
	{
		ERROR_AND_DIE("Something went wrong while parsing the file " + fileName);
	}

	return (int)outString.size();
}


//--------------------------------------------------------------------------------------------------
#include <filesystem>
void FileWriteFromBuffer(std::vector<uint8_t> const& inBuffer, std::string const& fileName)
{
	FILE* fileInfoPtr = nullptr;
	std::filesystem::path filePath(fileName);
	filePath.remove_filename();
	if (!std::filesystem::exists(filePath))
	{
		std::filesystem::create_directory(filePath);
	}

	errno_t fOpenErr = fopen_s(&fileInfoPtr, fileName.c_str(), "wb");
	if (fOpenErr)
	{
		ERROR_AND_DIE("Could not open the specified file" + fOpenErr);
	}

	size_t bytesWritten = fwrite(inBuffer.data(), sizeof(uint8_t), inBuffer.size(), fileInfoPtr);
	fclose(fileInfoPtr);
	(void)bytesWritten;
}


//--------------------------------------------------------------------------------------------------
bool DoesFileExist(std::string const& filePath)
{
	FILE* fileInfoPtr = nullptr;
	errno_t fOpenErr = fopen_s(&fileInfoPtr, filePath.c_str(), "rb");
	if (fOpenErr)
	{
		return false;
	}
	fclose(fileInfoPtr);
	return true;
}