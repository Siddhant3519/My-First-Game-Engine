#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/BufferUtils.hpp"


//--------------------------------------------------------------------------------------------------
BufferWriter::BufferWriter(Buffer& buffer, eBufferEndian endianMode /*= eBufferEndian::NATIVE*/) : 
	m_bufferToWriteTo(buffer)
{
	SetEndianMode(endianMode);
}


//--------------------------------------------------------------------------------------------------
eBufferEndian BufferWriter::GetNativeEndianness() const
{
	int number = 0x12345678;
	unsigned char* firstByte = reinterpret_cast<unsigned char*>(&number);
	if (*firstByte == 0x12)
	{
		return eBufferEndian::BIG;
	}
	else
	{
		return eBufferEndian::LITTLE;
	}
}


//--------------------------------------------------------------------------------------------------
eBufferEndian BufferWriter::GetCurrentEndianness() const
{
	return m_currentEndianness;
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::SetEndianMode(eBufferEndian endianMode)
{
	eBufferEndian nativeEndianness = GetNativeEndianness();
	if (endianMode == eBufferEndian::NATIVE)
	{
		return;
	}
	m_currentEndianness			=	endianMode;
	m_isOppositeNativeEndian	=	(nativeEndianness != endianMode);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendByte(uint8_t byteToAppend)
{
	m_bufferToWriteTo.emplace_back(byteToAppend);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendChar(int8_t dataToAppend)
{
	m_bufferToWriteTo.emplace_back((uint8_t)dataToAppend);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendBool(bool booleanToAppend)
{
	AppendByte((uint8_t)booleanToAppend);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendUShort16(uint16_t wordToAppend)
{
	uint8_t* memAddr = reinterpret_cast<uint8_t*>(&wordToAppend);
	if (m_isOppositeNativeEndian)
	{
		Reverse2BytesInPlace(memAddr);
	}
	AppendByte(memAddr[0]);
	AppendByte(memAddr[1]);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendShort16(int16_t wordToAppend)
{
	uint8_t* memAddr = reinterpret_cast<uint8_t*>(&wordToAppend);
	if (m_isOppositeNativeEndian)
	{
		Reverse2BytesInPlace(memAddr);
	}
	AppendChar(memAddr[0]);
	AppendChar(memAddr[1]);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendUInt32(uint32_t dwordToAppend)
{
	uint8_t* memAddr = reinterpret_cast<uint8_t*>(&dwordToAppend);
	if (m_isOppositeNativeEndian)
	{
		Reverse4BytesInPlace(memAddr);
	}
	AppendByte(memAddr[0]);
	AppendByte(memAddr[1]);
	AppendByte(memAddr[2]);
	AppendByte(memAddr[3]);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendInt32(int32_t dwordToAppend)
{
	uint8_t* memAddr = reinterpret_cast<uint8_t*>(&dwordToAppend);
	if (m_isOppositeNativeEndian)
	{
		Reverse4BytesInPlace(memAddr);
	}
	AppendChar(memAddr[0]);
	AppendChar(memAddr[1]);
	AppendChar(memAddr[2]);
	AppendChar(memAddr[3]);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendUInt64(uint64_t qwordToAppend)
{
	uint8_t* memAddr = reinterpret_cast<uint8_t*>(&qwordToAppend);
	if (m_isOppositeNativeEndian)
	{
		Reverse8BytesInPlace(memAddr);
	}
	AppendByte(memAddr[0]);
	AppendByte(memAddr[1]);
	AppendByte(memAddr[2]);
	AppendByte(memAddr[3]);
	AppendByte(memAddr[4]);
	AppendByte(memAddr[5]);
	AppendByte(memAddr[6]);
	AppendByte(memAddr[7]);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendInt64(int64_t qwordToAppend)
{
	uint8_t* memAddr = reinterpret_cast<uint8_t*>(&qwordToAppend);
	if (m_isOppositeNativeEndian)
	{
		Reverse8BytesInPlace(memAddr);
	}
	AppendChar(memAddr[0]);
	AppendChar(memAddr[1]);
	AppendChar(memAddr[2]);
	AppendChar(memAddr[3]);
	AppendChar(memAddr[4]);
	AppendChar(memAddr[5]);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendFloat(float floatToAppend)
{
	float* floatMemAddr			=	&floatToAppend;
	uint32_t* uint32MemAddr		=	reinterpret_cast<uint32_t*>(floatMemAddr);
	AppendUInt32(*uint32MemAddr);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendDouble(double doubleToAppend)
{
	double* doubleMemAddr = &doubleToAppend;
	uint64_t* uint64MemAddr = reinterpret_cast<uint64_t*>(doubleMemAddr);
	AppendUInt64(*uint64MemAddr);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendStringZeroTerminated(char const* stringToAppend)
{
	size_t charIndex = 0;
	while (true)
	{
		char charToAppend = stringToAppend[charIndex++];
		AppendChar(charToAppend);
		if (charToAppend == '\0')
		{
			return;
		}
	}
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendStringAfter32BitLength(/*int8_t*/ char const* stringToAppend)
{
	// int length = strlen(stringToAppend);
	// AppendUInt32(length);
	// for (int i = 0; i < length; ++i)
	// {
	// 	AppendChar(stringToAppend[i]);
	// }

	size_t bufferIndexBeforeAppendingString = m_bufferToWriteTo.size() - 1;
	AppendUInt32(0);
	uint32_t charIndex = 0;
	while (true)
	{
		char charToAppend = stringToAppend[charIndex++];
		if (charToAppend == '\0')
		{
			OverwriteUint32(bufferIndexBeforeAppendingString + 1, --charIndex);
			// uint8_t* memAddr = reinterpret_cast<uint8_t*>(&charIndex);
			// if (m_isOppositeNativeEndian)
			// {
			// 	Reverse4BytesInPlace(memAddr);
			// }
			// m_bufferToWriteTo[bufferIndexBeforeAppendingString + 1] = memAddr[0];
			// m_bufferToWriteTo[bufferIndexBeforeAppendingString + 2] = memAddr[1];
			// m_bufferToWriteTo[bufferIndexBeforeAppendingString + 3] = memAddr[2];
			// m_bufferToWriteTo[bufferIndexBeforeAppendingString + 4] = memAddr[3];
			return;
		}
		AppendChar(charToAppend);
	}
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendVec2(Vec2 vec2ToAppend)
{
	AppendFloat(vec2ToAppend.x);
	AppendFloat(vec2ToAppend.y);
}

void BufferWriter::AppendVec3(Vec3 const& vec3ToAppend)
{
	AppendFloat(vec3ToAppend.x);
	AppendFloat(vec3ToAppend.y);
	AppendFloat(vec3ToAppend.z);
}

void BufferWriter::AppendVec4(Vec4 const& vec4ToAppend)
{
	AppendFloat(vec4ToAppend.x);
	AppendFloat(vec4ToAppend.y);
	AppendFloat(vec4ToAppend.z);
	AppendFloat(vec4ToAppend.w);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendIntVec2(IntVec2 intVec2ToAppend)
{
	AppendInt32(intVec2ToAppend.x);
	AppendInt32(intVec2ToAppend.y);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendIntVec3(IntVec3 intVec2ToAppend)
{
	AppendInt32(intVec2ToAppend.x);
	AppendInt32(intVec2ToAppend.y);
	AppendInt32(intVec2ToAppend.z);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendRgba(Rgba8 rgbaToAppend)
{
	AppendByte(rgbaToAppend.r);
	AppendByte(rgbaToAppend.g);
	AppendByte(rgbaToAppend.b);
	AppendByte(rgbaToAppend.a);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendRgb(Rgba8 rgbToAppend)
{
	AppendByte(rgbToAppend.r);
	AppendByte(rgbToAppend.g);
	AppendByte(rgbToAppend.b);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendAABB2(AABB2 const& aabbToAppend)
{
	AppendVec2(aabbToAppend.m_mins);
	AppendVec2(aabbToAppend.m_maxs);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendAABB3(AABB3 const& aabbToAppend)
{
	AppendVec3(aabbToAppend.m_mins);
	AppendVec3(aabbToAppend.m_maxs);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendOBB2(OBB2 const& obbToAppend)
{
	AppendVec2(obbToAppend.m_center);
	AppendVec2(obbToAppend.m_iBasisNormal);
	AppendVec2(obbToAppend.m_halfDimensions);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendOBB3(OBB3 const& obbToAppend)
{
	AppendVec3(obbToAppend.m_center);
	AppendVec3(obbToAppend.m_iBasis);
	AppendVec3(obbToAppend.m_jBasis);
	AppendVec3(obbToAppend.m_kBasis);
	AppendVec3(obbToAppend.m_halfDims);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendPlane2D(Plane2D const& planeToAppend)
{
	AppendVec2(planeToAppend.m_normal);
	AppendFloat(planeToAppend.m_distFromOrigin);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendVertexPCU(Vertex_PCU const& vertexToAppend)
{
	AppendVec3(vertexToAppend.m_position);
	AppendRgba(vertexToAppend.m_color);
	AppendVec2(vertexToAppend.m_uvTexCoords);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::AppendVertexPCUTBN(Vertex_PCUTBN const& vertexToAppend)
{
	AppendVec3(vertexToAppend.m_position);
	AppendRgba(vertexToAppend.m_color);
	AppendVec2(vertexToAppend.m_uvTexCoords);
	AppendVec3(vertexToAppend.m_tangent);
	AppendVec3(vertexToAppend.m_binormal);
	AppendVec3(vertexToAppend.m_normal);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::OverwriteUint32(uint64_t writePosOffset, uint32_t overwrittingDWord)
{
	GUARANTEE_OR_DIE((m_bufferToWriteTo.size() > writePosOffset), "Write Position Offset out of bounds");
	uint8_t* bufferStartAddress		=	m_bufferToWriteTo.data();
	uint32_t* bufferWriteAddress	=	reinterpret_cast<uint32_t*>(bufferStartAddress + writePosOffset);
	*bufferWriteAddress = overwrittingDWord;
	if (m_isOppositeNativeEndian)
	{
		Reverse4BytesInPlace(bufferWriteAddress);
	}
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::Reverse2BytesInPlace(void* bytesToReverseStartAddr)
{
	uint16_t originalWordData				=	*reinterpret_cast<uint16_t*>(bytesToReverseStartAddr);
	*(uint16_t*)bytesToReverseStartAddr		=	((originalWordData & 0x00'FF) << 8 |
												 (originalWordData & 0xFF'00) >> 8);
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::Reverse4BytesInPlace(void* bytesToReverseStartAddr)
{
	uint32_t originalDWordData				=	*reinterpret_cast<uint32_t*>(bytesToReverseStartAddr);
	*(uint32_t*)bytesToReverseStartAddr		=	(((originalDWordData & 0x00'00'00'FF) << 24) |
												 ((originalDWordData & 0x00'00'FF'00) << 8)  |
												 ((originalDWordData & 0x00'FF'00'00) >> 8)  |
												 ((originalDWordData & 0xFF'00'00'00) >> 24));
}


//--------------------------------------------------------------------------------------------------
void BufferWriter::Reverse8BytesInPlace(void* bytesToReverseStartAddr)
{
	uint64_t originalDWordData				=	*reinterpret_cast<uint64_t*>(bytesToReverseStartAddr);
	*(uint64_t*)bytesToReverseStartAddr		=	(((originalDWordData & 0x00'00'00'00'00'00'00'FF) << 56) |
												 ((originalDWordData & 0x00'00'00'00'00'00'FF'00) << 40) |
												 ((originalDWordData & 0x00'00'00'00'00'FF'00'00) << 24) |
												 ((originalDWordData & 0x00'00'00'00'FF'00'00'00) << 8)  |
												 ((originalDWordData & 0x00'00'00'FF'00'00'00'00) >> 8)  |
												 ((originalDWordData & 0x00'00'FF'00'00'00'00'00) >> 24) |
												 ((originalDWordData & 0x00'FF'00'00'00'00'00'00) >> 40) |
												 ((originalDWordData & 0xFF'00'00'00'00'00'00'00) >> 56));
}


//--------------------------------------------------------------------------------------------------
BufferParser::BufferParser(unsigned char const* bufferToParse, uint32_t bufferSizeInBytes, eBufferEndian endianMode /*= eBufferEndian::NATIVE*/) :
	m_bufferStart(bufferToParse),
	m_bufferSize(bufferSizeInBytes)
{
	SetEndianMode(endianMode);
}


//--------------------------------------------------------------------------------------------------
BufferParser::BufferParser(Buffer const& buffer, eBufferEndian endianMode /*= eBufferEndian::NATIVE*/) : 
	m_bufferStart(buffer.data()),
	m_bufferSize((uint32_t)buffer.size())
{
	SetEndianMode(endianMode);
}


//--------------------------------------------------------------------------------------------------
eBufferEndian BufferParser::GetNativeEndianness() const
{
	int number = 0x12345678;
	unsigned char* firstByte = reinterpret_cast<unsigned char*>(&number);
	if (*firstByte == 0x12)
	{
		return eBufferEndian::BIG;
	}
	else
	{
		return eBufferEndian::LITTLE;
	}
}


//--------------------------------------------------------------------------------------------------
void BufferParser::SetEndianMode(eBufferEndian endianMode)
{
	eBufferEndian nativeEndianness = GetNativeEndianness();
	if (endianMode == eBufferEndian::NATIVE)
	{
		return;
	}
	m_currentEndianness				=	endianMode;
	m_isOppositeNativeEndianess		=	(nativeEndianness != endianMode);
}


//--------------------------------------------------------------------------------------------------
unsigned char BufferParser::ParseByte()
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 1 <= m_bufferSize, "Parsing Index out of bounds");
	return m_bufferStart[m_currentOffsetFromStart++];
}


//--------------------------------------------------------------------------------------------------
char BufferParser::ParseChar()
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 1 <= m_bufferSize, "Parsing Index out of bounds");
	return (char)ParseByte();
}


//--------------------------------------------------------------------------------------------------
bool BufferParser::ParseBool()
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 1 <= m_bufferSize, "Parsing Index out of bounds");
	return m_bufferStart[m_currentOffsetFromStart++];
}


//--------------------------------------------------------------------------------------------------
uint16_t BufferParser::ParseUShort16()
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 2 <= m_bufferSize, "Parsing Index out of bounds");
	uint16_t const* memAddr = (uint16_t*)(&m_bufferStart[m_currentOffsetFromStart]);
	uint16_t valAtGivenAddress = *memAddr;
	if (m_isOppositeNativeEndianess)
	{
		Reverse2BytesInPlace(&valAtGivenAddress);
	}
	m_currentOffsetFromStart += 2;
	return valAtGivenAddress;
}


//--------------------------------------------------------------------------------------------------
int16_t BufferParser::ParseShort16()
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 2 <= m_bufferSize, "Parsing Index out of bounds");
	int16_t const* memAddr		=	(int16_t*)(&m_bufferStart[m_currentOffsetFromStart]);
	int16_t valAtGivenAddress	=	*memAddr;
	if (m_isOppositeNativeEndianess)
	{
		Reverse2BytesInPlace(&valAtGivenAddress);
	}
	m_currentOffsetFromStart += 2;
	return valAtGivenAddress;
}


//--------------------------------------------------------------------------------------------------
uint32_t BufferParser::ParseUint32()
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 4 <= m_bufferSize, "Parsing Index out of bounds");
	uint32_t const* memAddr		=	(uint32_t*)(&m_bufferStart[m_currentOffsetFromStart]);
	uint32_t valAtGivenAddress	=	*memAddr;
	if (m_isOppositeNativeEndianess)
	{
		Reverse4BytesInPlace(&valAtGivenAddress);
	}
	m_currentOffsetFromStart += 4;
	return valAtGivenAddress;
}


//--------------------------------------------------------------------------------------------------
int32_t BufferParser::ParseInt32()
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 4 <= m_bufferSize, "Parsing Index out of bounds");
	int32_t const* memAddr = (int32_t*)(&m_bufferStart[m_currentOffsetFromStart]);
	int32_t valAtGivenAddress = *memAddr;
	if (m_isOppositeNativeEndianess)
	{
		Reverse4BytesInPlace(&valAtGivenAddress);
	}
	m_currentOffsetFromStart += 4;
	return valAtGivenAddress;
}


//--------------------------------------------------------------------------------------------------
uint64_t BufferParser::ParseUInt64()
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 8 <= m_bufferSize, "Parsing Index out of bounds");
	uint64_t const* memAddr = (uint64_t*)(&m_bufferStart[m_currentOffsetFromStart]);
	uint64_t valAtGivenAddress = *memAddr;
	if (m_isOppositeNativeEndianess)
	{
		Reverse8BytesInPlace(&valAtGivenAddress);
	}
	m_currentOffsetFromStart += 8;
	return valAtGivenAddress;
}


//--------------------------------------------------------------------------------------------------
int64_t BufferParser::ParseInt64()
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 8 <= m_bufferSize, "Parsing Index out of bounds");
	int64_t const* memAddr = (int64_t*)(&m_bufferStart[m_currentOffsetFromStart]);
	int64_t valAtGivenAddress = *memAddr;
	if (m_isOppositeNativeEndianess)
	{
		Reverse8BytesInPlace(&valAtGivenAddress);
	}
	m_currentOffsetFromStart += 8;
	return valAtGivenAddress;
}


//--------------------------------------------------------------------------------------------------
float BufferParser::ParseFloat()
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 4 <= m_bufferSize, "Parsing Index out of bounds");
	float const* memAddr = (float*)(&m_bufferStart[m_currentOffsetFromStart]);
	float valAtGivenAddress = *memAddr;
	if (m_isOppositeNativeEndianess)
	{
		Reverse4BytesInPlace(&valAtGivenAddress);
	}
	m_currentOffsetFromStart += 4;
	return valAtGivenAddress;
}


//--------------------------------------------------------------------------------------------------
double BufferParser::ParseDouble()
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 8 <= m_bufferSize, "Parsing Index out of bounds");
	double const* memAddr = reinterpret_cast<double const*>(&m_bufferStart[m_currentOffsetFromStart]);
	double valAtGivenAddress = *memAddr;
	if (m_isOppositeNativeEndianess)
	{
		Reverse8BytesInPlace(&valAtGivenAddress);
	}
	m_currentOffsetFromStart += 8;
	return valAtGivenAddress;
}


//--------------------------------------------------------------------------------------------------
void BufferParser::ParseStringZeroTerminated(std::string& parsedZeroTerminatedString)
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 1 <= m_bufferSize, "Parsing Index out of bounds");
	while (true)
	{
		char parsedChar				=	ParseChar();
		if (parsedChar == '\0')
		{
			return;
		}
		parsedZeroTerminatedString	+=	parsedChar;
	}
}


//--------------------------------------------------------------------------------------------------
void BufferParser::ParseStringAfter32BitLength(std::string& parsedString)
{
	GUARANTEE_OR_DIE(m_currentOffsetFromStart + 1 <= m_bufferSize, "Parsing Index out of bounds");
	size_t		stringIndex		=	parsedString.size();
	uint32_t	stringLength	=	ParseUint32();
	parsedString.reserve(stringLength + stringIndex);
	while (stringLength--)
	{
		char parsedChar =	ParseChar();
		parsedString	+=	parsedChar;
	}
}


//--------------------------------------------------------------------------------------------------
Vec2 BufferParser::ParseVec2()
{
	Vec2 parsedVec2;
	parsedVec2.x = ParseFloat();
	parsedVec2.y = ParseFloat();
	return parsedVec2;
}


//--------------------------------------------------------------------------------------------------
Vec3 BufferParser::ParseVec3()
{
	Vec3 parsedVec3;
	parsedVec3.x = ParseFloat();
	parsedVec3.y = ParseFloat();
	parsedVec3.z = ParseFloat();
	return parsedVec3;
}


//--------------------------------------------------------------------------------------------------
Vec4 BufferParser::ParseVec4()
{
	Vec4 parsedVec4;
	parsedVec4.x = ParseFloat();
	parsedVec4.y = ParseFloat();
	parsedVec4.z = ParseFloat();
	parsedVec4.w = ParseFloat();
	return parsedVec4;
}


//--------------------------------------------------------------------------------------------------
IntVec2 BufferParser::ParseIntVec2()
{
	IntVec2 parsedIntVec;
	parsedIntVec.x = ParseInt32();
	parsedIntVec.y = ParseInt32();
	return parsedIntVec;
}


//--------------------------------------------------------------------------------------------------
IntVec3 BufferParser::ParseIntVec3()
{
	IntVec3 parsedIntVec;
	parsedIntVec.x = ParseInt32();
	parsedIntVec.y = ParseInt32();
	parsedIntVec.z = ParseInt32();
	return parsedIntVec;
}


//--------------------------------------------------------------------------------------------------
Rgba8 BufferParser::ParseRgba()
{
	Rgba8 parsedColor;
	parsedColor.r = ParseChar();
	parsedColor.g = ParseChar();
	parsedColor.b = ParseChar();
	parsedColor.a = ParseChar();
	return parsedColor;
}


//--------------------------------------------------------------------------------------------------
Rgba8 BufferParser::ParseRgb()
{
	Rgba8 parsedColor;
	parsedColor.r = ParseChar();
	parsedColor.g = ParseChar();
	parsedColor.b = ParseChar();
	return parsedColor;
}


//--------------------------------------------------------------------------------------------------
AABB2 BufferParser::ParseAABB2()
{
	AABB2 parsedAABB;
	parsedAABB.m_mins = ParseVec2();
	parsedAABB.m_maxs = ParseVec2();
	return parsedAABB;
}


//--------------------------------------------------------------------------------------------------
AABB3 BufferParser::ParseAABB3()
{
	AABB3 parsedAABB;
	parsedAABB.m_mins = ParseVec3();
	parsedAABB.m_maxs = ParseVec3();
	return parsedAABB;
}


//--------------------------------------------------------------------------------------------------
OBB2 BufferParser::ParseOBB2()
{
	OBB2 parsedOBB;
	parsedOBB.m_center			=	ParseVec2();
	parsedOBB.m_iBasisNormal	=	ParseVec2();
	parsedOBB.m_halfDimensions	=	ParseVec2();
	return parsedOBB;
}


//--------------------------------------------------------------------------------------------------
OBB3 BufferParser::ParseOBB3()
{
	OBB3 parsedOBB;
	parsedOBB.m_center			=	ParseVec3();
	parsedOBB.m_iBasis			=	ParseVec3();
	parsedOBB.m_jBasis			=	ParseVec3();
	parsedOBB.m_kBasis			=	ParseVec3();
	parsedOBB.m_halfDims		=	ParseVec3();
	return parsedOBB;
}


//--------------------------------------------------------------------------------------------------
Plane2D BufferParser::ParsePlane2D()
{
	Plane2D parsedPlane;
	parsedPlane.m_normal			= ParseVec2();
	parsedPlane.m_distFromOrigin	= ParseFloat();
	return Plane2D();
}


//--------------------------------------------------------------------------------------------------
Vertex_PCU BufferParser::ParseVertexPCU()
{
	Vertex_PCU parsedVertex;
	parsedVertex.m_position		= ParseVec3();
	parsedVertex.m_color		= ParseRgba();
	parsedVertex.m_uvTexCoords	= ParseVec2();
	return parsedVertex;
}


//--------------------------------------------------------------------------------------------------
Vertex_PCUTBN BufferParser::ParseVertexPCUTBN()
{
	Vertex_PCUTBN parsedVertex;
	parsedVertex.m_position		=	ParseVec3();
	parsedVertex.m_color		=	ParseRgba();
	parsedVertex.m_uvTexCoords	=	ParseVec2();
	parsedVertex.m_tangent		=	ParseVec3();
	parsedVertex.m_binormal		=	ParseVec3();
	parsedVertex.m_normal		=	ParseVec3();
	return parsedVertex;
}


//--------------------------------------------------------------------------------------------------
uint32_t BufferParser::GetCurrentReadPosition() const
{
	return m_currentOffsetFromStart;
}


//--------------------------------------------------------------------------------------------------
void BufferParser::JumpCurrentReadPositionToDesiredOffsetWithinTheBuffer(uint32_t newOffset)
{
	GUARANTEE_OR_DIE(newOffset < m_bufferSize, "Please enter a valid offset that lies within the buffer");
	m_currentOffsetFromStart = newOffset;
}


//--------------------------------------------------------------------------------------------------
void BufferParser::Reverse2BytesInPlace(void* bytesToReverseStartAddr)
{
	uint16_t originalWordData				=	*reinterpret_cast<uint16_t*>(bytesToReverseStartAddr);
	*(uint16_t*)bytesToReverseStartAddr		=	((originalWordData & 0x00'FF) << 8 |
												 (originalWordData & 0xFF'00) >> 8);
}


//--------------------------------------------------------------------------------------------------
void BufferParser::Reverse4BytesInPlace(void* bytesToReverseStartAddr)
{
	uint32_t originalDWordData				=	*reinterpret_cast<uint32_t*>(bytesToReverseStartAddr);
	*(uint32_t*)bytesToReverseStartAddr		=	(((originalDWordData & 0x00'00'00'FF) << 24) |
												 ((originalDWordData & 0x00'00'FF'00) << 8)  |
												 ((originalDWordData & 0x00'FF'00'00) >> 8)  |
												 ((originalDWordData & 0xFF'00'00'00) >> 24));
}


//--------------------------------------------------------------------------------------------------
void BufferParser::Reverse8BytesInPlace(void* bytesToReverseStartAddr)
{
	uint64_t originalDWordData				=	*reinterpret_cast<uint64_t*>(bytesToReverseStartAddr);
	*(uint64_t*)bytesToReverseStartAddr		=	(((originalDWordData & 0x00'00'00'00'00'00'00'FF) << 56) |
												 ((originalDWordData & 0x00'00'00'00'00'00'FF'00) << 40) |
												 ((originalDWordData & 0x00'00'00'00'00'FF'00'00) << 24) |
												 ((originalDWordData & 0x00'00'00'00'FF'00'00'00) << 8)  |
												 ((originalDWordData & 0x00'00'00'FF'00'00'00'00) >> 8)  |
												 ((originalDWordData & 0x00'00'FF'00'00'00'00'00) >> 24) |
												 ((originalDWordData & 0x00'FF'00'00'00'00'00'00) >> 40) |
												 ((originalDWordData & 0xFF'00'00'00'00'00'00'00) >> 56));
}
