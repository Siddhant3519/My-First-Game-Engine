#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"


//--------------------------------------------------------------------------------------------------
#include <d3d11.h>


//--------------------------------------------------------------------------------------------------
StructuredBuffer::StructuredBuffer(size_t size) :
	m_size(size)
{
}


//--------------------------------------------------------------------------------------------------
StructuredBuffer::StructuredBuffer(size_t size, unsigned int stride) : 
	m_size(size),
	m_stride(stride)
{
}


//--------------------------------------------------------------------------------------------------
StructuredBuffer::~StructuredBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}


//--------------------------------------------------------------------------------------------------
unsigned int StructuredBuffer::GetStride() const
{
	return m_stride;
}
