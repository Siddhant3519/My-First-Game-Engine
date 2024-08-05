#pragma once


//--------------------------------------------------------------------------------------------------
struct ID3D11Buffer;


//--------------------------------------------------------------------------------------------------
class StructuredBuffer
{
	friend class Renderer;

public:
	StructuredBuffer(size_t size);
	StructuredBuffer(size_t size, unsigned int stride);
	StructuredBuffer(StructuredBuffer const& copy) = delete;
	virtual ~StructuredBuffer();

	unsigned int GetStride() const;

public:
	ID3D11Buffer*	m_buffer	=	nullptr;
	unsigned int	m_stride	=	0;
	size_t			m_size		=	0;
};