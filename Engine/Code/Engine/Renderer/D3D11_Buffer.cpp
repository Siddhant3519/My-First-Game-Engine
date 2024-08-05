#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/D3D11_Buffer.hpp"


//--------------------------------------------------------------------------------------------------
#include <d3d11.h>


//--------------------------------------------------------------------------------------------------
D3D11_Buffer::D3D11_Buffer(unsigned int numOfElements, unsigned int elementSize) : 
	m_numOfElements(numOfElements),
	m_elementSize(elementSize)
{
}


//--------------------------------------------------------------------------------------------------
D3D11_Buffer::~D3D11_Buffer()
{
	DX_SAFE_RELEASE(m_unorderedAccessView);
	DX_SAFE_RELEASE(m_shaderResourceView);
	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_depthStencilView);
	DX_SAFE_RELEASE(m_buffer);
}


//--------------------------------------------------------------------------------------------------
unsigned int D3D11_Buffer::GetElementSize() const
{
	return m_elementSize;
}


//--------------------------------------------------------------------------------------------------
unsigned int D3D11_Buffer::GetNumOfElements() const
{
	return m_numOfElements;
}


//--------------------------------------------------------------------------------------------------
unsigned int D3D11_Buffer::GetBufferSizeInBytes() const
{
	return (m_numOfElements * m_elementSize);
}
