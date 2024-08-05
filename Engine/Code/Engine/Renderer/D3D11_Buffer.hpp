#pragma once


//--------------------------------------------------------------------------------------------------
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Buffer;


//--------------------------------------------------------------------------------------------------
class D3D11_Buffer
{
	friend class Renderer;
private:
	D3D11_Buffer() { };
	D3D11_Buffer(unsigned int numOfElements, unsigned int elementSize);
	D3D11_Buffer(D3D11_Buffer const& copy) = delete;
	~D3D11_Buffer();

public:
	unsigned int GetElementSize()	const;
	unsigned int GetNumOfElements()	const;
	unsigned int GetBufferSizeInBytes() const;

protected:
	ID3D11UnorderedAccessView*  m_unorderedAccessView	=	nullptr;
	ID3D11ShaderResourceView*	m_shaderResourceView	=	nullptr;
	ID3D11RenderTargetView*		m_renderTargetView		=	nullptr;
	ID3D11DepthStencilView*		m_depthStencilView		=	nullptr;
	ID3D11Buffer*				m_buffer				=	nullptr;

public:
	unsigned int m_numOfElements	=	0;
	unsigned int m_elementSize		=	0;
};