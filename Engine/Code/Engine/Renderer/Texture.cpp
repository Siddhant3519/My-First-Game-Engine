#include "Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <d3d11.h>

Texture::~Texture()
{
	DX_SAFE_RELEASE(m_texture);
	DX_SAFE_RELEASE(m_shaderResourceView);
	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_depthStencilView);
	DX_SAFE_RELEASE(m_readOnlyDepthStencilView);
	DX_SAFE_RELEASE(m_unorderedAccessView);
}
