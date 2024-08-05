#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"


//--------------------------------------------------------------------------------------------------
#include <vector>
#include <stdint.h>
#include <Engine/Core/Rgba8.hpp>


//--------------------------------------------------------------------------------------------------
typedef std::vector<unsigned char> Buffer;


//--------------------------------------------------------------------------------------------------
enum class eBufferEndian : unsigned char
{
	NATIVE = 0,
	LITTLE,
	BIG,
};


//--------------------------------------------------------------------------------------------------
class BufferWriter
{
public:
	BufferWriter(Buffer& buffer, eBufferEndian endianMode = eBufferEndian::NATIVE);
	~BufferWriter() { };

	eBufferEndian GetNativeEndianness() const;
	eBufferEndian GetCurrentEndianness() const;
	void SetEndianMode(eBufferEndian endianMode);
	
	void AppendByte(uint8_t dataToAppend);
	void AppendChar(int8_t dataToAppend);
	void AppendBool(bool booleanToAppend);
	void AppendUShort16(uint16_t wordToAppend);
	void AppendShort16(int16_t dataToAppend);
	void AppendUInt32(uint32_t dwordToAppend);
	void AppendInt32(int32_t dwordToAppend);
	void AppendUInt64(uint64_t qwordToAppend);
	void AppendInt64(int64_t qwordToAppend);
	void AppendFloat(float floatToAppend);
	void AppendDouble(double doubleToAppend);
	void AppendStringZeroTerminated(/*int8_t*/ char const* stringToAppend);
	void AppendStringAfter32BitLength(/*int8_t*/ char const* stringToAppend);
	void AppendVec2(Vec2 vec2ToAppend);
	void AppendVec3(Vec3 const& vec3ToAppend);
	void AppendVec4(Vec4 const& vec4ToAppend);
	void AppendIntVec2(IntVec2 intVec2ToAppend);
	void AppendIntVec3(IntVec3 intVec2ToAppend);
	void AppendRgba(Rgba8 rgbaToAppend);
	void AppendRgb(Rgba8 rgbToAppend);
	void AppendAABB2(AABB2 const& aabbToAppend);
	void AppendAABB3(AABB3 const& aabbToAppend);
	void AppendOBB2(OBB2 const& obbToAppend);
	void AppendOBB3(OBB3 const& obbToAppend);
	void AppendPlane2D(Plane2D const& planeToAppend);
	void AppendVertexPCU(Vertex_PCU const& vertexToAppend);
	void AppendVertexPCUTBN(Vertex_PCUTBN const& vertexToAppend);
	void OverwriteUint32(uint64_t writePosOffset, uint32_t overwrittingDWord);

	void Reverse2BytesInPlace(void* bytesToReverseStartAddr);
	void Reverse4BytesInPlace(void* bytesToReverseStartAddr);
	void Reverse8BytesInPlace(void* bytesToReverseStartAddr);

private:
	Buffer&			m_bufferToWriteTo;
	eBufferEndian	m_currentEndianness			=	eBufferEndian::NATIVE;
	bool			m_isOppositeNativeEndian	=	false;
};


//--------------------------------------------------------------------------------------------------
class BufferParser
{
public:
	BufferParser(unsigned char const* bufferToParse, uint32_t bufferSizeInBytes, eBufferEndian endianMode = eBufferEndian::NATIVE);
	BufferParser(Buffer const& buffer, eBufferEndian endianMode = eBufferEndian::NATIVE);
	~BufferParser() { };

	eBufferEndian GetNativeEndianness() const;
	void SetEndianMode(eBufferEndian endianMode);

	unsigned char	ParseByte();
	char			ParseChar();
	bool			ParseBool();
	uint16_t		ParseUShort16();
	int16_t			ParseShort16();
	uint32_t		ParseUint32();
	int32_t			ParseInt32();
	uint64_t		ParseUInt64();
	int64_t			ParseInt64();
	float			ParseFloat();
	double			ParseDouble();
	void			ParseStringZeroTerminated(std::string& parsedZeroTerminatedString);
	void			ParseStringAfter32BitLength(std::string& parsedZeroTerminatedString);
	Vec2			ParseVec2();
	Vec3			ParseVec3();
	Vec4			ParseVec4();
	IntVec2			ParseIntVec2();
	IntVec3			ParseIntVec3();
	Rgba8			ParseRgba();
	Rgba8			ParseRgb();
	AABB2			ParseAABB2();
	AABB3			ParseAABB3();
	OBB2			ParseOBB2();
	OBB3			ParseOBB3();
	Plane2D			ParsePlane2D();
	Vertex_PCU		ParseVertexPCU();
	Vertex_PCUTBN	ParseVertexPCUTBN();

	uint32_t	GetCurrentReadPosition() const;
	void		JumpCurrentReadPositionToDesiredOffsetWithinTheBuffer(uint32_t newOffset);

	void Reverse2BytesInPlace(void* bytesToReverseStartAddr);
	void Reverse4BytesInPlace(void* bytesToReverseStartAddr);
	void Reverse8BytesInPlace(void* bytesToReverseStartAddr);

public:
	unsigned char const*	m_bufferStart					=	nullptr;
	uint32_t				m_bufferSize					=	0;
	uint32_t				m_currentOffsetFromStart		=	0;
	bool					m_isOppositeNativeEndianess		=	false;
	eBufferEndian			m_currentEndianness				=	eBufferEndian::NATIVE;
};