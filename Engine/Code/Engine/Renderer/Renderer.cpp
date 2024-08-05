#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/D3D11_Buffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Window/Window.hpp"


//--------------------------------------------------------------------------------------------------
#include "ThirdParty/stb/stb_image.h"


//--------------------------------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi.h>
#include <d3dcompiler.h>


//--------------------------------------------------------------------------------------------------
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

#if defined(ENGINE_DEBUG_RENDERER)
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif


//--------------------------------------------------------------------------------------------------
#if defined(OPAQUE)
#undef OPAQUE
#endif


//--------------------------------------------------------------------------------------------------
struct LightingConstants
{
	Vec3		sunDirection;											// 12 bytes
	float		sunIntensity;											// 4  bytes
	//-----------------------------------								(16 byte boundary)
	float		ambientIntensity;										// 4  bytes
	Vec3		worldEyePosition;										// 12 bytes
	//-----------------------------------								(16 byte boundary)
	Light		Lights[MAX_LIGHTS];										// 64 * 8	bytes				 
	//-----------------------------------								(64 * 8 || 16 byte boundary)
	int			normalMode;												// 4  bytes
	int			specularMode;											// 4  bytes
	float		specularIntensity;										// 4  bytes
	float		specularPower;											// 4  bytes
	//-----------------------------------								(16 byte boundary)
};
static int const s_lightingConstantsSlot = 1;


//--------------------------------------------------------------------------------------------------
struct CameraConstants
{
	Mat44 projectionMatrix;
	Mat44 viewMatrix;
};
static int const s_cameraConstantsSlot = 2;


//--------------------------------------------------------------------------------------------------
struct ModelConstants
{
	Mat44 modelMatrix;
	float modelColor[4];
};
static int const s_modelConstantsSlot = 3;


//--------------------------------------------------------------------------------------------------
Renderer::Renderer(RendererConfig const& config) :
	m_config(config)
{
}


//--------------------------------------------------------------------------------------------------
Renderer::~Renderer()
{

}


//--------------------------------------------------------------------------------------------------
void Renderer::Startup()
{
#if defined (ENGINE_DEBUG_RENDERER)
	m_dxgiDebugModule = (void*) ::LoadLibraryA("dxgidebug.dll");
	if (m_dxgiDebugModule == nullptr)
	{
		ERROR_AND_DIE("Could not load dxgidebug.dll");
	}

	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	((GetDebugModuleCB)::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))(__uuidof(IDXGIDebug), &m_dxgiDebug);

	if (m_dxgiDebug == nullptr)
	{
		ERROR_AND_DIE("Could not load debug module");
	}
#endif

	IntVec2 windowDim = m_config.m_window->GetClientDimensions(); // g_theWindow->GetClientDimensions();
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};										// swap chain is a collection of buffers

	swapChainDesc.BufferDesc.Width	=	windowDim.x;								// Back-buffer width
	swapChainDesc.BufferDesc.Height =	windowDim.y;								// Back-buffer height
	swapChainDesc.BufferDesc.Format =	DXGI_FORMAT_R8G8B8A8_UNORM;					// Color format 
	swapChainDesc.SampleDesc.Count	=	1;											// Multi sample anti aliasing (number of multi-samples)
	swapChainDesc.BufferUsage		=	DXGI_USAGE_RENDER_TARGET_OUTPUT;			// DXGI_USAGE_RENDER_TARGET_OUTPUT -> This specifies that the output should be used as the target output
	swapChainDesc.BufferCount		=	2;											// Number of buffers
	swapChainDesc.OutputWindow		=	(HWND)m_config.m_window->GetHwnd();			// The handle to the window
	swapChainDesc.Windowed			=	true;										// Specifies that the window is in windowed mode
	swapChainDesc.SwapEffect		=	DXGI_SWAP_EFFECT_FLIP_DISCARD;				// Flip signifies there is not copy between the buffers(back-buffer is swapped), Discards back-buffer after the swapchain

	unsigned int deviceFlags = 0; // D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT
#if defined(ENGINE_DEBUG_RENDERER)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	const uint8_t numOfFeatureLevels								=	7;
	D3D_FEATURE_LEVEL featureLevelsSupported[numOfFeatureLevels]	=	{	
																			D3D_FEATURE_LEVEL_11_1,
																			D3D_FEATURE_LEVEL_11_0,
																			D3D_FEATURE_LEVEL_10_1,
																			D3D_FEATURE_LEVEL_10_0,
																			D3D_FEATURE_LEVEL_9_3,
																			D3D_FEATURE_LEVEL_9_2,
																			D3D_FEATURE_LEVEL_9_1, 
																		};
	HRESULT hResult;
	hResult = D3D11CreateDeviceAndSwapChain(	NULL,											// Let it be null unless you want to specify the video adapter
												D3D_DRIVER_TYPE_HARDWARE,						// Enables the use of D3D on the Hardware(GPU)
												NULL,											// Only useful when D3D_DRIVER_TYPE |^ is D3D_DRIVER_TYPE_SOFTWARE
												deviceFlags,
												featureLevelsSupported,							// Feature level related (d3d version AND feature list)
												numOfFeatureLevels, 							// Feature level related (d3d version AND feature list)
												D3D11_SDK_VERSION,								// Lets the client know which version of DirectX was the game built on
												&swapChainDesc,									// Pointer to the Swap_chain_description struct
												&m_swapChain,
												&m_d3d11Device,
												NULL,											// Feature Level related
												&m_d3d11DeviceContext
											);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create D3D11 device and swap chain");
	}

	ID3D11Texture2D* backBuffer;
	// HRESULT is a data type that represents the completion status of a function
	hResult = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer); // Fills in the Back-buffer/ Gets the location of where in memory the back-buffer is
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not get swap chain buffer");
	}

	hResult = m_d3d11Device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView); // Create a COM object using that address to represent the render target
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could create render target view for swap chain buffer");
	}

	Shader* shader		=	CreateShader("Default", g_theShaderSource);
	m_defaultShader		=	shader;
	BindShader(shader);

	Image defaultImage = Image(IntVec2(1, 1), Rgba8(255, 255, 255));
	defaultImage.m_imageFilePath	=	"DEFAULT";
	m_defaultTexture				=	CreateTextureFromImage(defaultImage);
	BindTexture(m_defaultTexture);

	m_immediateVBO		=	CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	m_immediatePNCUVBO	=	CreateVertexBuffer(sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	m_immediateIBO		=	CreateIndexBuffer(sizeof(unsigned int));
	m_cameraCBO			=	CreateConstantBuffer(sizeof(CameraConstants));
	m_modelCBO			=	CreateConstantBuffer(sizeof(ModelConstants));
	m_lightingCBO		=	CreateConstantBuffer(sizeof(LightingConstants));
	
	CreateAndInitializeBlendModes();
	CreateSamplerModes();

	backBuffer->Release();
	
	CreateDefaultDepthTextureAndView();

	CreateDepthMode();

	CreateRasterizerMode();

	SetModelConstants();

	CreateUserDefinedDebugAnnotation();
}


//--------------------------------------------------------------------------------------------------
void Renderer::BeginFrame()
{
	m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	D3D11_VIEWPORT viewport =	{};
	viewport.MaxDepth		=	1;
	IntVec2 windowDim		=	m_config.m_window->GetClientDimensions();
	float	clientWidth		=	(float)windowDim.x;
	float	clientHeight	=	(float)windowDim.y;
	viewport.Width			=	clientWidth;
	viewport.Height			=	clientHeight;
	m_d3d11DeviceContext->RSSetViewports(1, &viewport);
}


//--------------------------------------------------------------------------------------------------
void Renderer::EndFrame()
{
	HRESULT hResult = m_swapChain->Present(0, 0);
	if (hResult == DXGI_ERROR_DEVICE_REMOVED || hResult == DXGI_ERROR_DEVICE_RESET)
	{
		ERROR_AND_DIE("Device has been lost application will now terminate");
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::Shutdown()
{
	for (int samplerIndex = 0; samplerIndex < int(SamplerMode::COUNT); ++samplerIndex)
	{
		DX_SAFE_RELEASE(m_samplerStates[samplerIndex]);
	}
	for (int blendIndex = 0; blendIndex < int(BlendMode::COUNT); ++blendIndex)
	{
		DX_SAFE_RELEASE(m_blendStates[blendIndex]);
	}
	for (int rasterizerIndex = 0; rasterizerIndex < int(RasterizerMode::COUNT); ++rasterizerIndex)
	{
		DX_SAFE_RELEASE(m_rasterizerStates[rasterizerIndex]);
	}
	for (int depthIndex = 0; depthIndex < int(DepthMode::COUNT); ++depthIndex)
	{
		DX_SAFE_RELEASE(m_depthStencilStates[depthIndex]);
	}

	DX_SAFE_RELEASE(m_customBlendState);
	DX_SAFE_RELEASE(m_debugAnnotation);
	DX_SAFE_RELEASE(m_depthStencilTexture);
	DX_SAFE_RELEASE(m_depthStencilView);
	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_d3d11DeviceContext);
	DX_SAFE_RELEASE(m_d3d11Device);
	for (int shaderIndex = 0; shaderIndex < m_loadedShaders.size(); ++shaderIndex)
	{
		delete m_loadedShaders[shaderIndex];
		m_loadedShaders[shaderIndex] = nullptr;
	}

	m_currentShader = nullptr;
	
	for (int textureIndex = 0; textureIndex < m_loadedTextures.size(); ++textureIndex)
	{
		delete m_loadedTextures[textureIndex];
		m_loadedTextures[textureIndex] = nullptr;
	}
	delete m_defaultTexture;
	m_defaultTexture = nullptr;

	for (int resourceIndex = 0; resourceIndex < m_loadedResources.size(); ++resourceIndex)
	{
		delete m_loadedResources[resourceIndex];
		m_loadedResources[resourceIndex] = nullptr;
	}

	delete m_immediateVBO;
	m_immediateVBO = nullptr;
	delete m_immediatePNCUVBO;
	m_immediatePNCUVBO = nullptr;
	delete m_immediateIBO;
	m_immediateIBO = nullptr;
	delete m_cameraCBO;
	m_cameraCBO = nullptr;
	delete m_modelCBO;
	m_modelCBO = nullptr;
	delete m_lightingCBO;
	m_lightingCBO = nullptr;

#if defined(ENGINE_DEBUG_RENDERER)
	((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));

	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;

	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif
}


//--------------------------------------------------------------------------------------------------
void Renderer::ClearScreen(Rgba8 const& clearColor)
{
	float clearColorAsFloats[4] = {};
	clearColor.GetAsFloats(clearColorAsFloats);
	m_d3d11DeviceContext->ClearRenderTargetView(m_renderTargetView, clearColorAsFloats);
	m_d3d11DeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BeginCamera(Camera const& camera)
{
	Mat44 projectionMat				=	camera.GetProjectionMatrix();
	Mat44 viewMat					=	camera.GetViewMatrix();
	CameraConstants camConstants	=	{};
	camConstants.projectionMatrix	=	projectionMat;
	camConstants.viewMatrix			=	viewMat;

	CopyCPUToGPU(&camConstants, sizeof(camConstants), m_cameraCBO);
	BindConstantBuffer(s_cameraConstantsSlot, m_cameraCBO);

	IntVec2 windowDim		=	m_config.m_window->GetClientDimensions();
	float width				=	(float)windowDim.x;
	float height			=	(float)windowDim.y;
	AABB2 clientBounds		=	AABB2(0.f, 0.f, width, height);

	D3D11_VIEWPORT viewport		=	{};
	AABB2 cameraViewport		=	camera.GetCameraViewport();
	
	AABB2 cameraBounds		=	clientBounds.GetBoxAtUVs(cameraViewport);
	Vec2 cameraDimensions	=	cameraBounds.GetDimensions();
	
	if (cameraViewport == AABB2::INVALID)
	{
		// Only set the viewport while binding Render and depth targets
		// viewport.Width		=	width;
		// viewport.Height		=	height;
		// viewport.MaxDepth	=	1;
		// m_d3d11DeviceContext->RSSetViewports(1, &viewport);
	}
	else
	{
		viewport.TopLeftX	=	cameraBounds.m_mins.x;
		viewport.TopLeftY	=	height - cameraBounds.m_maxs.y;
		viewport.Width		=	cameraDimensions.x;
		viewport.Height		=	cameraDimensions.y;
		viewport.MaxDepth = 1;
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::EndCamera(Camera const& camera)
{
	(void)camera;
}


//--------------------------------------------------------------------------------------------------
void Renderer::DrawVertexArray(int numVertexes, Vertex_PCU const* vertexes)
{
	CopyCPUToGPU(vertexes, (size_t)m_immediateVBO->GetStride() * numVertexes, m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, numVertexes);
}


//--------------------------------------------------------------------------------------------------
void Renderer::DrawVertexArray(std::vector<Vertex_PCU> const& vertexes, PrimitiveTopology const& primitiveTopology)
{
	int numVertexes = (int)vertexes.size();
	CopyCPUToGPU(vertexes.data(), (size_t)m_immediateVBO->GetStride() * numVertexes, m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, numVertexes, primitiveTopology);
}


//--------------------------------------------------------------------------------------------------
void Renderer::DrawIndexedArray(int numVertexes, Vertex_PCU const* vertexes, int numIndexes, unsigned int const* indexes)
{
	SetStatesIfChanged();
	CopyCPUToGPU(vertexes, (size_t)m_immediateVBO->GetStride() * numVertexes, m_immediateVBO);
	BindVertexBuffer(m_immediateVBO);
	CopyCPUToGPU(indexes, (size_t)m_immediateIBO->GetStride() * numIndexes, m_immediateIBO);
	BindIndexBuffer(m_immediateIBO);

	DrawIndexed(numIndexes);
}


//--------------------------------------------------------------------------------------------------
void Renderer::DrawIndexedArray(int numVertexes, Vertex_PCUTBN const* vertexes, int numIndexes, unsigned int const* indexes)
{
	SetStatesIfChanged();
	CopyCPUToGPU(vertexes, (size_t)m_immediatePNCUVBO->GetStride() * numVertexes, m_immediatePNCUVBO);
	BindVertexBuffer(m_immediatePNCUVBO);
	CopyCPUToGPU(indexes, (size_t)m_immediateIBO->GetStride() * numIndexes, m_immediateIBO);
	BindIndexBuffer(m_immediateIBO);

	DrawIndexed(numIndexes);
}


//--------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, unsigned char* texelData)
{
	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("CreateTextureFromData failed for \"%s\" - texelData was null!", name));
	GUARANTEE_OR_DIE(bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf("CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel));
	GUARANTEE_OR_DIE(dimensions.x > 0 && dimensions.y > 0, Stringf("CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y));

	Texture* newTexture = new Texture();
	newTexture->m_name = name; // NOTE: m_name must be a std::string, otherwise it may point to temporary data!
	newTexture->m_dimensions = dimensions;

	return newTexture;
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindTexture(Texture const* texture, BindingLocation bindingLocation)
{
	if (texture == nullptr)
	{
		texture = m_defaultTexture;
	}

	if (bindingLocation == BindingLocation::PIXEL_SHADER)
	{
		m_d3d11DeviceContext->PSSetShaderResources(0, 1, &texture->m_shaderResourceView);
	}
	if (bindingLocation == BindingLocation::VERTEX_SHADER)
	{
		m_d3d11DeviceContext->VSSetShaderResources(0, 1, &texture->m_shaderResourceView);
	}
	if (bindingLocation == BindingLocation::COMPUTE_SHADER)
	{
		m_d3d11DeviceContext->CSSetShaderResources(0, 1, &texture->m_shaderResourceView);
	}
}


//--------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromFile(char const* imageFilePath)
{
	Image* newImage = new Image(imageFilePath);
	Texture* newTexture = CreateTextureFromImage(*newImage);
	if (newTexture != nullptr)
	{
		m_loadedTextures.push_back(newTexture);
	}

	return newTexture;
}


//--------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromImage(Image const& image)
{
	IntVec2 textureDim					=	image.GetDimensions();
	D3D11_TEXTURE2D_DESC texture2DDesc	=	{};
	texture2DDesc.Width					=	textureDim.x;
	texture2DDesc.Height				=	textureDim.y;
	texture2DDesc.MipLevels				=	1;
	texture2DDesc.ArraySize				=	1;
	texture2DDesc.Format				=	DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2DDesc.Usage					=	D3D11_USAGE_IMMUTABLE;
	texture2DDesc.BindFlags				=	D3D11_BIND_SHADER_RESOURCE;
	texture2DDesc.SampleDesc.Count		=	1;

	D3D11_SUBRESOURCE_DATA subresourceData	=	{};
	subresourceData.pSysMem					=	image.GetRawData();
	if (texture2DDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		subresourceData.SysMemPitch = textureDim.x * 4;
	}
	else
	{
		ERROR_AND_DIE("We only support RGBA8 for now, format specified here is" + texture2DDesc.Format);
	}
	Texture* texture		=	new Texture;
	texture->m_dimensions	=	image.GetDimensions();
	texture->m_name			=	image.GetImageFilePath();

	HRESULT hResult;
	hResult = m_d3d11Device->CreateTexture2D(&texture2DDesc, &subresourceData, &texture->m_texture);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("could not create texture2D");
	}
	hResult = m_d3d11Device->CreateShaderResourceView(texture->m_texture, NULL, &texture->m_shaderResourceView);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a shader Resource View");
	}

	return texture;
}


//--------------------------------------------------------------------------------------------------
BitmapFont* Renderer::CreateBitmapFont(char const* imageFilePathWithNoExtension)
{
	std::string imageFilePathString		=	imageFilePathWithNoExtension + std::string(".png");
	char const* imageFilePath			=	imageFilePathString.c_str();
	Texture* bitmapFontTexture			=	CreateTextureFromFile(imageFilePath);
	BitmapFont* newBitmapFont			=	new BitmapFont(imageFilePathWithNoExtension, *bitmapFontTexture);
	m_loadedFonts.emplace_back(newBitmapFont);
	return newBitmapFont;
}


//--------------------------------------------------------------------------------------------------
Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureForFileName(imageFilePath);
	if (existingTexture)
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile(imageFilePath);
	return newTexture;
}


//--------------------------------------------------------------------------------------------------
BitmapFont* Renderer::CreateOrGetBitmapFont(char const* bitmapFontFilePathWithNoExtension)
{
	BitmapFont* existingBitmap = GetBitmapFontForFileName(bitmapFontFilePathWithNoExtension);
	if (existingBitmap)
	{
		return existingBitmap;
	}

	BitmapFont* newBitmapFont = CreateBitmapFont(bitmapFontFilePathWithNoExtension);
	return newBitmapFont;
}


//--------------------------------------------------------------------------------------------------
Shader* Renderer::CreateOrGetShader(char const* shaderFilePath, InputLayout const& inputLayout)
{
	for (int shaderIndex = 0; shaderIndex < m_loadedShaders.size(); ++shaderIndex)
	{
		if (m_loadedShaders[shaderIndex]->m_config.m_name == shaderFilePath)
		{
			return m_loadedShaders[shaderIndex];
		}
	}

	Shader* newShader = CreateShader(shaderFilePath, inputLayout);
	return newShader;
}


//--------------------------------------------------------------------------------------------------
Texture* Renderer::GetTextureForFileName(char const* imageFilePath)
{
	for (int textureIndex = 0; textureIndex < m_loadedTextures.size(); ++textureIndex)
	{
		if (m_loadedTextures[textureIndex]->m_name == imageFilePath)
		{
			return m_loadedTextures[textureIndex];
		}
	}

	return nullptr;
}


//--------------------------------------------------------------------------------------------------
BitmapFont* Renderer::GetBitmapFontForFileName(char const* bitmapFontFilePathWithNoExtension)
{
	for (int bitmapFontIndex = 0; bitmapFontIndex < m_loadedFonts.size(); ++bitmapFontIndex)
	{
		if (m_loadedFonts[bitmapFontIndex]->m_fontFilePathNameWithNoExtension == bitmapFontFilePathWithNoExtension)
		{
			return m_loadedFonts[bitmapFontIndex];
		}
	}

	return nullptr;
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindShader(Shader* shader, BindingLocation bindingLocation)
{
	if (shader == nullptr)
	{
		shader = m_defaultShader;
		m_d3d11DeviceContext->VSSetShader(nullptr, NULL, NULL);
		m_d3d11DeviceContext->PSSetShader(nullptr, NULL, NULL);
		m_d3d11DeviceContext->CSSetShader(nullptr, NULL, NULL);
	}

	m_d3d11DeviceContext->IASetInputLayout(shader->m_inputLayout);
	if (bindingLocation == BindingLocation::VERTEX_SHADER || bindingLocation == BindingLocation::PIXEL_SHADER)
	{
		m_d3d11DeviceContext->VSSetShader(shader->m_vertexShader, NULL, NULL);
		m_d3d11DeviceContext->PSSetShader(shader->m_pixelShader, NULL, NULL);
	}

	if (bindingLocation == BindingLocation::COMPUTE_SHADER)
	{
		m_d3d11DeviceContext->CSSetShader(shader->m_computeShader, NULL, NULL);
	}

	m_currentShader = shader;
}


//--------------------------------------------------------------------------------------------------
void Renderer::UnbindShaders()
{
	ID3D11ShaderResourceView* nullViews[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	m_d3d11DeviceContext->PSSetShaderResources(0, 8, nullViews);
}


//--------------------------------------------------------------------------------------------------
Shader* Renderer::CreateShader(char const* shaderName, char const* shaderSource, InputLayout const& inputLayout)
{
	HRESULT hResult;
	ShaderConfig shaderConfig;
	shaderConfig.m_name = shaderName;
	
	Shader* shader = new Shader(shaderConfig);

	std::vector<unsigned char> vertexShaderByteCode;
	bool isVertexShaderCompiled = CompileShaderToByteCode(vertexShaderByteCode, shaderName, shaderSource, shaderConfig.m_vertexEntryPoint.c_str(), "vs_5_0");
	if (isVertexShaderCompiled)
	{
		hResult = m_d3d11Device->CreateVertexShader(vertexShaderByteCode.data(), vertexShaderByteCode.size(), NULL, &shader->m_vertexShader);
		if (!SUCCEEDED(hResult))
		{
			ERROR_AND_DIE("Could not create a Vertex Shader")
		}
	}
	std::vector<unsigned char> pixelShaderByteCode;
	bool isPixelShaderCompiled = CompileShaderToByteCode(pixelShaderByteCode, shaderName, shaderSource, shaderConfig.m_pixelEntryPoint.c_str(), "ps_5_0");
	if (isPixelShaderCompiled)
	{
		hResult = m_d3d11Device->CreatePixelShader(pixelShaderByteCode.data(), pixelShaderByteCode.size(), NULL, &shader->m_pixelShader);
		if (!SUCCEEDED(hResult))
		{
			ERROR_AND_DIE("Could not create a Pixel Shader")
		}
	}

	if (inputLayout == InputLayout::VERTEX_PCUTBN)
	{
		// Input element is a property of a vertex, to give a vertex multiple properties we make an array of vertices like in our case with a array of PCUs
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,							 D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		}; // organizing the data in a way so that the GPU can understand how our Vertices are laid out in memory
		hResult = m_d3d11Device->CreateInputLayout(inputElementDesc, 6, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &shader->m_inputLayout);
	}
	else if (inputLayout == InputLayout::VERTEX_PCU)
	{
		// Input element is a property of a vertex, to give a vertex multiple properties we make an array of vertices like in our case with a array of PCUs
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,							 D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		}; // organizing the data in a way so that the GPU can understand how our Vertices are laid out in memory
		hResult = m_d3d11Device->CreateInputLayout(inputElementDesc, 3, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &shader->m_inputLayout);
	}
	else
	{
		ERROR_AND_DIE("Do not Support the specified Input Layout");
	}
	// Input element is a property of a vertex, to give a vertex multiple properties we make an array of vertices like in our case with a array of PCUs
	// D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	// {
	// 	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	// 	{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	// 	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	// }; // organizing the data in a way so that the GPU can understand how our Vertices are laid out in memory


	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create an input layout for how our vertices are laid out in memory")
	}

	m_loadedShaders.push_back(shader);

	return shader;
}


//--------------------------------------------------------------------------------------------------
Shader* Renderer::CreateShader(char const* shaderName, InputLayout const& inputLayout)
{
	std::string shaderFilePath = shaderName + std::string(".hlsl");
	std::string outShaderContents;
	int shaderItemsRead = FileReadToString(outShaderContents, shaderFilePath);
	if (shaderItemsRead == 0)
	{
		ERROR_AND_DIE("Failed to read the shader file: " + shaderFilePath);
	}
	Shader* shader = CreateShader(shaderName, outShaderContents.c_str(), inputLayout);
	return shader;
}


//--------------------------------------------------------------------------------------------------
Shader* Renderer::CreateComputeShader(char const* shaderName)
{
	std::string shaderFilePath = shaderName + std::string(".hlsl");
	
	std::string shaderSource;
	int shaderItemsRead = FileReadToString(shaderSource, shaderFilePath);
	if (shaderItemsRead == 0)
	{
		ERROR_AND_DIE("Failed to read the shader file: " + shaderFilePath);
	}
	
	ShaderConfig shaderConfig;
	shaderConfig.m_name = shaderName;

	Shader* shader = new Shader(shaderConfig);

	std::vector<unsigned char> computeShaderByteCode;
	bool isComputeShaderCompiled = CompileShaderToByteCode(computeShaderByteCode, shaderName, shaderSource.c_str(), shaderConfig.m_computeShaderEntryPoint.c_str(), "cs_5_0");
	if (isComputeShaderCompiled)
	{
		HRESULT hResult = m_d3d11Device->CreateComputeShader(computeShaderByteCode.data(), computeShaderByteCode.size(), NULL, &shader->m_computeShader);
		if (!SUCCEEDED(hResult))
		{
			ERROR_AND_DIE("Could not create a Compute Shader")
		}
	}

	m_loadedShaders.push_back(shader);

	return shader;
}


//--------------------------------------------------------------------------------------------------
bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target)
{
	unsigned int flags1 = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDERER)
	flags1 = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

	ID3DBlob* pointerToAccessCompiledCode = nullptr; 
	ID3DBlob* pointerToAccessErrorMesg = nullptr;

	HRESULT hResult;
	hResult = D3DCompile(source,
		strlen(source),
		name,
		NULL,
		NULL,
		entryPoint,
		target,
		flags1,
		0,
		&pointerToAccessCompiledCode,	// shader blob (compiled code)
		&pointerToAccessErrorMesg); // error code

	if (!SUCCEEDED(hResult))
	{
		DebuggerPrintf((char const*)pointerToAccessErrorMesg->GetBufferPointer());
		ERROR_AND_DIE("Could not compile the HLSL shader source");
	}
	outByteCode.resize(pointerToAccessCompiledCode->GetBufferSize());
	memcpy(outByteCode.data(), pointerToAccessCompiledCode->GetBufferPointer(), pointerToAccessCompiledCode->GetBufferSize());

	DX_SAFE_RELEASE(pointerToAccessCompiledCode);
	DX_SAFE_RELEASE(pointerToAccessErrorMesg);

	return true;
}


//--------------------------------------------------------------------------------------------------
VertexBuffer* Renderer::CreateVertexBuffer(size_t const size)
{
	// // Vertices are stored in the system memory, to get them on the video memory for rendering we need to create a buffer on the video memory as well
	D3D11_BUFFER_DESC vertexBufferDesc = {};									// When rendering needs our vertices Direct3D will copy contents over to the video memory.

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;								// Signifies data is accessible by both GPU(read only) and CPU(write only), cpu updates the resource every frame
	vertexBufferDesc.ByteWidth = (unsigned int)size;							// Size of the buffer to be created, It has to be same size as the array of vertices we want it to reflect
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;						// Type of buffer we want to make, in this case it's a vertex buffer, the buffer is bound to pipeline as vertex buffer
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;					// The buffer is mappable in such a way that its contents are directly effected by CPU, CPU can change it's contents

	HRESULT hResult;
	VertexBuffer* vertexBuffer = new VertexBuffer(size);
	hResult = m_d3d11Device->CreateBuffer(&vertexBufferDesc, NULL, &vertexBuffer->m_buffer); // The second parameter specifies the default initialization value, having it be NULL signifies only space is allocated and the values will be garbage
	// hResult = m_d3d11Device->CreateBuffer(&vertexBufferDesc, NULL, &m_immediateVBO->m_buffer); // The second parameter specifies the default initialization value, having it be NULL signifies only space is allocated and the values will be garbage
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create the vertex buffer");
	}
	// m_immediateVBO->m_size = size;
	return vertexBuffer;
}


//--------------------------------------------------------------------------------------------------
VertexBuffer* Renderer::CreateVertexBuffer(size_t const size, unsigned int stride)
{
	// // Vertices are stored in the system memory, to get them on the video memory for rendering we need to create a buffer on the video memory as well
	D3D11_BUFFER_DESC vertexBufferDesc = {};									// When rendering needs our vertices Direct3D will copy contents over to the video memory.

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;								// Signifies data is accessible by both GPU(read only) and CPU(write only), cpu updates the resource every frame
	vertexBufferDesc.ByteWidth = (unsigned int)size;							// Size of the buffer to be created, It has to be same size as the array of vertices we want it to reflect
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;						// Type of buffer we want to make, in this case it's a vertex buffer, the buffer is bound to pipeline as vertex buffer
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;					// The buffer is mappable in such a way that its contents are directly effected by CPU, CPU can change it's contents

	HRESULT hResult;
	VertexBuffer* vertexBuffer = new VertexBuffer(size, stride);
	hResult = m_d3d11Device->CreateBuffer(&vertexBufferDesc, NULL, &vertexBuffer->m_buffer); // The second parameter specifies the default initialization value, having it be NULL signifies only space is allocated and the values will be garbage
	// hResult = m_d3d11Device->CreateBuffer(&vertexBufferDesc, NULL, &m_immediateVBO->m_buffer); // The second parameter specifies the default initialization value, having it be NULL signifies only space is allocated and the values will be garbage
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create the vertex buffer");
	}
	// m_immediateVBO->m_size = size;
	return vertexBuffer;
}


//--------------------------------------------------------------------------------------------------
VertexBuffer* Renderer::CreateVertexBuffer(size_t const size, unsigned int stride, ResourceUsage bufferUsage, void* defaultInitializationData, bool isStreamOut)
{
	size_t vertexBufferSizeInBytes = size * stride;
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = (unsigned int)(vertexBufferSizeInBytes);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	if (isStreamOut)
	{
		vertexBufferDesc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
	}

	switch (bufferUsage)
	{
	case ResourceUsage::GPU_READ:
	{
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		break;
	}
	case ResourceUsage::GPU_READ_GPU_WRITE:
	{
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		break;
	}
	case ResourceUsage::GPU_READ_CPU_WRITE:
	{
		vertexBufferDesc.Usage			= D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		break;
	}
	case ResourceUsage::GPU_READ_CPU_READ_GPU_WRITE_CPU_WRITE:
	{
		vertexBufferDesc.Usage			= D3D11_USAGE_STAGING;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		break;
	}
	default:
	{
		ERROR_AND_DIE("Please provide a valid buffer usage when creating the vertex buffer");
		break;
	}
	}

	D3D11_SUBRESOURCE_DATA initializationData = { };
	initializationData.pSysMem = defaultInitializationData;

	VertexBuffer* vertexBuffer = new VertexBuffer(vertexBufferSizeInBytes, stride);
	HRESULT hResult = m_d3d11Device->CreateBuffer(&vertexBufferDesc, &initializationData, &vertexBuffer->m_buffer);
	GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create a vertex buffer");

	return vertexBuffer;
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateVertexBuffer(D3D11_Buffer* out_buffer, unsigned int numOfElements, unsigned int elementSize, ResourceUsage const& bufferUsage, void* defaultDataToBePopulated, bool isStreamOut)
{
	unsigned int vertexBufferSizeInBytes = numOfElements * elementSize;
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = (unsigned int)(vertexBufferSizeInBytes);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	if (isStreamOut)
	{
		vertexBufferDesc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
	}

	switch (bufferUsage)
	{
	case ResourceUsage::GPU_READ:
	{
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		break;
	}
	case ResourceUsage::GPU_READ_GPU_WRITE:
	{
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		break;
	}
	case ResourceUsage::GPU_READ_CPU_WRITE:
	{
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		break;
	}
	case ResourceUsage::GPU_READ_CPU_READ_GPU_WRITE_CPU_WRITE:
	{
		vertexBufferDesc.Usage = D3D11_USAGE_STAGING;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		break;
	}
	default:
	{
		ERROR_AND_DIE("Please provide a valid buffer usage when creating the vertex buffer");
		break;
	}
	}

	D3D11_SUBRESOURCE_DATA initializationData = { };
	initializationData.pSysMem = defaultDataToBePopulated;

	out_buffer = new D3D11_Buffer(numOfElements, elementSize);
	HRESULT hResult = m_d3d11Device->CreateBuffer(&vertexBufferDesc, &initializationData, &out_buffer->m_buffer);
	GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create a vertex buffer");
}


//--------------------------------------------------------------------------------------------------
IndexBuffer* Renderer::CreateIndexBuffer(size_t const size)
{
	// // Vertices are stored in the system memory, to get them on the video memory for rendering we need to create a buffer on the video memory as well
	D3D11_BUFFER_DESC indexBufferDesc = {};										// When rendering needs our vertices Direct3D will copy contents over to the video memory.

	indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;								// Signifies data is accessible by both GPU(read only) and CPU(write only), cpu updates the resource every frame
	indexBufferDesc.ByteWidth = (unsigned int)size;								// Size of the buffer to be created, It has to be same size as the array of vertices we want it to reflect
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;						// Type of buffer we want to make, in this case it's a vertex buffer, the buffer is bound to pipeline as vertex buffer
	indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;					// The buffer is mappable in such a way that its contents are directly effected by CPU, CPU can change it's contents

	HRESULT hResult;
	IndexBuffer* indexBuffer = new IndexBuffer(size);
	hResult = m_d3d11Device->CreateBuffer(&indexBufferDesc, NULL, &indexBuffer->m_buffer); // The second parameter specifies the default initialization value, having it be NULL signifies only space is allocated and the values will be garbage
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create the index buffer");
	}
	return indexBuffer;
}


//--------------------------------------------------------------------------------------------------
IndexBuffer* Renderer::CreateIndexBuffer(size_t const size, ResourceUsage bufferUsage, void* defaultInitializationData)
{
	size_t indexBufferSizeInBytes = size * sizeof(unsigned int);

	D3D11_BUFFER_DESC indexBufferDesc = { };
	
	indexBufferDesc.ByteWidth		=	(unsigned int)indexBufferSizeInBytes;
	indexBufferDesc.BindFlags		=	D3D11_BIND_INDEX_BUFFER;
	
	switch (bufferUsage)
	{
	case ResourceUsage::GPU_READ:
	{
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		break;
	}
	case ResourceUsage::GPU_READ_GPU_WRITE:
	{
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		break;
	}
	case ResourceUsage::GPU_READ_CPU_WRITE:
	{
		indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		break;
	}
	case ResourceUsage::GPU_READ_CPU_READ_GPU_WRITE_CPU_WRITE:
	{
		indexBufferDesc.Usage = D3D11_USAGE_STAGING;
		indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		break;
	}
	default:
	{
		ERROR_AND_DIE("Please provide a valid buffer usage when creating the vertex buffer");
		break;
	}
	}

	D3D11_SUBRESOURCE_DATA initializationData;
	initializationData.pSysMem = defaultInitializationData;

	IndexBuffer* indexBuffer = new IndexBuffer(indexBufferSizeInBytes);
	HRESULT hResult = m_d3d11Device->CreateBuffer(&indexBufferDesc, &initializationData, &indexBuffer->m_buffer);
	GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create an index buffer");

	return indexBuffer;
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateStructuredBuffer(D3D11_Buffer*& out_buffer, unsigned int numOfElements, unsigned int elementSize, ResourceUsage const& bufferUsage, void* defaultDataToBePopulated, bool isStandard, ResourceViewFormat const resourceViewFormat, UAVResourceFlag const uavResourceFlag)
{
	D3D11_BUFFER_DESC structuredBufferDesc		=	{ };
	structuredBufferDesc.ByteWidth				=	numOfElements * elementSize;
	structuredBufferDesc.MiscFlags				=	D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	structuredBufferDesc.StructureByteStride	=	elementSize;
	structuredBufferDesc.BindFlags				=	D3D11_BIND_SHADER_RESOURCE;

	switch (bufferUsage)
	{
	case ResourceUsage::GPU_READ:
	{
		structuredBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		break;
	}
	case ResourceUsage::GPU_READ_GPU_WRITE:
	{
		structuredBufferDesc.Usage		 =	D3D11_USAGE_DEFAULT;
		structuredBufferDesc.BindFlags	|=	D3D11_BIND_UNORDERED_ACCESS;
		break;
	}
	case ResourceUsage::GPU_READ_CPU_WRITE:
	{
		structuredBufferDesc.Usage				=	D3D11_USAGE_DYNAMIC;
		structuredBufferDesc.CPUAccessFlags		=	D3D11_CPU_ACCESS_WRITE;
		break;
	}
	case ResourceUsage::GPU_READ_CPU_READ_GPU_WRITE_CPU_WRITE:
	{
		ERROR_AND_DIE("Don't create a structured buffer that is staging!!!");
		// structuredBufferDesc.Usage = D3D11_USAGE_STAGING;
		// structuredBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		// break;
	}
	default:
	{
		ERROR_AND_DIE("Please provide a valid buffer usage when creating the structured buffer");
	}
	}

	out_buffer = new D3D11_Buffer(numOfElements, elementSize);

	if (defaultDataToBePopulated)
	{
		D3D11_SUBRESOURCE_DATA initializationData = { };
		initializationData.pSysMem = defaultDataToBePopulated;
		HRESULT hResult = m_d3d11Device->CreateBuffer(&structuredBufferDesc, &initializationData, &out_buffer->m_buffer);
		GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a structured buffer!!!");
	}
	else
	{
		HRESULT hResult = m_d3d11Device->CreateBuffer(&structuredBufferDesc, NULL, &out_buffer->m_buffer);
		GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a structured buffer!!!");
	}
	

	if (structuredBufferDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
		if (isStandard)
		{
			srvDesc.Format = DXGI_FORMAT(resourceViewFormat);
		}
		else
		{
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		}
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements	= numOfElements;

		HRESULT hResult = m_d3d11Device->CreateShaderResourceView(out_buffer->m_buffer, &srvDesc, &out_buffer->m_shaderResourceView);
		GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a structured buffers shader resource view!!!");
	}
	if (structuredBufferDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };
		if (isStandard)
		{
			uavDesc.Format = DXGI_FORMAT(resourceViewFormat);
		}
		else
		{
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		}
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements	= numOfElements;
		uavDesc.Buffer.Flags		= (unsigned int)uavResourceFlag;

		HRESULT hResult = m_d3d11Device->CreateUnorderedAccessView(out_buffer->m_buffer, &uavDesc, &out_buffer->m_unorderedAccessView);
		GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a structured buffers unordered access view!!!");
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateAppendConsumeBuffer(D3D11_Buffer*& out_buffer, unsigned int numOfElements, unsigned int elementSize, void* defaultDataToBePopulated)
{
	// Create append and consume buffer
	D3D11_BUFFER_DESC appendConsumeBufferDesc		=	{ };
	appendConsumeBufferDesc.StructureByteStride		=	elementSize;
	appendConsumeBufferDesc.ByteWidth				=	numOfElements * elementSize;
	appendConsumeBufferDesc.BindFlags				=	D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	appendConsumeBufferDesc.MiscFlags				=	D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	appendConsumeBufferDesc.Usage					=	D3D11_USAGE_DEFAULT;
	
	out_buffer = new D3D11_Buffer(numOfElements, elementSize);

	if (defaultDataToBePopulated)
	{
		D3D11_SUBRESOURCE_DATA initializationData = { };
		initializationData.pSysMem = defaultDataToBePopulated;
		HRESULT hResult = m_d3d11Device->CreateBuffer(&appendConsumeBufferDesc, &initializationData, &out_buffer->m_buffer);
		GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating an Append/Consume buffer!!!");
	}
	else
	{
		HRESULT hResult = m_d3d11Device->CreateBuffer(&appendConsumeBufferDesc, NULL, &out_buffer->m_buffer);
		GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a Append/Consume buffer!!!");
	}

	// Create Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };

	srvDesc.Format				=	DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension		=	D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement =	0;
	srvDesc.Buffer.NumElements	=	numOfElements;

	HRESULT hResult = m_d3d11Device->CreateShaderResourceView(out_buffer->m_buffer, &srvDesc, &out_buffer->m_shaderResourceView);
	GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a structured buffers shader resource view!!!");

	// Create Unordered Access View
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };
	uavDesc.Format				=	DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension		=	D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement	=	0;
	uavDesc.Buffer.NumElements	=	numOfElements;
	uavDesc.Buffer.Flags		=	D3D11_BUFFER_UAV_FLAG_APPEND;

	hResult = m_d3d11Device->CreateUnorderedAccessView(out_buffer->m_buffer, &uavDesc, &out_buffer->m_unorderedAccessView);
	GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a structured buffers unordered access view!!!");
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateRawBuffer(D3D11_Buffer*& out_buffer, unsigned int numOfElements, ResourceUsage const& bufferUsage, void* defaultDataToBePopulated)
{
	constexpr int RAW_BUFFER_STRIDE		=	4;
	D3D11_BUFFER_DESC rawBufferDesc		=	{ };
	rawBufferDesc.ByteWidth				=	numOfElements * RAW_BUFFER_STRIDE;
	rawBufferDesc.MiscFlags				=	D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	rawBufferDesc.StructureByteStride	=	0;
	rawBufferDesc.BindFlags				|=	D3D11_BIND_SHADER_RESOURCE;


	switch (bufferUsage)
	{
	case ResourceUsage::GPU_READ:
	{
		rawBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		break;
	}
	case ResourceUsage::GPU_READ_GPU_WRITE:
	{
		rawBufferDesc.Usage		=	D3D11_USAGE_DEFAULT;
		rawBufferDesc.BindFlags |=	D3D11_BIND_UNORDERED_ACCESS;
		break;
	}
	case ResourceUsage::GPU_READ_CPU_WRITE:
	{
		rawBufferDesc.Usage				=	D3D11_USAGE_DYNAMIC;
		rawBufferDesc.CPUAccessFlags	=	D3D11_CPU_ACCESS_WRITE;
		break;
	}
	case ResourceUsage::GPU_READ_CPU_READ_GPU_WRITE_CPU_WRITE:
	{
		ERROR_AND_DIE("Don't create a raw buffer that is staging!!!");
		// structuredBufferDesc.Usage = D3D11_USAGE_STAGING;
		// structuredBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		// break;
	}
	default:
	{
		ERROR_AND_DIE("Please provide a valid buffer usage when creating the raw buffer");
	}
	}

	out_buffer = new D3D11_Buffer(numOfElements, RAW_BUFFER_STRIDE);

	if (defaultDataToBePopulated)
	{
		D3D11_SUBRESOURCE_DATA initializationData	=	{ };
		initializationData.pSysMem					=	defaultDataToBePopulated;
		HRESULT hResult = m_d3d11Device->CreateBuffer(&rawBufferDesc, &initializationData, &out_buffer->m_buffer);
		GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a raw buffer!!!");
	}
	else
	{
		HRESULT hResult = m_d3d11Device->CreateBuffer(&rawBufferDesc, NULL, &out_buffer->m_buffer);
		GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a raw buffer!!!");
	}

	if (rawBufferDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
		srvDesc.BufferEx.Flags			=	D3D11_BUFFEREX_SRV_FLAG_RAW;
		srvDesc.BufferEx.FirstElement	=	0;
		srvDesc.BufferEx.NumElements	=	numOfElements;
		srvDesc.ViewDimension			=	D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.Format					=	DXGI_FORMAT_R32_TYPELESS;

		HRESULT hResult = m_d3d11Device->CreateShaderResourceView(out_buffer->m_buffer, &srvDesc, &out_buffer->m_shaderResourceView);
		GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a raw buffers shader resource view!!!");
	}
	if (rawBufferDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };

		uavDesc.Format					=	DXGI_FORMAT_R32_TYPELESS;
		uavDesc.ViewDimension			=	D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement		=	0;
		uavDesc.Buffer.NumElements		=	numOfElements;
		uavDesc.Buffer.Flags			=	D3D11_BUFFER_UAV_FLAG_RAW;

		HRESULT hResult = m_d3d11Device->CreateUnorderedAccessView(out_buffer->m_buffer, &uavDesc, &out_buffer->m_unorderedAccessView);
		GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a structured buffers unordered access view!!!");
	}
}

//--------------------------------------------------------------------------------------------------
void Renderer::CopyCPUToGPU(void const* data, size_t size, VertexBuffer*& vbo)
{
	if (vbo->m_size < size)
	{
		int newStride = vbo->GetStride();
		DX_SAFE_RELEASE(vbo->m_buffer);
		vbo = CreateVertexBuffer(size, newStride);
	}
	// We now have Vertices(system memory) and a vertex buffer(video memory) to place them in, now all we are required to do is copy the vertices in-to the buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;																	// Will be filled with all the imp info about the buffer including a pointer to the buffer's location
	HRESULT hResult = m_d3d11DeviceContext->Map(vbo->m_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);	// Maps the buffer(GPU is blocked from using this buffer), giving us access to it, map_write_discard clears the prev contents of the buffers, and opens the new buffer for writting
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not map the buffer");
	}
	memcpy(mappedSubresource.pData, data, size);												// copies the contents from system memory to video memory
	m_d3d11DeviceContext->Unmap(vbo->m_buffer, NULL);														// GPU regains access to the buffer, CPU losses access
}


//--------------------------------------------------------------------------------------------------
void Renderer::CopyCPUToGPU(void const* data, size_t size, IndexBuffer*& ibo)
{
	if (ibo->m_size < size)
	{
		DX_SAFE_RELEASE(ibo->m_buffer);
		ibo = CreateIndexBuffer(size);
	}
	// We now have Vertices(system memory) and a vertex buffer(video memory) to place them in, now all we are required to do is copy the vertices in-to the buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;																	// Will be filled with all the imp info about the buffer including a pointer to the buffer's location
	HRESULT hResult = m_d3d11DeviceContext->Map(ibo->m_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);	// Maps the buffer(GPU is blocked from using this buffer), giving us access to it, map_write_discard clears the prev contents of the buffers, and opens the new buffer for writting
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not map the index buffer");
	}
	memcpy(mappedSubresource.pData, data, size);												// copies the contents from system memory to video memory
	m_d3d11DeviceContext->Unmap(ibo->m_buffer, NULL);														// GPU regains access to the buffer, CPU losses access
}


//--------------------------------------------------------------------------------------------------
void Renderer::CopyGPUToCPU(D3D11_Resource const* resourceToCopy, void*& out_data)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hResult;
	switch (resourceToCopy->m_resourceType)
	{
		case ResourceType::TEXTURE1D:
		{
			hResult = m_d3d11DeviceContext->Map(resourceToCopy->m_texture1d, NULL, D3D11_MAP_READ, NULL, &mappedSubresource);
			break;
		}
		case ResourceType::TEXTURE2D:
		{
			hResult = m_d3d11DeviceContext->Map(resourceToCopy->m_texture2d, NULL, D3D11_MAP_READ, NULL, &mappedSubresource);
			break;
		}
		case ResourceType::TEXTURE3D:
		{
			hResult = m_d3d11DeviceContext->Map(resourceToCopy->m_texture3d, NULL, D3D11_MAP_READ, NULL, &mappedSubresource);
			break;
		}
		case ResourceType::STRUCTURED_BUFFER:
		{
			hResult = m_d3d11DeviceContext->Map(resourceToCopy->m_buffer, NULL, D3D11_MAP_READ, NULL, &mappedSubresource);
			break;
		}
		case ResourceType::RAW_BUFFER:
		{
			hResult = m_d3d11DeviceContext->Map(resourceToCopy->m_buffer, NULL, D3D11_MAP_READ, NULL, &mappedSubresource);
			break;
		}
		default:
		{
			ERROR_AND_DIE("Please provide a valid resource type");
		}
	}

	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not map the Resource");
	}

	free(out_data);
	out_data = malloc(size_t(resourceToCopy->m_numOfElements) * size_t(resourceToCopy->m_elementStride));
	// size_t numOfBytes = size_t(resourceToCopy->m_numOfElements) * size_t(resourceToCopy->m_elementStride);
	// out_data = realloc(out_data, numOfBytes);
	memcpy(out_data, mappedSubresource.pData, size_t(resourceToCopy->m_numOfElements) * size_t(resourceToCopy->m_elementStride));
	switch (resourceToCopy->m_resourceType)
	{
		case ResourceType::TEXTURE1D:
		{
			m_d3d11DeviceContext->Unmap(resourceToCopy->m_texture1d, NULL);
			break;
		}
		case ResourceType::TEXTURE2D:
		{
			m_d3d11DeviceContext->Unmap(resourceToCopy->m_texture2d, NULL);
			break;
		}
		case ResourceType::TEXTURE3D:
		{
			m_d3d11DeviceContext->Unmap(resourceToCopy->m_texture3d, NULL);
			break;
		}
		case ResourceType::STRUCTURED_BUFFER:
		{
			m_d3d11DeviceContext->Unmap(resourceToCopy->m_buffer, NULL);
			break;
		}
		case ResourceType::RAW_BUFFER:
		{
			m_d3d11DeviceContext->Unmap(resourceToCopy->m_buffer, NULL);
			break;
		}
		default:
		{
			ERROR_AND_DIE("Please provide a valid resource type");
		}
	}
}


//--------------------------------------------------------------------------------------------------
// Binds a single vertex buffer to the pipeline for the Input Assembler 
void Renderer::BindVertexBuffer(VertexBuffer* vbo, PrimitiveTopology primitiveTopology)
{
	unsigned int vertexStride = vbo->GetStride();
	unsigned int offset = 0;
	// Set vertex buffers requires an offset from where to start in the buffer, the number of vertex buffers to be bound, pointer to the array of vertex buffers
	// the strides of all the vertex buffers to be bound and the offset of all the vertex buffers
	m_d3d11DeviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &vertexStride, &offset);
	m_d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY(primitiveTopology));
}


//--------------------------------------------------------------------------------------------------
// void Renderer::BindVertexBuffers(VertexBuffer** arrayOfVertexBuffers, unsigned int* arrayOfStrides, unsigned int* arrayOfOffsets, unsigned int numOfVertexBuffersToBind, unsigned int startOffset)
// {
// 	ID3D11Buffer*	pBuffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
// 	unsigned int	pStrides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
// 	unsigned int	pOffsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = { 0 };
// 
// // Need to fetch the Id3d11Buffer component from the VertexBuffer and use those populate pBuffers
// 	memcpy(pBuffers, arrayOfVertexBuffers,	numOfVertexBuffersToBind);
// 	memcpy(pStrides, arrayOfStrides,		numOfVertexBuffersToBind);
// 	memcpy(pOffsets, arrayOfOffsets,		numOfVertexBuffersToBind);
// 	m_d3d11DeviceContext->IASetVertexBuffers(startOffset, numOfVertexBuffersToBind, pBuffers, pStrides, pOffsets);
// 	m_d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
// }


//--------------------------------------------------------------------------------------------------
// Binds the index buffer to the pipeline for the Input Assembler
void Renderer::BindIndexBuffer(IndexBuffer* ibo)
{
	m_d3d11DeviceContext->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, 0);
}


//--------------------------------------------------------------------------------------------------
ConstantBuffer* Renderer::CreateConstantBuffer(size_t const size)
{
	D3D11_BUFFER_DESC constantBufferDesc = {};

	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;								// Signifies GPU(read access), CPU (write access)
	constantBufferDesc.ByteWidth = (unsigned int)size;									// size of the buffer to be created
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;					// Type of buffer we want to make, in this case it's a constant buffer
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;					// The buffer is mapped in such a way that it's contents are directly effected by the CPU, CPU can change it's contents

	HRESULT hResult;
	ConstantBuffer* constantBuffer = new ConstantBuffer(size);
	hResult = m_d3d11Device->CreateBuffer(&constantBufferDesc, NULL, &constantBuffer->m_buffer);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a constant buffer");
	}

	return constantBuffer;
}



//--------------------------------------------------------------------------------------------------
ConstantBuffer* Renderer::CreateConstantBuffer(unsigned int const& size, bool isDynamic, bool isCPUUpdate, void* defaultInitializationData)
{
	D3D11_BUFFER_DESC constantBufferDesc = {};

	constantBufferDesc.ByteWidth = size;								// Must be a multiple of 16
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	if (isDynamic && isCPUUpdate)
	{
		constantBufferDesc.Usage			=	D3D11_USAGE_DYNAMIC;
		constantBufferDesc.CPUAccessFlags	=	D3D11_CPU_ACCESS_WRITE;
	}
	else if (isDynamic && !isCPUUpdate)
	{
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	}
	else
	{
		constantBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	}

	D3D11_SUBRESOURCE_DATA initializationData = { };
	initializationData.pSysMem = defaultInitializationData;

	ConstantBuffer* constantBuffer = new ConstantBuffer(size_t(size));
	HRESULT hResult = m_d3d11Device->CreateBuffer(&constantBufferDesc, &initializationData, &constantBuffer->m_buffer);
	GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a constant buffer");

	return constantBuffer;
}


//--------------------------------------------------------------------------------------------------
void Renderer::CopyCPUToGPU(void const* data, size_t size, ConstantBuffer*& cbo)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hResult = m_d3d11DeviceContext->Map(cbo->m_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not map the constantBuffer");
	}
	memcpy(mappedSubresource.pData, data, size);
	m_d3d11DeviceContext->Unmap(cbo->m_buffer, NULL);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindConstantBuffer(int slot, ConstantBuffer* cbo, BindingLocation bindingLocation)
{
	if (bindingLocation == BindingLocation::VERTEX_SHADER || bindingLocation == BindingLocation::PIXEL_SHADER)
	{
		m_d3d11DeviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
		m_d3d11DeviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	}
	if (bindingLocation == BindingLocation::COMPUTE_SHADER)
	{
		m_d3d11DeviceContext->CSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, PrimitiveTopology const& primitiveTopology, int vertexOffset)
{
	SetStatesIfChanged();
	BindVertexBuffer(vbo, primitiveTopology);
	m_d3d11DeviceContext->Draw(vertexCount, vertexOffset);
}


//--------------------------------------------------------------------------------------------------
void Renderer::DrawVertexAndIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset, int vertexOffset)
{
	SetStatesIfChanged();
	BindVertexBuffer(vbo);
	BindIndexBuffer(ibo);

	m_d3d11DeviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}


//--------------------------------------------------------------------------------------------------
void Renderer::DrawIndexedBuffer(IndexBuffer* ibo, VertexBuffer* vbo, unsigned int indexCount, unsigned int indexOffset /*= 0*/, int vertexOffset /*= 0*/)
{
	SetStatesIfChanged();
	BindVertexBuffer(vbo);
	BindIndexBuffer(ibo);
	m_d3d11DeviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}


//--------------------------------------------------------------------------------------------------
void Renderer::DrawIndexed(int indexCount, int indexOffset, int vertexOffset)
{
	m_d3d11DeviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}


//--------------------------------------------------------------------------------------------------
void Renderer::ComputeShaderDispatch(unsigned int threadGroupX, unsigned int threadGroupY, unsigned int threadGroupZ)
{
	m_d3d11DeviceContext->Dispatch(threadGroupX, threadGroupY, threadGroupZ);
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateUserDefinedDebugAnnotation()
{
	HRESULT hResult = m_d3d11DeviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&m_debugAnnotation);
	if (FAILED(hResult))
	{
		return;
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateAndInitializeBlendModes()
{
	D3D11_BLEND_DESC blendStateDesc = {};
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;

	blendStateDesc.RenderTarget[0].BlendOp					=	D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].BlendOpAlpha				=	D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask	=	D3D11_COLOR_WRITE_ENABLE_ALL;

	// Alpha blending using the Under operator, Front to back blending
	blendStateDesc.RenderTarget[0].SrcBlendAlpha	=	D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha	=	D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].SrcBlend			=	D3D11_BLEND_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend		=	D3D11_BLEND_ONE;
	HRESULT hResult;
	hResult = m_d3d11Device->CreateBlendState(&blendStateDesc, &m_blendStates[int(BlendMode::ALPHA_UNDER)]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a blend state")
	}

	// Alpha blending using the Over operator, Back to front blending
	blendStateDesc.RenderTarget[0].SrcBlendAlpha	=	D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha	=	D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].SrcBlend			=	D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend		=	D3D11_BLEND_INV_SRC_ALPHA;
	hResult = m_d3d11Device->CreateBlendState(&blendStateDesc, &m_blendStates[int(BlendMode::ALPHA)]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a blend state")
	}

	// Additive blending
	blendStateDesc.RenderTarget[0].SrcBlend			=	D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlend		=	D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha	=	D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha	=	D3D11_BLEND_ZERO;
	hResult = m_d3d11Device->CreateBlendState(&blendStateDesc, &m_blendStates[int(BlendMode::ADDITIVE)]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a blend state")
	}

	// Opaque Blending
	blendStateDesc.RenderTarget[0].SrcBlend		=	D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlend	=	D3D11_BLEND_ZERO;
	hResult = m_d3d11Device->CreateBlendState(&blendStateDesc, &m_blendStates[int(BlendMode::OPAQUE)]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a blend state")
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateSamplerModes()
{
	HRESULT hResult;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter			=	D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU		=	D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV		=	D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW		=	D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc	=	D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD			=	D3D11_FLOAT32_MAX;

	hResult = m_d3d11Device->CreateSamplerState(&samplerDesc, &m_samplerStates[int(SamplerMode::POINT_CLAMP)]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create POINT_CLAMP sampler state");
	}

	samplerDesc.Filter		=	D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU	=	D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV	=	D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW	=	D3D11_TEXTURE_ADDRESS_WRAP;

	hResult = m_d3d11Device->CreateSamplerState(&samplerDesc, &m_samplerStates[int(SamplerMode::BILINEAR_WRAP)]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a BILINEAR_WRAP sampler state");
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateRasterizerMode()
{
	HRESULT hResult;
	D3D11_RASTERIZER_DESC rasterizerDesc	=	{};
	rasterizerDesc.FillMode					=	D3D11_FILL_SOLID;													// Fill triangles formed by vertices
	rasterizerDesc.CullMode					=	D3D11_CULL_BACK;													// Only draw front face
	rasterizerDesc.FrontCounterClockwise	=	true;
	rasterizerDesc.DepthClipEnable			=	true;
	rasterizerDesc.AntialiasedLineEnable	=	true;

	hResult = m_d3d11Device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_BACK]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a rasterizer state");
	}

	rasterizerDesc.FillMode =	D3D11_FILL_SOLID;
	rasterizerDesc.CullMode =	D3D11_CULL_FRONT;
	hResult					=	m_d3d11Device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_FRONT]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a rasterizer state");
	}

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;													
	rasterizerDesc.CullMode = D3D11_CULL_NONE;													
	
	hResult = m_d3d11Device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a rasterizer state");
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;	

	hResult = m_d3d11Device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_NONE]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a rasterizer state");
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;												
	rasterizerDesc.CullMode = D3D11_CULL_BACK;													

	hResult = m_d3d11Device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_BACK]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a rasterizer state");
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateDepthMode()
{
	HRESULT hResult;
	
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc	=	{};
	depthStencilDesc.DepthEnable				=	true;
	depthStencilDesc.DepthWriteMask				=	D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc					=	D3D11_COMPARISON_LESS_EQUAL;

	hResult = m_d3d11Device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::ENABLED]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a depth stencil state");
	}

	depthStencilDesc.DepthWriteMask		=	D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc			=	D3D11_COMPARISON_GREATER_EQUAL;
	hResult								=	m_d3d11Device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::GREATER]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a \"greater\" depth stencil state");
	}

	depthStencilDesc.DepthWriteMask		=	D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc			=	D3D11_COMPARISON_ALWAYS;

	hResult = m_d3d11Device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::DISABLED]);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a depth stencil state");
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetBlendMode(BlendMode blendMode)
{
	m_desiredBlendMode = blendMode;
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateBlendModes(BlendMode const* blendModes, unsigned short numOfBlendModes)
{	
	HRESULT hResult;
	D3D11_BLEND_DESC blendStateDesc			=	{};
	blendStateDesc.IndependentBlendEnable	=	numOfBlendModes > 1;
	for (unsigned short blendModeIndex = 0; blendModeIndex < numOfBlendModes; ++blendModeIndex)
	{
		BlendMode const& currentBlendMode									=	blendModes[blendModeIndex];
		D3D11_RENDER_TARGET_BLEND_DESC& currentRenderTargetBlendDesc		=	blendStateDesc.RenderTarget[blendModeIndex];
		currentRenderTargetBlendDesc.BlendEnable							=	TRUE;
		currentRenderTargetBlendDesc.BlendOp								=	D3D11_BLEND_OP_ADD;
		currentRenderTargetBlendDesc.BlendOpAlpha							=	D3D11_BLEND_OP_ADD;
		switch (currentBlendMode)
		{
			case BlendMode::ALPHA:
			{
				currentRenderTargetBlendDesc.SrcBlend				=	D3D11_BLEND_SRC_ALPHA;
				currentRenderTargetBlendDesc.DestBlend				=	D3D11_BLEND_INV_SRC_ALPHA;
				currentRenderTargetBlendDesc.BlendOpAlpha			=	D3D11_BLEND_OP_ADD;
				currentRenderTargetBlendDesc.SrcBlendAlpha			=	D3D11_BLEND_ZERO;
				currentRenderTargetBlendDesc.DestBlendAlpha			=	D3D11_BLEND_ONE;
				currentRenderTargetBlendDesc.RenderTargetWriteMask	=	D3D11_COLOR_WRITE_ENABLE_ALL;
				break;
			}
			case BlendMode::ADDITIVE:
			{
				currentRenderTargetBlendDesc.SrcBlend				=	D3D11_BLEND_SRC_ALPHA;
				currentRenderTargetBlendDesc.DestBlend				=	D3D11_BLEND_ONE;
				currentRenderTargetBlendDesc.SrcBlendAlpha			=	D3D11_BLEND_SRC_ALPHA;
				currentRenderTargetBlendDesc.DestBlendAlpha			=	D3D11_BLEND_ONE;
				currentRenderTargetBlendDesc.RenderTargetWriteMask	=	D3D11_COLOR_WRITE_ENABLE_ALL;
				break;
			}
			case BlendMode::OPAQUE:
			{
				currentRenderTargetBlendDesc.SrcBlend				=	D3D11_BLEND_ONE;
				currentRenderTargetBlendDesc.DestBlend				=	D3D11_BLEND_ZERO;
				currentRenderTargetBlendDesc.SrcBlendAlpha			=	D3D11_BLEND_ONE;
				currentRenderTargetBlendDesc.DestBlendAlpha			=	D3D11_BLEND_ZERO;
				currentRenderTargetBlendDesc.RenderTargetWriteMask	=	D3D11_COLOR_WRITE_ENABLE_ALL;
				break;
			}
			case BlendMode::ACCUMULATION:
			{
				currentRenderTargetBlendDesc.SrcBlend				=	D3D11_BLEND_ONE;
				currentRenderTargetBlendDesc.DestBlend				=	D3D11_BLEND_ONE;
				currentRenderTargetBlendDesc.SrcBlendAlpha			=	D3D11_BLEND_ONE;
				currentRenderTargetBlendDesc.DestBlendAlpha			=	D3D11_BLEND_ONE;
				currentRenderTargetBlendDesc.RenderTargetWriteMask	=	D3D11_COLOR_WRITE_ENABLE_ALL;
				break;
			}
			case BlendMode::REVEALAGE:
			{
				currentRenderTargetBlendDesc.SrcBlend				=	D3D11_BLEND_ZERO;
				currentRenderTargetBlendDesc.DestBlend				=	D3D11_BLEND_INV_SRC_COLOR;
				currentRenderTargetBlendDesc.SrcBlendAlpha			=	D3D11_BLEND_ZERO;
				currentRenderTargetBlendDesc.DestBlendAlpha			=	D3D11_BLEND_ZERO;
				currentRenderTargetBlendDesc.RenderTargetWriteMask	=	D3D11_COLOR_WRITE_ENABLE_ALL;
				break;
			}
			default:
			{
				ERROR_AND_DIE("Provide a valid blend mode");
			}
		}
	}

	hResult = m_d3d11Device->CreateBlendState(&blendStateDesc, &m_customBlendState);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a custom blend state")
	}

#if defined(ENGINE_DEBUG_RENDERER)
	m_customBlendState->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)sizeof("CustomBlendMode"), "CustomBlendMode");
#endif		
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetCustomBlendMode()
{
	float blendFactor[4]		=	{ 0 };
	unsigned int sampleMask		=	0xffffffff;
	m_desiredBlendMode			=	BlendMode::INVALID;
	m_d3d11DeviceContext->OMSetBlendState(m_customBlendState, blendFactor, sampleMask);
	m_blendState = m_customBlendState;
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetSamplerMode(SamplerMode samplerMode)
{
	m_desiredSamplerMode = samplerMode;
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetRasterizerMode(RasterizerMode rasterizerMode)
{
	m_desiredRasterizedMode = rasterizerMode;
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetDepthMode(DepthMode depthMode)
{
	m_desiredDepthMode = depthMode;
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetStatesIfChanged()
{
	if (m_blendStates[int(m_desiredBlendMode)] != m_blendState)
	{
		m_blendState				=	m_desiredBlendMode != BlendMode::INVALID ? m_blendStates[int(m_desiredBlendMode)] : m_blendState;
		float blendFactor[4]		=	{ 0 };
		unsigned int sampleMask		=	0xffffffff;
		m_d3d11DeviceContext->OMSetBlendState(m_blendState, blendFactor, sampleMask);
	}

	if (m_samplerStates[int(m_desiredSamplerMode)] != m_d3d11SamplerState)
	{
		m_d3d11SamplerState = m_samplerStates[int(m_desiredSamplerMode)];
		m_d3d11DeviceContext->PSSetSamplers(0, 1, &m_d3d11SamplerState);
	}

	if (m_rasterizerStates[int(m_desiredRasterizedMode)] != m_d3d11RasterizeState)
	{
		m_d3d11RasterizeState = m_rasterizerStates[int(m_desiredRasterizedMode)];
		m_d3d11DeviceContext->RSSetState(m_d3d11RasterizeState);
	}

	if (m_depthStencilStates[int(m_desiredDepthMode)] != m_depthStencilState)
	{
		m_depthStencilState = m_depthStencilStates[int(m_desiredDepthMode)];
		m_d3d11DeviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetModelConstants(Mat44 const& modelMatrix, Rgba8 const& modelColor)
{
	ModelConstants modelConstants;
	modelConstants.modelMatrix = modelMatrix;
	modelColor.GetAsFloats(modelConstants.modelColor);
	CopyCPUToGPU(&modelConstants, sizeof(modelConstants), m_modelCBO);
	BindConstantBuffer(s_modelConstantsSlot, m_modelCBO);
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetLightingConstants(Vec3 const& sunDirection, float sunIntensity, float ambientIntensity, Vec3 worldEyePosition /*= Vec3::ZERO*/, int normalMode /*= 0*/, int specularMode /*= 0*/, float specularIntensity /*= 0.f*/, float specularPower /*= 0.f*/)
{
	LightingConstants lightingConstants;

	lightingConstants.sunDirection			=	sunDirection;
	lightingConstants.sunIntensity			=	sunIntensity;
	lightingConstants.ambientIntensity		=	ambientIntensity;
	lightingConstants.worldEyePosition		=	worldEyePosition;
	lightingConstants.normalMode			=	normalMode;
	lightingConstants.specularMode			=	specularMode;
	lightingConstants.specularIntensity		=	specularIntensity;
	lightingConstants.specularPower			=	specularPower;
	memcpy(&lightingConstants.Lights, &m_lights, sizeof(m_lights));

	CopyCPUToGPU(&lightingConstants, sizeof(lightingConstants), m_lightingCBO);
	BindConstantBuffer(s_lightingConstantsSlot, m_lightingCBO);
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetLightAt(Light const& lightToSet, int indexToSet)
{
	bool isIndexValid	=	indexToSet < MAX_LIGHTS ? true : false;
	isIndexValid		=	isIndexValid && (indexToSet >= 0 ? true : false);
	
	if (!isIndexValid) return;

	m_lights[indexToSet] = lightToSet;
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetCustomAnnotationMarker(wchar_t const* annotationText)
{
	m_debugAnnotation->SetMarker(annotationText);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BeginAnnotationEvent(wchar_t const* annotationText)
{
	int numOfAnnotationEventsStartedButNotEnded = m_debugAnnotation->BeginEvent(annotationText);

	// redundant
	if (numOfAnnotationEventsStartedButNotEnded == -1)
	{
		return;
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::EndAnnotationEvent()
{
	m_debugAnnotation->EndEvent();
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateDefaultDepthTextureAndView()
{
	IntVec2 const& windowDim = m_config.m_window->GetClientDimensions();
	HRESULT hResult;

	D3D11_TEXTURE2D_DESC depthTextureDesc	=	{};
	depthTextureDesc.Width					=	windowDim.x;
	depthTextureDesc.Height					=	windowDim.y;
	depthTextureDesc.MipLevels				=	1;
	depthTextureDesc.ArraySize				=	1;
	depthTextureDesc.Usage					=	D3D11_USAGE_DEFAULT;
	depthTextureDesc.Format					=	DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTextureDesc.BindFlags				=	D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.SampleDesc.Count		=	1;

	hResult = m_d3d11Device->CreateTexture2D(&depthTextureDesc, nullptr, &m_depthStencilTexture);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a depth stencil texture");
	}

	hResult = m_d3d11Device->CreateDepthStencilView(m_depthStencilTexture, nullptr, &m_depthStencilView);
	if (!SUCCEEDED(hResult))
	{
		ERROR_AND_DIE("Could not create a depth stencil view");
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateNewDepthTextureAndView(Texture*& depthTexture, unsigned int debugResourceNameSize, char const* debugResourceName, IntVec2 const& textureDims)
{
	IntVec2 const& textureDim =	(textureDims.x == -1 || textureDims.y == -1) ? m_config.m_window->GetClientDimensions() : textureDims;
	HRESULT hResult;

	D3D11_TEXTURE2D_DESC depthTextureDesc	=	{};
	depthTextureDesc.Width					=	textureDim.x;
	depthTextureDesc.Height					=	textureDim.y;
	depthTextureDesc.MipLevels				=	1;
	depthTextureDesc.ArraySize				=	1;
	depthTextureDesc.Usage					=	D3D11_USAGE_DEFAULT;
	depthTextureDesc.Format					=	DXGI_FORMAT_R32_TYPELESS;
	depthTextureDesc.BindFlags				=	D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthTextureDesc.SampleDesc.Count		=	1;

	depthTexture = new Texture;
	depthTexture->m_dimensions = textureDim;
	hResult = m_d3d11Device->CreateTexture2D(&depthTextureDesc, nullptr, &depthTexture->m_texture);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create a depth Stencil texture");
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = { };
	depthStencilViewDesc.Format			=	DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension	=	D3D11_DSV_DIMENSION_TEXTURE2D;

	hResult = m_d3d11Device->CreateDepthStencilView(depthTexture->m_texture, &depthStencilViewDesc, &depthTexture->m_depthStencilView);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create the depth stencil view");
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = { };
	shaderResourceViewDesc.Format				= DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension		= D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels	= 1;

	hResult = m_d3d11Device->CreateShaderResourceView(depthTexture->m_texture, &shaderResourceViewDesc, &depthTexture->m_shaderResourceView);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create the shader resource view");
	}

	if (debugResourceNameSize != 0)
	{
		SetDebugResourceName(depthTexture, debugResourceNameSize, debugResourceName);
	}

	m_loadedTextures.push_back(depthTexture);
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateWritableRenderTarget(Texture*& renderTarget, unsigned int debugResourceNameSize, char const* debugResourceName, IntVec2 const& textureDims)
{
	IntVec2 const& textureDim	=	textureDims.x == -1 || textureDims.y == -1 ? m_config.m_window->GetClientDimensions() : textureDims;
	HRESULT hResult;

	D3D11_TEXTURE2D_DESC renderTargetTextureDesc	=	{};
	renderTargetTextureDesc.Width					=	textureDim.x;
	renderTargetTextureDesc.Height					=	textureDim.y;
	renderTargetTextureDesc.MipLevels				=	1;
	renderTargetTextureDesc.ArraySize				=	1;
	renderTargetTextureDesc.Usage					=	D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.Format					=	DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetTextureDesc.BindFlags				=	D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	renderTargetTextureDesc.SampleDesc.Count		=	1;

	renderTarget				=	new Texture;
	renderTarget->m_dimensions	=	textureDim;
	hResult = m_d3d11Device->CreateTexture2D(&renderTargetTextureDesc, nullptr, &renderTarget->m_texture);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create a render target texture");
	}

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc		=	{};
	renderTargetViewDesc.Format								=	renderTargetTextureDesc.Format;
	renderTargetViewDesc.ViewDimension						=	D3D11_RTV_DIMENSION_TEXTURE2D;
	hResult = m_d3d11Device->CreateRenderTargetView(renderTarget->m_texture, &renderTargetViewDesc, &renderTarget->m_renderTargetView);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create a Render Target View");
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc = { };
	unorderedAccessViewDesc.Format			=	renderTargetTextureDesc.Format;
	unorderedAccessViewDesc.ViewDimension	=	D3D11_UAV_DIMENSION_TEXTURE2D;
	hResult = m_d3d11Device->CreateUnorderedAccessView(renderTarget->m_texture, &unorderedAccessViewDesc, &renderTarget->m_unorderedAccessView);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create the unordered Access View");
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc		=	{};
	shaderResourceViewDesc.Format								=	renderTargetTextureDesc.Format;
	shaderResourceViewDesc.ViewDimension						=	D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels					=	renderTargetTextureDesc.MipLevels;

	hResult = m_d3d11Device->CreateShaderResourceView(renderTarget->m_texture, &shaderResourceViewDesc, &renderTarget->m_shaderResourceView);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create the shader resource view");
	}

	if (debugResourceNameSize != 0)
	{
		SetDebugResourceName(renderTarget, debugResourceNameSize, debugResourceName);
	}

	m_loadedTextures.push_back(renderTarget);
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateNewRenderTextureShaderResourceAndView(Texture*& textureToRenderTo) const
{
	IntVec2 const& windowDim = m_config.m_window->GetClientDimensions();
	HRESULT hResult;

	D3D11_TEXTURE2D_DESC renderTargetTextureDesc	=	{};
	renderTargetTextureDesc.Width					=	windowDim.x;
	renderTargetTextureDesc.Height					=	windowDim.y;
	renderTargetTextureDesc.MipLevels				=	1;
	renderTargetTextureDesc.ArraySize				=	1;
	renderTargetTextureDesc.Usage					=	D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.Format					=	DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetTextureDesc.BindFlags				=	D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTargetTextureDesc.SampleDesc.Count		=	1;

	textureToRenderTo = new Texture;
	hResult = m_d3d11Device->CreateTexture2D(&renderTargetTextureDesc, nullptr, &textureToRenderTo->m_texture);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create a render target texture");
	}

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc	=	{};
	renderTargetViewDesc.Format							=	renderTargetTextureDesc.Format;
	renderTargetViewDesc.ViewDimension					=	D3D11_RTV_DIMENSION_TEXTURE2D;

	hResult = m_d3d11Device->CreateRenderTargetView(textureToRenderTo->m_texture, &renderTargetViewDesc, &textureToRenderTo->m_renderTargetView);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create a Render Target View");
	}
	
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc	=	{};
	shaderResourceViewDesc.Format							=	renderTargetTextureDesc.Format;
	shaderResourceViewDesc.ViewDimension					=	D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels				=	renderTargetTextureDesc.MipLevels;

	hResult = m_d3d11Device->CreateShaderResourceView(textureToRenderTo->m_texture, &shaderResourceViewDesc, &textureToRenderTo->m_shaderResourceView);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create the shader resource view");
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateNewShaderResourceView(Texture*& textureContainingSRV) const
{
	HRESULT hResult;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = { };
	shaderResourceViewDesc.Format				=	DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension		=	D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels	=	1;

	hResult = m_d3d11Device->CreateShaderResourceView(textureContainingSRV->m_texture, &shaderResourceViewDesc, &textureContainingSRV->m_shaderResourceView);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create the shader resource view");
	}
}


//--------------------------------------------------------------------------------------------------
// #HACK:
void Renderer::CreateReadOnlyDepthStencilView(Texture*& texture) const
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc	=	{ };
	depthStencilViewDesc.Format							=	DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension					=	D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags							=	D3D11_DSV_READ_ONLY_DEPTH;
	HRESULT hResult										=	m_d3d11Device->CreateDepthStencilView(texture->m_texture, &depthStencilViewDesc, &texture->m_readOnlyDepthStencilView);
	GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create a depth stencil view for the desired texture2D resource")
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateWritableTextureAndView(Texture*& textureContainingUAV)
{
	IntVec2 const& windowDim					=	m_config.m_window->GetClientDimensions();
	D3D11_TEXTURE2D_DESC writableTextureDesc	=	{};
	writableTextureDesc.Width					=	windowDim.x;
	writableTextureDesc.Height					=	windowDim.y;
	writableTextureDesc.MipLevels				=	1;
	writableTextureDesc.ArraySize				=	1;
	writableTextureDesc.Usage					=	D3D11_USAGE_DEFAULT;
	writableTextureDesc.Format					=	DXGI_FORMAT_R32G32B32A32_FLOAT;
	writableTextureDesc.BindFlags				=	D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	writableTextureDesc.SampleDesc.Count		=	1;

	textureContainingUAV	=	new Texture;
	HRESULT hResult			=	m_d3d11Device->CreateTexture2D(&writableTextureDesc, nullptr, &textureContainingUAV->m_texture);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create a writable texture");
	}


	D3D11_UNORDERED_ACCESS_VIEW_DESC writableTextureUAVDesc		=	{ };
	writableTextureUAVDesc.Format								=	writableTextureDesc.Format;
	writableTextureUAVDesc.ViewDimension						=	D3D11_UAV_DIMENSION_TEXTURE2D;

	hResult = m_d3d11Device->CreateUnorderedAccessView(textureContainingUAV->m_texture, &writableTextureUAVDesc, &textureContainingUAV->m_unorderedAccessView);
	if (FAILED(hResult))
	{
		ERROR_AND_DIE("Could not create the unordered Access View");
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc		=	{ };
	srvDesc.Format								=	writableTextureDesc.Format;
	srvDesc.ViewDimension						=	D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip			=	0;
	srvDesc.Texture2D.MipLevels					=	1;

	hResult = m_d3d11Device->CreateShaderResourceView(textureContainingUAV->m_texture, &srvDesc, &textureContainingUAV->m_shaderResourceView);
	m_loadedTextures.push_back(textureContainingUAV);
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateTextureFromConfig(TextureConfig const& config, Texture*& texture)
{
	HRESULT hResult;
	switch (config.m_type)
	{
		case ResourceType::TEXTURE1D:
		{

			break;
		}
		case ResourceType::TEXTURE2D:
		{
			IntVec2 const& textureDim = (config.m_width == (unsigned int)-1 || config.m_height == (unsigned int)-1) ? m_config.m_window->GetClientDimensions() : IntVec2(config.m_width, config.m_height);

			D3D11_TEXTURE2D_DESC textureDesc = { };
			textureDesc.Usage				=	(D3D11_USAGE)config.m_usageFlag;
			textureDesc.Format				=	(DXGI_FORMAT)config.m_format;
			textureDesc.BindFlags			=	(unsigned int)config.m_bindFlags;
			textureDesc.Width				=	textureDim.x;
			textureDesc.Height				=	textureDim.y;
			textureDesc.MipLevels			=	config.m_mipLevels;
			textureDesc.ArraySize			=	config.m_numOfSlices;
			textureDesc.SampleDesc.Count	=	config.m_multiSampleCount;
			textureDesc.SampleDesc.Quality	=	config.m_multiSampleQuality;

			texture											=	new Texture();
			texture->m_dimensions							=	textureDim;
			D3D11_SUBRESOURCE_DATA initializationData		=	{ };
			initializationData.pSysMem						=	config.m_defaultInitializationData;
			initializationData.SysMemPitch					=	textureDesc.Width * config.m_sizeOfTexelInBytes;
			D3D11_SUBRESOURCE_DATA* defaultPopulatedData	=	config.m_defaultInitializationData == nullptr ? nullptr : &initializationData;
			hResult											=	m_d3d11Device->CreateTexture2D(&textureDesc, defaultPopulatedData, &texture->m_texture);
			GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create the desired Texture2D resource")

			unsigned int const& bindFlag	=	(unsigned int)config.m_bindFlags;
			bool const& isMultiSampled		=	textureDesc.SampleDesc.Count > 1;
			if (bindFlag & (unsigned int)ResourceBindFlag::SHADER_RESOURCE)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc	=	{ };
				shaderResourceViewDesc.Format							=	textureDesc.Format;
				shaderResourceViewDesc.ViewDimension					=	!isMultiSampled ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
				shaderResourceViewDesc.Texture2D.MipLevels				=	textureDesc.MipLevels;
				hResult													=	m_d3d11Device->CreateShaderResourceView(texture->m_texture, &shaderResourceViewDesc, &texture->m_shaderResourceView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create a shader resource view for the desired texture2D resource")
			}

			if (bindFlag & (unsigned int)ResourceBindFlag::UNORDERED_ACCESS)
			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc	=	{ };
				unorderedAccessViewDesc.Format								=	textureDesc.Format;
				unorderedAccessViewDesc.ViewDimension						=	D3D11_UAV_DIMENSION_TEXTURE2D;
				hResult														=	m_d3d11Device->CreateUnorderedAccessView(texture->m_texture, &unorderedAccessViewDesc, &texture->m_unorderedAccessView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create an unordered access view for the desired texture2D resource")
			}

			if (bindFlag & (unsigned int)ResourceBindFlag::RENDER_TARGET)
			{
				D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc	=	{};
				renderTargetViewDesc.Format							=	textureDesc.Format;
				renderTargetViewDesc.ViewDimension					=	!isMultiSampled ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
				hResult												=	m_d3d11Device->CreateRenderTargetView(texture->m_texture, &renderTargetViewDesc, &texture->m_renderTargetView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create a render target view for the desired texture2D resource")
			}

			if (bindFlag & (unsigned int)ResourceBindFlag::DEPTH_STENCIL)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc	=	{ };
				depthStencilViewDesc.Format							=	textureDesc.Format;
				depthStencilViewDesc.ViewDimension					=	!isMultiSampled ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
				hResult												=	m_d3d11Device->CreateDepthStencilView(texture->m_texture, &depthStencilViewDesc, &texture->m_depthStencilView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create a depth stencil view for the desired texture2D resource")
			}

			break;
		}
		case ResourceType::TEXTURE3D:
		{

			break;
		}
		default:
		{
			ERROR_AND_DIE("Invalid texture type provided");
		}
	}

	if (config.m_debugNameSize != 0)
	{
		SetDebugResourceName(texture, config.m_debugNameSize, config.m_debugName);
	}
	m_loadedTextures.push_back(texture);
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateResourceFromConfig(D3D11_ResourceConfig const& config, D3D11_Resource*& out_resource)
{
	HRESULT hResult;
	switch (config.m_type)
	{
		case ResourceType::TEXTURE1D:
		{

			break;
		}
		case ResourceType::TEXTURE2D:
		{
			IntVec2 const& textureDim = (config.m_width == (unsigned int)-1 || config.m_height == (unsigned int)-1) ? m_config.m_window->GetClientDimensions() : IntVec2(config.m_width, config.m_height);

			D3D11_TEXTURE2D_DESC textureDesc	=	{ };
			textureDesc.Usage					=	(D3D11_USAGE)config.m_usageFlag;
			textureDesc.Format					=	(DXGI_FORMAT)config.m_format;
			textureDesc.BindFlags				=	(unsigned int)config.m_bindFlags;
			textureDesc.Width					=	textureDim.x;
			textureDesc.Height					=	textureDim.y;
			textureDesc.MipLevels				=	config.m_mipLevels;
			textureDesc.ArraySize				=	config.m_numOfSlices;
			textureDesc.SampleDesc.Count		=	config.m_multiSampleCount;
			textureDesc.SampleDesc.Quality		=	config.m_multiSampleQuality;

			out_resource					=	new D3D11_Resource();
			out_resource->m_textureDims		=	IntVec3(textureDim.x, textureDim.y, 0);
			out_resource->m_resourceType	=	config.m_type;

			D3D11_SUBRESOURCE_DATA initializationData		=	{ };
			initializationData.pSysMem						=	config.m_defaultInitializationData;
			initializationData.SysMemPitch					=	textureDesc.Width * config.m_sizeOfTexelInBytes;
			D3D11_SUBRESOURCE_DATA* defaultPopulatedData	=	config.m_defaultInitializationData == nullptr ? nullptr : &initializationData;
			hResult											=	m_d3d11Device->CreateTexture2D(&textureDesc, defaultPopulatedData, &out_resource->m_texture2d);
			GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create the desired Texture2D resource")

			unsigned int const& bindFlag	=	(unsigned int)config.m_bindFlags;
			bool const& isMultiSampled		=	textureDesc.SampleDesc.Count > 1;
			if (bindFlag & (unsigned int)ResourceBindFlag::SHADER_RESOURCE)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc	=	{ };
				shaderResourceViewDesc.Format							=	textureDesc.Format != DXGI_FORMAT_R32_TYPELESS ? textureDesc.Format : DXGI_FORMAT_R32_FLOAT;
				shaderResourceViewDesc.ViewDimension					=	!isMultiSampled ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
				shaderResourceViewDesc.Texture2D.MipLevels				=	textureDesc.MipLevels;
				hResult													=	m_d3d11Device->CreateShaderResourceView(out_resource->m_texture2d, &shaderResourceViewDesc, &out_resource->m_shaderResourceView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create a shader resource view for the desired texture2D resource")
			}

			if (bindFlag & (unsigned int)ResourceBindFlag::UNORDERED_ACCESS)
			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc	=	{ };
				unorderedAccessViewDesc.Format								=	textureDesc.Format;
				unorderedAccessViewDesc.ViewDimension						=	D3D11_UAV_DIMENSION_TEXTURE2D;
				hResult														=	m_d3d11Device->CreateUnorderedAccessView(out_resource->m_texture2d, &unorderedAccessViewDesc, &out_resource->m_unorderedAccessView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create an unordered access view for the desired texture2D resource")
			}

			if (bindFlag & (unsigned int)ResourceBindFlag::RENDER_TARGET)
			{
				D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc	=	{};
				renderTargetViewDesc.Format							=	textureDesc.Format;
				renderTargetViewDesc.ViewDimension					=	!isMultiSampled ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
				hResult												=	m_d3d11Device->CreateRenderTargetView(out_resource->m_texture2d, &renderTargetViewDesc, &out_resource->m_renderTargetView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create a render target view for the desired texture2D resource")
			}

			if (bindFlag & (unsigned int)ResourceBindFlag::DEPTH_STENCIL)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc	=	{ };
				depthStencilViewDesc.Format							=	textureDesc.Format == DXGI_FORMAT_R32_TYPELESS ? DXGI_FORMAT_D32_FLOAT : textureDesc.Format;
				depthStencilViewDesc.ViewDimension					=	!isMultiSampled ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
				hResult												=	m_d3d11Device->CreateDepthStencilView(out_resource->m_texture2d, &depthStencilViewDesc, &out_resource->m_depthStencilView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create a depth stencil view for the desired texture2D resource");
				if (config.m_canDepthBeReadOnly)
				{
					depthStencilViewDesc.Flags	=	D3D11_DSV_READ_ONLY_DEPTH;
					hResult				=	m_d3d11Device->CreateDepthStencilView(out_resource->m_texture2d, &depthStencilViewDesc, &out_resource->m_readOnlyDepthStencilView);
					GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Could not create a read only depth stencil view for the desired texture2D resource")
				}
			}

			break;
		}
		
		case ResourceType::TEXTURE3D:
		{
		
			break;
		}

		case ResourceType::STRUCTURED_BUFFER:
		{
			D3D11_BUFFER_DESC structuredBufferDesc		=	{ };
			structuredBufferDesc.ByteWidth				=	config.m_numOfElements * config.m_elementStride;
			structuredBufferDesc.MiscFlags				=	D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			structuredBufferDesc.StructureByteStride	=	config.m_elementStride;
			structuredBufferDesc.BindFlags				=	D3D11_BIND_SHADER_RESOURCE;

			switch (config.m_usageFlag)
			{
			case ResourceUsage::GPU_READ:
			{
				structuredBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
				break;
			}
			case ResourceUsage::GPU_READ_GPU_WRITE:
			{
				structuredBufferDesc.Usage		 =	D3D11_USAGE_DEFAULT;
				structuredBufferDesc.BindFlags	|=	D3D11_BIND_UNORDERED_ACCESS;
				break;
			}
			case ResourceUsage::GPU_READ_CPU_WRITE:
			{
				structuredBufferDesc.Usage				=	D3D11_USAGE_DYNAMIC;
				structuredBufferDesc.CPUAccessFlags		=	D3D11_CPU_ACCESS_WRITE;
				break;
			}
			case ResourceUsage::GPU_READ_CPU_READ_GPU_WRITE_CPU_WRITE:
			{
				ERROR_AND_DIE("Don't create a structured buffer that is staging!!!");
				// structuredBufferDesc.Usage = D3D11_USAGE_STAGING;
				// structuredBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
				// break;
			}
			case ResourceUsage::GPU_READ_GPU_WRITE_CPU_READ:
			{
				structuredBufferDesc.Usage				=	D3D11_USAGE_STAGING;
				structuredBufferDesc.CPUAccessFlags		=	D3D11_CPU_ACCESS_READ;
				structuredBufferDesc.BindFlags			=	0;
				break;
			}
			default:
			{
				ERROR_AND_DIE("Please provide a valid buffer usage when creating the structured buffer");
			}
			}

			out_resource					=	new D3D11_Resource();
			out_resource->m_numOfElements	=	config.m_numOfElements;
			out_resource->m_elementStride	=	config.m_elementStride;

			if (config.m_defaultInitializationData)
			{
				D3D11_SUBRESOURCE_DATA initializationData	=	{ };
				initializationData.pSysMem					=	config.m_defaultInitializationData;
				hResult										=	m_d3d11Device->CreateBuffer(&structuredBufferDesc, &initializationData, &out_resource->m_buffer);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a structured buffer!!!");
			}
			else
			{
				hResult = m_d3d11Device->CreateBuffer(&structuredBufferDesc, NULL, &out_resource->m_buffer);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a structured buffer!!!");
			}
			

			if (structuredBufferDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
				if (config.m_isStandard)
				{
					srvDesc.Format = DXGI_FORMAT(config.m_format);
				}
				else
				{
					srvDesc.Format = DXGI_FORMAT_UNKNOWN;
				}
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				srvDesc.Buffer.FirstElement = 0;
				srvDesc.Buffer.NumElements	= config.m_numOfElements;

				hResult = m_d3d11Device->CreateShaderResourceView(out_resource->m_buffer, &srvDesc, &out_resource->m_shaderResourceView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a structured buffers shader resource view!!!");
			}
			if (structuredBufferDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };
				if (config.m_isStandard)
				{
					uavDesc.Format = DXGI_FORMAT(config.m_format);
				}
				else
				{
					uavDesc.Format = DXGI_FORMAT_UNKNOWN;
				}
				uavDesc.ViewDimension			=	D3D11_UAV_DIMENSION_BUFFER;
				uavDesc.Buffer.FirstElement		=	0;
				uavDesc.Buffer.NumElements		=	config.m_numOfElements;
				uavDesc.Buffer.Flags			=	(unsigned int)config.m_uavResourceFlag;

				hResult = m_d3d11Device->CreateUnorderedAccessView(out_resource->m_buffer, &uavDesc, &out_resource->m_unorderedAccessView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a structured buffers unordered access view!!!");
			}
			
			out_resource->m_resourceType = config.m_type;
			break;
		}

		case ResourceType::RAW_BUFFER:
		{
			constexpr int RAW_BUFFER_STRIDE		=	4;
			D3D11_BUFFER_DESC rawBufferDesc		=	{ };
			rawBufferDesc.ByteWidth				=	config.m_numOfElements * RAW_BUFFER_STRIDE;
			rawBufferDesc.MiscFlags				=	D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
			rawBufferDesc.StructureByteStride	=	0;
			rawBufferDesc.BindFlags				|=	D3D11_BIND_SHADER_RESOURCE;


			switch (config.m_usageFlag)
			{
			case ResourceUsage::GPU_READ:
			{
				rawBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
				break;
			}
			case ResourceUsage::GPU_READ_GPU_WRITE:
			{
				rawBufferDesc.Usage			=	D3D11_USAGE_DEFAULT;
				rawBufferDesc.BindFlags		|=	D3D11_BIND_UNORDERED_ACCESS;
				break;
			}
			case ResourceUsage::GPU_READ_CPU_WRITE:
			{
				rawBufferDesc.Usage				=	D3D11_USAGE_DYNAMIC;
				rawBufferDesc.CPUAccessFlags	=	D3D11_CPU_ACCESS_WRITE;
				break;
			}
			case ResourceUsage::GPU_READ_CPU_READ_GPU_WRITE_CPU_WRITE:
			{
				ERROR_AND_DIE("Don't create a raw buffer that is staging!!!");
				// structuredBufferDesc.Usage = D3D11_USAGE_STAGING;
				// structuredBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
				// break;
			}
			default:
			{
				ERROR_AND_DIE("Please provide a valid buffer usage when creating the raw buffer");
			}
			}

			out_resource					=	new D3D11_Resource();
			out_resource->m_numOfElements	=	config.m_numOfElements;
			out_resource->m_elementStride	=	RAW_BUFFER_STRIDE;

			if (config.m_defaultInitializationData)
			{
				D3D11_SUBRESOURCE_DATA initializationData	=	{ };
				initializationData.pSysMem					=	config.m_defaultInitializationData;
				hResult										=	m_d3d11Device->CreateBuffer(&rawBufferDesc, &initializationData, &out_resource->m_buffer);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a raw buffer!!!");
			}
			else
			{
				hResult = m_d3d11Device->CreateBuffer(&rawBufferDesc, NULL, &out_resource->m_buffer);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a raw buffer!!!");
			}

			if (rawBufferDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc		=	{ };
				srvDesc.BufferEx.Flags						=	D3D11_BUFFEREX_SRV_FLAG_RAW;
				srvDesc.BufferEx.FirstElement				=	0;
				srvDesc.BufferEx.NumElements				=	config.m_numOfElements;
				srvDesc.ViewDimension						=	D3D11_SRV_DIMENSION_BUFFEREX;
				srvDesc.Format								=	DXGI_FORMAT_R32_TYPELESS;

				hResult = m_d3d11Device->CreateShaderResourceView(out_resource->m_buffer, &srvDesc, &out_resource->m_shaderResourceView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a raw buffers shader resource view!!!");
			}
			if (rawBufferDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };

				uavDesc.Format					=	DXGI_FORMAT_R32_TYPELESS;
				uavDesc.ViewDimension			=	D3D11_UAV_DIMENSION_BUFFER;
				uavDesc.Buffer.FirstElement		=	0;
				uavDesc.Buffer.NumElements		=	config.m_numOfElements;
				uavDesc.Buffer.Flags			=	D3D11_BUFFER_UAV_FLAG_RAW;

				hResult = m_d3d11Device->CreateUnorderedAccessView(out_resource->m_buffer, &uavDesc, &out_resource->m_unorderedAccessView);
				GUARANTEE_OR_DIE(SUCCEEDED(hResult), "Something went wrong while creating a raw buffers unordered access view!!!");
			}
			break;
		}

		default:
		{
			ERROR_AND_DIE("Invalid resource type provided");
		}
	}

	if (config.m_debugNameSize != 0)
	{
		SetDebugResourceName(out_resource, config.m_debugNameSize, config.m_debugName);
	}
	m_loadedResources.push_back(out_resource);
}


//--------------------------------------------------------------------------------------------------
D3D11_Resource* Renderer::CreateTextureResourceFromFile(char const* imageFilePath, char const* debugResourceName /*= nullptr*/, unsigned int debugResourceNameSize /*= 0*/)
{
	Image* newImage = new Image(imageFilePath);
	IntVec2 textureDim = newImage->GetDimensions();

	D3D11_ResourceConfig textureResourceConfig			=	{ };
	textureResourceConfig.m_type						=	ResourceType::TEXTURE2D;
	textureResourceConfig.m_format						=	ResourceViewFormat::DXGI_FORMAT_R8G8B8A8_UNORM;
	textureResourceConfig.m_width						=	textureDim.x;
	textureResourceConfig.m_height						=	textureDim.y;
	textureResourceConfig.m_bindFlags					=	ResourceBindFlag::SHADER_RESOURCE;
	textureResourceConfig.m_usageFlag					=	ResourceUsage::GPU_READ;
	textureResourceConfig.m_defaultInitializationData	=	(void*)newImage->GetRawData();
	textureResourceConfig.m_sizeOfTexelInBytes			=	4;
	textureResourceConfig.m_debugName					=	debugResourceName;
	textureResourceConfig.m_debugNameSize				=	debugResourceNameSize;

	D3D11_Resource* newTextureResource = nullptr;
	CreateResourceFromConfig(textureResourceConfig, newTextureResource);
	return newTextureResource;
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateDepthResource(D3D11_Resource*& depthResource, char const* debugResourceName /*= "None"*/, unsigned int debugResourceNameSize /*= 0*/, bool canBeReadOnly /*= false*/, IntVec2 const& textureDims /*= IntVec2(-1, -1)*/)
{
	D3D11_ResourceConfig depthResourceConfig	=	{ };
	depthResourceConfig.m_debugName				=	debugResourceName;
	depthResourceConfig.m_debugNameSize			=	debugResourceNameSize;
	depthResourceConfig.m_width					=	textureDims.x;
	depthResourceConfig.m_height				=	textureDims.y;
	depthResourceConfig.m_bindFlags				=	ResourceBindFlag((unsigned char)ResourceBindFlag::DEPTH_STENCIL | (unsigned char)ResourceBindFlag::SHADER_RESOURCE);
	depthResourceConfig.m_usageFlag				=	ResourceUsage::GPU_READ_GPU_WRITE;
	depthResourceConfig.m_format				=	ResourceViewFormat::DXGI_FORMAT_R32_TYPELESS;
	depthResourceConfig.m_canDepthBeReadOnly	=	canBeReadOnly;

	CreateResourceFromConfig(depthResourceConfig, depthResource);
}


//--------------------------------------------------------------------------------------------------
void Renderer::CreateRenderTargetResource(D3D11_Resource*& renderTargetResource, bool isWritable, char const* debugResourceName /*= nullptr*/, unsigned int debugResourceNameSize /*= 0*/, IntVec2 const& textureDims /*= IntVec2(-1, -1)*/)
{
	D3D11_ResourceConfig renderTargetResourceConfig		=	{ };
	renderTargetResourceConfig.m_debugName				=	debugResourceName;
	renderTargetResourceConfig.m_debugNameSize			=	debugResourceNameSize;
	renderTargetResourceConfig.m_width					=	textureDims.x;
	renderTargetResourceConfig.m_height					=	textureDims.y;
	renderTargetResourceConfig.m_bindFlags				=	ResourceBindFlag((unsigned char)ResourceBindFlag::RENDER_TARGET | (unsigned char)ResourceBindFlag::SHADER_RESOURCE);
	renderTargetResourceConfig.m_usageFlag				=	ResourceUsage::GPU_READ_GPU_WRITE;
	renderTargetResourceConfig.m_format					=	ResourceViewFormat::DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetResourceConfig.m_sizeOfTexelInBytes		=	16;
	if (isWritable)
	{
		renderTargetResourceConfig.m_bindFlags = ResourceBindFlag((unsigned char)(renderTargetResourceConfig.m_bindFlags) | (unsigned char)ResourceBindFlag::UNORDERED_ACCESS);
	}

	CreateResourceFromConfig(renderTargetResourceConfig, renderTargetResource);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindRenderTargetOnly(Texture* renderTarget)
{
	if (!renderTarget)
	{
		m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
		return;
	}
	m_d3d11DeviceContext->OMSetRenderTargets(1, &renderTarget->m_renderTargetView, nullptr);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindRenderAndDepthTargetViewsToThePipeline(Texture* renderTexture, Texture* depthTexture, D3D11_Buffer* buffer)
{
	UnbindRenderAndDepthTargets();

	D3D11_VIEWPORT viewport		=	{};
	viewport.MaxDepth			=	1;

	IntVec2 windowDim		=	m_config.m_window->GetClientDimensions();
	float	clientWidth		=	(float)windowDim.x;
	float	clientHeight	=	(float)windowDim.y;

	if (buffer)
	{
		std::vector<ID3D11UnorderedAccessView*> uavs;
		std::vector<unsigned int> appendConsumeBufferOffsets;
		uavs.resize(1);
		appendConsumeBufferOffsets.resize(1);

		uavs[0]							=	buffer->m_unorderedAccessView;
		appendConsumeBufferOffsets[0]	=	(unsigned int)-1;

		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, 1, 1, uavs.data(), appendConsumeBufferOffsets.data());
	}
	else if (!renderTexture && !depthTexture)
	{
		m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
		viewport.Width		=	clientWidth;
		viewport.Height		=	clientHeight;
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
	}
	else if (!renderTexture && depthTexture)
	{
		IntVec2 const& depthTextureDims		=	depthTexture->GetDimensions();
		viewport.Width						=	float(depthTextureDims.x);
		viewport.Height						=	float(depthTextureDims.y);
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
		m_d3d11DeviceContext->OMSetRenderTargets(0, nullptr, depthTexture->m_depthStencilView);
	}
	else if (renderTexture && !depthTexture)
	{
		IntVec2 const& renderTextureDims	=	renderTexture->GetDimensions();
		viewport.Width						=	float(renderTextureDims.x);
		viewport.Height						=	float(renderTextureDims.y);
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
		m_d3d11DeviceContext->OMSetRenderTargets(1, &renderTexture->m_renderTargetView, nullptr);
	}
	else if (renderTexture && depthTexture)
	{
		IntVec2 const& renderTargetTextureDims	=	renderTexture->GetDimensions();
		viewport.Width							=	float(renderTargetTextureDims.x);
		viewport.Height							=	float(renderTargetTextureDims.y);
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
		m_d3d11DeviceContext->OMSetRenderTargets(1, &renderTexture->m_renderTargetView, depthTexture->m_depthStencilView);
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindRenderAndDepthTargetViewsToThePipeline(unsigned int numOfRenderTargetViews, Texture* const* renderTargets /*= nullptr*/, Texture* depthTarget /*= nullptr*/, bool isDepthReadOnly /*= false*/)
{
	std::vector<ID3D11RenderTargetView*> renderTargetViews;
	renderTargetViews.resize(numOfRenderTargetViews);
	for (unsigned int arrayIndex = 0; arrayIndex < numOfRenderTargetViews; ++arrayIndex)
	{
		renderTargetViews[arrayIndex] = renderTargets[arrayIndex]->m_renderTargetView;
	}

	if (depthTarget)
	{
		m_d3d11DeviceContext->OMSetRenderTargets(numOfRenderTargetViews, renderTargetViews.data(), depthTarget->m_depthStencilView);
		if (isDepthReadOnly)
		{
			m_d3d11DeviceContext->OMSetRenderTargets(numOfRenderTargetViews, renderTargetViews.data(), depthTarget->m_readOnlyDepthStencilView);
		}
		return;
	}

	m_d3d11DeviceContext->OMSetRenderTargets(numOfRenderTargetViews, renderTargetViews.data(), nullptr);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindRenderAndDepthTargetViews(Texture* renderTarget /*= nullptr*/, Texture* depthStencil /*= nullptr*/, bool isDepthReadOnly /*= false*/)
{
	UnbindRenderAndDepthTargets();

	D3D11_VIEWPORT viewport		=	{};
	viewport.MaxDepth			=	1;

	IntVec2 windowDim		=	m_config.m_window->GetClientDimensions();
	float	clientWidth		=	(float)windowDim.x;
	float	clientHeight	=	(float)windowDim.y;

	if (!renderTarget && !depthStencil)
	{
		m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
		viewport.Width		=	clientWidth;
		viewport.Height		=	clientHeight;
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
	}
	else if (!renderTarget && depthStencil)
	{
		IntVec2 const& depthTextureDims		=	depthStencil->GetDimensions();
		viewport.Width						=	float(depthTextureDims.x);
		viewport.Height						=	float(depthTextureDims.y);
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
		if (!isDepthReadOnly)
		{
			m_d3d11DeviceContext->OMSetRenderTargets(0, nullptr, depthStencil->m_depthStencilView);
		}
		else
		{
			m_d3d11DeviceContext->OMSetRenderTargets(0, nullptr, depthStencil->m_readOnlyDepthStencilView);
		}
	}
	else if (renderTarget && !depthStencil)
	{
		IntVec2 const& renderTextureDims	=	renderTarget->GetDimensions();
		viewport.Width						=	float(renderTextureDims.x);
		viewport.Height						=	float(renderTextureDims.y);
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
		m_d3d11DeviceContext->OMSetRenderTargets(1, &renderTarget->m_renderTargetView, nullptr);
	}
	else if (renderTarget && depthStencil)
	{
		IntVec2 const& renderTargetTextureDims	=	renderTarget->GetDimensions();
		viewport.Width							=	float(renderTargetTextureDims.x);
		viewport.Height							=	float(renderTargetTextureDims.y);
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
		if (!isDepthReadOnly)
		{
			m_d3d11DeviceContext->OMSetRenderTargets(1, &renderTarget->m_renderTargetView, depthStencil->m_depthStencilView);
		}
		else
		{
			m_d3d11DeviceContext->OMSetRenderTargets(1, &renderTarget->m_renderTargetView, depthStencil->m_readOnlyDepthStencilView);
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindRenderAndDepthResources(D3D11_Resource* renderTarget /*= nullptr*/, D3D11_Resource* depthResource /*= nullptr*/, bool isDepthReadOnly /*= false*/, bool bindDefault /*= false*/)
{
	D3D11_VIEWPORT viewport		=	{};
	viewport.MaxDepth			=	1;

	IntVec2 windowDim		=	m_config.m_window->GetClientDimensions();
	float	clientWidth		=	(float)windowDim.x;
	float	clientHeight	=	(float)windowDim.y;

	if (!renderTarget && !depthResource)
	{
		viewport.Width		=	clientWidth;
		viewport.Height		=	clientHeight;
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);

		if (!bindDefault)
		{
			m_d3d11DeviceContext->OMSetRenderTargets(0, 0, 0);
		}
		else
		{
			m_d3d11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
		}
	}
	else if (!renderTarget && depthResource)
	{
		IntVec3 const& textureDims			=	depthResource->m_textureDims;
		IntVec2 const& depthTextureDims		=	IntVec2(textureDims.x, textureDims.y);
		viewport.Width						=	float(depthTextureDims.x);
		viewport.Height						=	float(depthTextureDims.y);
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
		if (!isDepthReadOnly)
		{
			m_d3d11DeviceContext->OMSetRenderTargets(0, nullptr, depthResource->m_depthStencilView);
		}
		else
		{
			m_d3d11DeviceContext->OMSetRenderTargets(0, nullptr, depthResource->m_readOnlyDepthStencilView);
		}
	}
	else if (renderTarget && !depthResource)
	{
		IntVec3 const& textureDims			=	renderTarget->m_textureDims;
		IntVec2 const& renderTextureDims	=	IntVec2(textureDims.x, textureDims.y);
		viewport.Width						=	float(renderTextureDims.x);
		viewport.Height						=	float(renderTextureDims.y);
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
		m_d3d11DeviceContext->OMSetRenderTargets(1, &renderTarget->m_renderTargetView, nullptr);
	}
	else if (renderTarget && depthResource)
	{
		IntVec3 const& textureDims			=	renderTarget->m_textureDims;
		IntVec2 const& renderTextureDims	=	IntVec2(textureDims.x, textureDims.y);
		viewport.Width						=	float(renderTextureDims.x);
		viewport.Height						=	float(renderTextureDims.y);
		m_d3d11DeviceContext->RSSetViewports(1, &viewport);
		if (!isDepthReadOnly)
		{
			m_d3d11DeviceContext->OMSetRenderTargets(1, &renderTarget->m_renderTargetView, depthResource->m_depthStencilView);
		}
		else
		{
			m_d3d11DeviceContext->OMSetRenderTargets(1, &renderTarget->m_renderTargetView, depthResource->m_readOnlyDepthStencilView);
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindRenderAndDepthResources(unsigned int numOfRenderTargetViews, D3D11_Resource* const* renderTargets /*= nullptr*/, D3D11_Resource* depthTarget /*= nullptr*/, bool isDepthReadOnly /*= false*/)
{
	std::vector<ID3D11RenderTargetView*> renderTargetViews;
	renderTargetViews.resize(numOfRenderTargetViews);
	for (unsigned int arrayIndex = 0; arrayIndex < numOfRenderTargetViews; ++arrayIndex)
	{
		renderTargetViews[arrayIndex] = renderTargets[arrayIndex]->m_renderTargetView;
	}

	if (depthTarget)
	{
		if (isDepthReadOnly)
		{
			m_d3d11DeviceContext->OMSetRenderTargets(numOfRenderTargetViews, renderTargetViews.data(), depthTarget->m_readOnlyDepthStencilView);
			return;
		}
		m_d3d11DeviceContext->OMSetRenderTargets(numOfRenderTargetViews, renderTargetViews.data(), depthTarget->m_depthStencilView);
		return;
	}

	m_d3d11DeviceContext->OMSetRenderTargets(numOfRenderTargetViews, renderTargetViews.data(), nullptr);
}


//--------------------------------------------------------------------------------------------------
void Renderer::UnbindRenderAndDepthTargets()
{
	m_d3d11DeviceContext->OMSetRenderTargets(0, 0, 0);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindWritableBuffersToThePipeline(D3D11_Buffer* const* writableBuffers, unsigned int numOfUAVs, unsigned int uavStartSlot /*= 0*/, Texture const* renderTargetTexture /*= nullptr*/, Texture const* depthStencilTexture /*= nullptr*/, bool isReadOnlyDepth /*= false*/)
{
	std::vector<ID3D11UnorderedAccessView*> emptyUAVs;
	emptyUAVs.resize(numOfUAVs, NULL);
	std::vector<ID3D11UnorderedAccessView*> uavs;
	std::vector<unsigned int> appendConsumeBufferOffsets;
	uavs.resize(numOfUAVs);
	appendConsumeBufferOffsets.resize(numOfUAVs);


	if (renderTargetTexture)
	{
		ID3D11RenderTargetView* currentArrayOfRenderTargets[1]		=	{ renderTargetTexture->m_renderTargetView };
		ID3D11DepthStencilView* currentArrayOfDepthStencilViews[1]	=	{ nullptr }; 
		if (!isReadOnlyDepth)
		{
			currentArrayOfDepthStencilViews[0] = { depthStencilTexture->m_depthStencilView };
		}
		else
		{
			currentArrayOfDepthStencilViews[0] = { depthStencilTexture->m_readOnlyDepthStencilView };
		}
		// ID3D11UnorderedAccessView** currentArrayOfUAVs = nullptr;
		// m_d3d11DeviceContext->OMGetRenderTargets(currentNumOfRenderTargets, currentArrayOfRenderTargets, currentArrayOfDepthStencilViews);

		if (writableBuffers == nullptr)
		{
			m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(1, currentArrayOfRenderTargets, *currentArrayOfDepthStencilViews, uavStartSlot, numOfUAVs, emptyUAVs.data(), appendConsumeBufferOffsets.data());
			return;
		}


		for (unsigned int uavIndex = 0; uavIndex < numOfUAVs; ++uavIndex)
		{
			uavs[uavIndex]							=	writableBuffers[uavIndex]->m_unorderedAccessView;
			appendConsumeBufferOffsets[uavIndex]	=	(unsigned int)0;
		}


		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(1, currentArrayOfRenderTargets, *currentArrayOfDepthStencilViews, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffsets.data());
		return;
	}

	if (writableBuffers == nullptr)
	{
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, uavStartSlot, numOfUAVs, emptyUAVs.data(), appendConsumeBufferOffsets.data());
		return;
	}


	for (unsigned int uavIndex = 0; uavIndex < numOfUAVs; ++uavIndex)
	{
		uavs[uavIndex]							=	writableBuffers[uavIndex]->m_unorderedAccessView;
		appendConsumeBufferOffsets[uavIndex]	=	(unsigned int)0;
	}


	m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffsets.data());
	m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffsets.data());
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindWritableBufferToThePipeline(D3D11_Buffer* writableBuffer, unsigned int uavStartSlot)
{
	unsigned int appendConsumeBufferOffset[1] = { 0 };
	if (writableBuffer)
	{
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, uavStartSlot, 1, &writableBuffer->m_unorderedAccessView, appendConsumeBufferOffset);
		return;
	}
	std::vector<ID3D11UnorderedAccessView*> emptyUAVs;
	emptyUAVs.resize(1, NULL);
	m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, uavStartSlot, 1, emptyUAVs.data(), appendConsumeBufferOffset);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindReadOnlyBufferToThePipeleine(D3D11_Buffer* readableBuffer, unsigned int startSlot, unsigned int numOfSRVs, unsigned int startOffset)
{
	// TODO: Make it use multiple buffers
	numOfSRVs	= 1;
	startOffset = 0;
	if (readableBuffer)
	{
		m_d3d11DeviceContext->CSSetShaderResources(startSlot, 1, &readableBuffer->m_shaderResourceView);
		return;
	}
	ID3D11ShaderResourceView* emptySRV[1] = { NULL };
	m_d3d11DeviceContext->CSSetShaderResources(startSlot, 1, emptySRV);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindWritableBufferToTheComputeShader(D3D11_Buffer* writableBuffer, unsigned int startSlot, unsigned int numOfUAVs, unsigned int startOffset)
{
	// TODO: Make it use multiple buffers
	numOfUAVs		=	1;
	startOffset		=	0;
	if (writableBuffer)
	{
		m_d3d11DeviceContext->CSSetUnorderedAccessViews(startSlot, 1, &writableBuffer->m_unorderedAccessView, nullptr);
		return;
	}
	ID3D11UnorderedAccessView* emptyUAV[1] = { NULL };
	m_d3d11DeviceContext->CSSetUnorderedAccessViews(startSlot, 1, emptyUAV, NULL);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindWritableTextureToTheComputeShader(Texture* writableTexture, unsigned int startSlot, unsigned int numOfUAVs, unsigned int startOffset)
{
	numOfUAVs		=	1;
	startOffset		=	0;
	if (writableTexture)
	{
		m_d3d11DeviceContext->CSSetUnorderedAccessViews(startSlot, 1, &writableTexture->m_unorderedAccessView, nullptr);
		return;
	}
	ID3D11UnorderedAccessView* emptyUAV[1] = { NULL };
	m_d3d11DeviceContext->CSSetUnorderedAccessViews(startSlot, 1, emptyUAV, NULL);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindWritableBuffersToTheComputeShader(D3D11_Buffer* const* writableBuffers, unsigned int numOfUAVs, unsigned int startSlot, unsigned int const* appendConsumeOffsets)
{
	GUARANTEE_OR_DIE(appendConsumeOffsets == nullptr, "Do not support append and consume buffers just yet");
	
	std::vector<unsigned int> appendConsumeBufferOffsets;
	appendConsumeBufferOffsets.resize(numOfUAVs);

	std::vector<ID3D11UnorderedAccessView*> uavs;
	uavs.resize(numOfUAVs);
	for (unsigned int uavIndex = 0; uavIndex < numOfUAVs; ++uavIndex)
	{
		uavs[uavIndex]							=	writableBuffers[uavIndex]->m_unorderedAccessView;
		appendConsumeBufferOffsets[uavIndex]	=	(unsigned int)0;
	}
	m_d3d11DeviceContext->CSSetUnorderedAccessViews(startSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffsets.data());
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindWritableTexturesToTheComputeShader(Texture* const* writableTextures, unsigned int numOfUAVs, unsigned int startSlot, unsigned int const* appendConsumeOffsets)
{
	GUARANTEE_OR_DIE(appendConsumeOffsets == nullptr, "Do not support append and consume buffers just yet");

	std::vector<unsigned int> appendConsumeBufferOffsets;
	appendConsumeBufferOffsets.resize(numOfUAVs);

	std::vector<ID3D11UnorderedAccessView*> uavs;
	uavs.resize(numOfUAVs);
	
	for (unsigned int uavIndex = 0; uavIndex < numOfUAVs; ++uavIndex)
	{
		uavs[uavIndex]							=	writableTextures[uavIndex]->m_unorderedAccessView;
		appendConsumeBufferOffsets[uavIndex]	=	(unsigned int)0;
	}
	m_d3d11DeviceContext->CSSetUnorderedAccessViews(startSlot, numOfUAVs, uavs.data(), appendConsumeOffsets);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindWritableResourcesToComputeShader(D3D11_Resource* const* writableResources, unsigned int numOfUAVs /*= 1*/, unsigned int startSlot /*= 0*/, unsigned int const* appendConsumeOffsets /*= nullptr*/)
{
	GUARANTEE_OR_DIE(appendConsumeOffsets == nullptr, "Do not support append and consume buffers just yet");

	std::vector<unsigned int> appendConsumeBufferOffsets;
	appendConsumeBufferOffsets.reserve(numOfUAVs);
	std::vector<ID3D11UnorderedAccessView*> uavs;
	uavs.reserve(numOfUAVs);

	for (unsigned int uavIndex = 0; uavIndex < numOfUAVs; ++uavIndex)
	{
		uavs.emplace_back(writableResources[uavIndex]->m_unorderedAccessView);
		appendConsumeBufferOffsets.emplace_back((unsigned int)0);
	}
	m_d3d11DeviceContext->CSSetUnorderedAccessViews(startSlot, numOfUAVs, uavs.data(), appendConsumeOffsets);
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindReadableBuffersToTheComputeShader(D3D11_Buffer* const* readOnlyBuffers, unsigned int numOfSRVs, unsigned int startSlot)
{
	std::vector<ID3D11ShaderResourceView*> srvs;
	srvs.resize(numOfSRVs);
	for (unsigned int srvIndex = 0; srvIndex < numOfSRVs; ++srvIndex)
	{
		srvs[srvIndex] = readOnlyBuffers[srvIndex]->m_shaderResourceView;
	}
	m_d3d11DeviceContext->CSSetShaderResources(startSlot, numOfSRVs, srvs.data());
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindReadableResources(D3D11_Resource* const* readableResourcesToBind, unsigned int numOfSRVs, unsigned int startSlot, BindingLocation bindingLocation)
{
	std::vector<ID3D11ShaderResourceView*> srvs;
	srvs.reserve(numOfSRVs);
	for (unsigned int srvIndex = 0; srvIndex < numOfSRVs; ++srvIndex)
	{
		D3D11_Resource const* currentReadableResource = readableResourcesToBind[srvIndex];
		if (currentReadableResource)
		{
			srvs.emplace_back(currentReadableResource->m_shaderResourceView);
		}
		else
		{
			srvs.emplace_back(m_defaultTexture->m_shaderResourceView);
		}
	}
	
	switch (bindingLocation)
	{
		case BindingLocation::VERTEX_SHADER:
		{
			m_d3d11DeviceContext->VSSetShaderResources(startSlot, numOfSRVs, srvs.data());
			break;
		}
		case BindingLocation::PIXEL_SHADER:
		{
			m_d3d11DeviceContext->PSSetShaderResources(startSlot, numOfSRVs, srvs.data());
			break;
		}
		case BindingLocation::COMPUTE_SHADER:
		{
			m_d3d11DeviceContext->CSSetShaderResources(startSlot, numOfSRVs, srvs.data());
			break;
		}
		default:
		{
			ERROR_AND_DIE("Please provide a valid binding location");
			break;
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindReadableTexturesToTheComputeShader(Texture* const* readableTextures, unsigned int numOfSRVs, unsigned int startSlot)
{
	std::vector<ID3D11ShaderResourceView*> srvs;
	srvs.resize(numOfSRVs);
	for (unsigned int srvIndex = 0; srvIndex < numOfSRVs; ++srvIndex)
	{
		srvs[srvIndex] = readableTextures[srvIndex]->m_shaderResourceView;
	}
	m_d3d11DeviceContext->CSSetShaderResources(startSlot, numOfSRVs, srvs.data());
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindReadableTexture(Texture* const* readableTextures, unsigned int numOfSRVs, unsigned int startSlot, BindingLocation bindingLocation)
{
	std::vector<ID3D11ShaderResourceView*> srvs;
	srvs.resize(numOfSRVs);
	for (unsigned int srvIndex = 0; srvIndex < numOfSRVs; ++srvIndex)
	{
		srvs[srvIndex] = readableTextures[srvIndex]->m_shaderResourceView;
	}

	switch (bindingLocation)
	{
	case BindingLocation::VERTEX_SHADER:
	{
		m_d3d11DeviceContext->VSSetShaderResources(startSlot, numOfSRVs, srvs.data());
		break;
	}
	case BindingLocation::PIXEL_SHADER:
	{
		m_d3d11DeviceContext->PSSetShaderResources(startSlot, numOfSRVs, srvs.data());
		break;
	}
	case BindingLocation::COMPUTE_SHADER:
	{
		m_d3d11DeviceContext->CSSetShaderResources(startSlot, numOfSRVs, srvs.data());
		break;
	}
	default:
		break;
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindUAVsRenderAndDepthTargets(unsigned int numOfUAVs, unsigned int uavStartSlot, Texture* const* uavTexture, unsigned int numOfRenderTargets /*= 1*/, Texture* const* renderTargets /*= nullptr*/, Texture* depthTexture /*= nullptr*/, bool isDepthReadOnly /*= false*/)
{
	std::vector<ID3D11UnorderedAccessView*> uavs;
	std::vector<unsigned int> appendConsumeBufferOffset;
	uavs.reserve(numOfUAVs);
	appendConsumeBufferOffset.reserve(numOfUAVs);
	for (unsigned int uavIndex = 0; uavIndex < numOfUAVs; ++uavIndex)
	{
		uavs.emplace_back(uavTexture[uavIndex]->m_unorderedAccessView);
		appendConsumeBufferOffset.emplace_back(0);
	}

	if (!numOfRenderTargets)
	{
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
		return;
	}

	ID3D11DepthStencilView* depthStencilView = !depthTexture ? m_depthStencilView : depthTexture->m_depthStencilView;
	if (renderTargets == nullptr)
	{
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_renderTargetView, depthStencilView, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
		return;
	}

	std::vector<ID3D11RenderTargetView*> renderTargetViews;
	renderTargetViews.reserve(numOfRenderTargets);
	for (unsigned int arrayIndex = 0; arrayIndex < numOfRenderTargets; ++arrayIndex)
	{
		Texture* currentRenderTargetTexture = renderTargets[arrayIndex];
		if (currentRenderTargetTexture)
		{
			renderTargetViews.emplace_back(currentRenderTargetTexture->m_renderTargetView);
			continue;
		}
		renderTargetViews.emplace_back(nullptr);
	}

	if (!isDepthReadOnly)
	{
		depthStencilView = !depthTexture ? m_depthStencilView : depthTexture->m_depthStencilView;
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(numOfRenderTargets, renderTargetViews.data(), depthStencilView, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
	}
	else
	{
		depthStencilView = !depthTexture ? m_depthStencilView : depthTexture->m_readOnlyDepthStencilView; 
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(numOfRenderTargets, renderTargetViews.data(), depthStencilView, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindUAVsRenderAndDepthTargets(unsigned int numOfUAVs, unsigned int uavStartSlot, D3D11_Resource* const* uavResources, unsigned int numOfRenderTargets /*= 1*/, Texture* const* renderTargets /*= nullptr*/, Texture* depthTexture /*= nullptr*/, bool isDepthReadOnly /*= false*/)
{
	std::vector<ID3D11UnorderedAccessView*> uavs;
	std::vector<unsigned int> appendConsumeBufferOffset;
	uavs.reserve(numOfUAVs);
	appendConsumeBufferOffset.reserve(numOfUAVs);
	for (unsigned int uavIndex = 0; uavIndex < numOfUAVs; ++uavIndex)
	{
		uavs.emplace_back(uavResources[uavIndex]->m_unorderedAccessView);
		appendConsumeBufferOffset.emplace_back(0);
	}

	if (!numOfRenderTargets)
	{
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
		return;
	}

	ID3D11DepthStencilView* depthStencilView = !depthTexture ? m_depthStencilView : depthTexture->m_depthStencilView;
	if (renderTargets == nullptr)
	{
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_renderTargetView, depthStencilView, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
		return;
	}

	std::vector<ID3D11RenderTargetView*> renderTargetViews;
	renderTargetViews.reserve(numOfRenderTargets);
	for (unsigned int arrayIndex = 0; arrayIndex < numOfRenderTargets; ++arrayIndex)
	{
		Texture* currentRenderTargetTexture = renderTargets[arrayIndex];
		if (currentRenderTargetTexture)
		{
			renderTargetViews.emplace_back(currentRenderTargetTexture->m_renderTargetView);
			continue;
		}
		renderTargetViews.emplace_back(nullptr);
	}

	if (!isDepthReadOnly)
	{
		depthStencilView = !depthTexture ? m_depthStencilView : depthTexture->m_depthStencilView;
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(numOfRenderTargets, renderTargetViews.data(), depthStencilView, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
	}
	else
	{
		depthStencilView = !depthTexture ? m_depthStencilView : depthTexture->m_readOnlyDepthStencilView;
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(numOfRenderTargets, renderTargetViews.data(), depthStencilView, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::BindUAVsRenderAndDepthTargets(unsigned int numOfUAVs, unsigned int uavStartSlot, D3D11_Resource* const* uavResources, unsigned int numOfRenderTargets /*= 1*/, D3D11_Resource* const* renderTargetResources /*= nullptr*/, D3D11_Resource* depthTargetResource /*= nullptr*/, bool isDepthReadOnly /*= false*/, bool isDepthBufferBound /*= true*/)
{
	std::vector<ID3D11UnorderedAccessView*> uavs;
	std::vector<unsigned int> appendConsumeBufferOffset;
	uavs.reserve(numOfUAVs);
	appendConsumeBufferOffset.reserve(numOfUAVs);
	for (unsigned int uavIndex = 0; uavIndex < numOfUAVs; ++uavIndex)
	{
		uavs.emplace_back(uavResources[uavIndex]->m_unorderedAccessView);
		appendConsumeBufferOffset.emplace_back(0);
	}

	if (!numOfRenderTargets)
	{
		if (!depthTargetResource)
		{
			m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
			return;
		}

		ID3D11DepthStencilView* depthStencilView = !isDepthReadOnly ? depthTargetResource->m_depthStencilView : depthTargetResource->m_readOnlyDepthStencilView;
		// Mid Thesis refactor
		// m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_renderTargetView, depthStencilView, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, depthStencilView, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
		return;
	}

	if (!renderTargetResources)
	{
		if (isDepthBufferBound)
		{
			ID3D11DepthStencilView* depthStencilView = !depthTargetResource ? m_depthStencilView : depthTargetResource->m_depthStencilView;
			m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_renderTargetView, depthStencilView, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
			return;
		}

		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_renderTargetView, 0, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
		return;
	}

	std::vector<ID3D11RenderTargetView*> renderTargetViews;
	renderTargetViews.reserve(numOfRenderTargets);
	for (unsigned int arrayIndex = 0; arrayIndex < numOfRenderTargets; ++arrayIndex)
	{
		D3D11_Resource* currentRenderTargetTexture = renderTargetResources[arrayIndex];
		if (currentRenderTargetTexture)
		{
			renderTargetViews.emplace_back(currentRenderTargetTexture->m_renderTargetView);
			continue;
		}
		renderTargetViews.emplace_back(nullptr);
	}

	if (!isDepthReadOnly)
	{
		if (isDepthBufferBound)
		{
			ID3D11DepthStencilView* depthStencilView = !depthTargetResource ? m_depthStencilView : depthTargetResource->m_depthStencilView;
			m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(numOfRenderTargets, renderTargetViews.data(), depthStencilView, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
			return;
		}
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(numOfRenderTargets, renderTargetViews.data(), 0, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());

	}
	else
	{
		if (isDepthBufferBound)
		{
			ID3D11DepthStencilView* depthStencilView = !depthTargetResource ? m_depthStencilView : depthTargetResource->m_readOnlyDepthStencilView;
			m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(numOfRenderTargets, renderTargetViews.data(), depthStencilView, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
			return;
		}
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(numOfRenderTargets, renderTargetViews.data(), 0, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeBufferOffset.data());
	}
}

//--------------------------------------------------------------------------------------------------
void Renderer::UnbindUAVsRenderAndDepthTargets(unsigned int numOfUAVs, unsigned int uavStartSlot, unsigned int numOfRenderTargets /*= 1*/)
{
	// unbind the requested uavs and render targets
	UNUSED(numOfUAVs);
	UNUSED(uavStartSlot);
	UNUSED(numOfRenderTargets);
	// ERROR_AND_DIE("Do not use, yet to be impelemented");
	m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, 0, 0, 0, 0);
}


//--------------------------------------------------------------------------------------------------
// #ToDO: refactor into one unbind readable resource and unbind writable source
void Renderer::UnbindWritableBuffers(unsigned int numOfUAVs, unsigned int startSlot)
{
	std::vector<ID3D11UnorderedAccessView*> uavs;
	std::vector<unsigned int> appendConsumeOffsets;
	uavs.resize(numOfUAVs);
	appendConsumeOffsets.resize(numOfUAVs);
	for (unsigned int uavIndex = 0; uavIndex < numOfUAVs; ++uavIndex)
	{
		uavs[uavIndex]					=	nullptr;
		appendConsumeOffsets[uavIndex]	=	(unsigned int)0;
	}

	m_d3d11DeviceContext->CSSetUnorderedAccessViews(startSlot, numOfUAVs, uavs.data(), appendConsumeOffsets.data());
}


//--------------------------------------------------------------------------------------------------
void Renderer::UnbindWritableResources(unsigned int numOfUAVs /*= 1*/, unsigned int uavStartSlot /*= 0*/, BindingLocation unblindingLocation /*= BindingLocation::PIXEL_SHADER*/)
{
	std::vector<ID3D11UnorderedAccessView*> uavs;
	std::vector<unsigned int> appendConsumeOffsets;
	uavs.reserve(numOfUAVs);
	appendConsumeOffsets.reserve(numOfUAVs);
	for (unsigned int uavIndex = 0; uavIndex < numOfUAVs; ++uavIndex)
	{
		uavs.emplace_back(nullptr);
		appendConsumeOffsets.emplace_back((unsigned int)0);
	}

	switch (unblindingLocation)
	{
	case BindingLocation::VERTEX_SHADER:
	case BindingLocation::PIXEL_SHADER:
	{
		m_d3d11DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, uavStartSlot, numOfUAVs, uavs.data(), appendConsumeOffsets.data());
		break;
	}
	case BindingLocation::COMPUTE_SHADER:
	{
		m_d3d11DeviceContext->CSSetUnorderedAccessViews(uavStartSlot, numOfUAVs, uavs.data(), appendConsumeOffsets.data());
		break;
	}
	default:
		ERROR_AND_DIE("Please provide a valid unbinding location");
		break;
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::UnbindReadableBuffers(unsigned int numOfSRVs, unsigned int startSlot, BindingLocation unbindingLocation)
{
	std::vector<ID3D11ShaderResourceView*> srvs;
	srvs.resize(numOfSRVs);
	for (unsigned int srvIndex = 0; srvIndex < numOfSRVs; ++srvIndex)
	{
		srvs[srvIndex] = nullptr;
	}

	switch (unbindingLocation)
	{
	case BindingLocation::VERTEX_SHADER:
	{
		m_d3d11DeviceContext->VSSetShaderResources(startSlot, numOfSRVs, srvs.data());
		break;
	}
	case BindingLocation::PIXEL_SHADER:
	{
		m_d3d11DeviceContext->PSSetShaderResources(startSlot, numOfSRVs, srvs.data());
		break;
	}
	case BindingLocation::COMPUTE_SHADER:
	{
		m_d3d11DeviceContext->CSSetShaderResources(startSlot, numOfSRVs, srvs.data());
		break;
	}
	default:
		break;
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::UnbindReadableResources(unsigned int numOfSRVs /*= 1*/, unsigned int startSlot /*= 0*/, BindingLocation unbindingLocation /*= BindingLocation::PIXEL_SHADER*/)
{
	std::vector<ID3D11ShaderResourceView*> srvs;
	srvs.reserve(numOfSRVs);
	for (unsigned int srvIndex = 0; srvIndex < numOfSRVs; ++srvIndex)
	{
		srvs.emplace_back(nullptr);
	}

	switch (unbindingLocation)
	{
		case BindingLocation::VERTEX_SHADER:
		{
			m_d3d11DeviceContext->VSSetShaderResources(startSlot, numOfSRVs, srvs.data());
			break;
		}
		case BindingLocation::PIXEL_SHADER:
		{
			m_d3d11DeviceContext->PSSetShaderResources(startSlot, numOfSRVs, srvs.data());
			break;
		}
		case BindingLocation::COMPUTE_SHADER:
		{
			m_d3d11DeviceContext->CSSetShaderResources(startSlot, numOfSRVs, srvs.data());
			break;
		}
		default:
		{
			ERROR_AND_DIE("Please provide a valid binding location");
			break;
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::UnbindWritableTextures(unsigned int numOfUAVs, unsigned int startSlot)
{
	std::vector<ID3D11UnorderedAccessView*> uavs;
	std::vector<unsigned int> appendConsumeOffsets;
	uavs.resize(numOfUAVs);
	appendConsumeOffsets.resize(numOfUAVs);
	for (unsigned int uavIndex = 0; uavIndex < numOfUAVs; ++uavIndex)
	{
		uavs[uavIndex]					=	nullptr;
		appendConsumeOffsets[uavIndex]	=	(unsigned int)0;
	}

	m_d3d11DeviceContext->CSSetUnorderedAccessViews(startSlot, numOfUAVs, uavs.data(), appendConsumeOffsets.data());
}


//--------------------------------------------------------------------------------------------------
void Renderer::UnbindReadableTextures(unsigned int numOfSRVs, unsigned int startSlot, BindingLocation unbindingLocation)
{
	std::vector<ID3D11ShaderResourceView*> srvs;
	srvs.resize(numOfSRVs);
	for (unsigned int srvIndex = 0; srvIndex < numOfSRVs; ++srvIndex)
	{
		srvs[srvIndex] = nullptr;
	}

	switch (unbindingLocation)
	{
	case BindingLocation::VERTEX_SHADER:
	{
		m_d3d11DeviceContext->VSSetShaderResources(startSlot, numOfSRVs, srvs.data());
		break;
	}
	case BindingLocation::PIXEL_SHADER:
	{
		m_d3d11DeviceContext->PSSetShaderResources(startSlot, numOfSRVs, srvs.data());
		break;
	}
	case BindingLocation::COMPUTE_SHADER:
	{
		m_d3d11DeviceContext->CSSetShaderResources(startSlot, numOfSRVs, srvs.data());
		break;
	}
	default:
		break;
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::ClearRenderTargetAndDepthStencil(Texture* const& renderTarget, Texture* const& depthStencil, Rgba8 const& clearColor)
{
	float clearColorAsFloats[4] = {};
	clearColor.GetAsFloats(clearColorAsFloats);
	if (renderTarget)
	{
		m_d3d11DeviceContext->ClearRenderTargetView(renderTarget->m_renderTargetView, clearColorAsFloats);
	}
	else
	{
		m_d3d11DeviceContext->ClearRenderTargetView(m_renderTargetView, clearColorAsFloats);
	}
	
	if (depthStencil)
	{
		float depthClearValue = m_desiredDepthMode != DepthMode::GREATER ? 1.f : 0.f;
		m_d3d11DeviceContext->ClearDepthStencilView(depthStencil->m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthClearValue, 0);
	}
	else
	{
		m_d3d11DeviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	}
}

//--------------------------------------------------------------------------------------------------
void Renderer::ClearDepthTextureAndView(Texture* textureToClear) const
{
	m_d3d11DeviceContext->ClearDepthStencilView(textureToClear->m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}


//--------------------------------------------------------------------------------------------------
void Renderer::ClearDepthResource(D3D11_Resource* depthResourceToClear, float depthValueToClearTo /*= 1.f*/) const
{
	m_d3d11DeviceContext->ClearDepthStencilView(depthResourceToClear->m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthValueToClearTo, 0);
}

//--------------------------------------------------------------------------------------------------
void Renderer::ClearRenderTargetTextureAndView(Texture* textureToClear, Rgba8 const& clearColor) const
{
	float clearColorAsFloats[4] = {};
	clearColor.GetAsFloats(clearColorAsFloats);
	m_d3d11DeviceContext->ClearRenderTargetView(textureToClear->m_renderTargetView, clearColorAsFloats);
}


//--------------------------------------------------------------------------------------------------
void Renderer::ClearRenderTargetResource(D3D11_Resource* rtToClear, Rgba8 const& clearColor) const
{
	float clearColorAsFloats[4] = {};
	clearColor.GetAsFloats(clearColorAsFloats);
	m_d3d11DeviceContext->ClearRenderTargetView(rtToClear->m_renderTargetView, clearColorAsFloats);
}


//--------------------------------------------------------------------------------------------------
void Renderer::ClearTextureView(Texture* textureToClear, Rgba8 const& clearColor) const
{
	float clearColorAsFloats[4] = {};
	clearColor.GetAsFloats(clearColorAsFloats);
	m_d3d11DeviceContext->ClearUnorderedAccessViewFloat(textureToClear->m_unorderedAccessView, clearColorAsFloats);
}


//--------------------------------------------------------------------------------------------------
void Renderer::ReleaseTexture(Texture*& textureToRelease) const
{
	delete textureToRelease;
	textureToRelease = nullptr;
}


//--------------------------------------------------------------------------------------------------
void Renderer::ReleaseBuffer(D3D11_Buffer*& bufferToRelease) const
{
	delete bufferToRelease;
	bufferToRelease = nullptr;
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetDebugResourceName(D3D11_Buffer*& bufferToName, unsigned int debugNameSize, char const* debugName)
{
#ifdef ENGINE_DEBUG_RENDERER
	ID3D11Buffer*&					currentBuffer	=	bufferToName->m_buffer;
	ID3D11UnorderedAccessView*&		currentUAV		=	bufferToName->m_unorderedAccessView;
	ID3D11ShaderResourceView*&		currentSRV		=	bufferToName->m_shaderResourceView;
	ID3D11DepthStencilView*&		currentDSV		=	bufferToName->m_depthStencilView;
	ID3D11RenderTargetView*&		currentRTV		=	bufferToName->m_renderTargetView;


	std::string debugNameUAV	=	std::string(debugName) + "_UAV";
	std::string debugNameSRV	=	std::string(debugName) + "_SRV";
	std::string debugNameDSV	=	std::string(debugName) + "_DSV";
	std::string debugNameRTV	=	std::string(debugName) + "_RTV";

	currentBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugNameSize, debugName);
	if (currentUAV)
	{
		currentUAV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameUAV.size(), debugNameUAV.c_str());
	}
	if (currentSRV)
	{
		currentSRV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameSRV.size(), debugNameSRV.c_str());
	}
	if (currentDSV)
	{
		currentDSV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameDSV.size(), debugNameDSV.c_str());
	}
	if (currentRTV)
	{
		currentRTV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameRTV.size(), debugNameRTV.c_str());
	}

	return;
#endif // ENGINE_DEBUG_RENDERER
	UNUSED(bufferToName)
	UNUSED(debugNameSize)
	UNUSED(debugName)
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetDebugResourceName(Texture*& textureToName, unsigned int debugNameSize, char const* debugName)
{
#ifdef ENGINE_DEBUG_RENDERER
	ID3D11Texture2D*&			currentTexture	=	textureToName->m_texture;
	ID3D11UnorderedAccessView*	currentUAV		=	textureToName->m_unorderedAccessView;
	ID3D11ShaderResourceView*&	currentSRV		=	textureToName->m_shaderResourceView;
	ID3D11DepthStencilView*&	currentDSV		=	textureToName->m_depthStencilView;
	ID3D11RenderTargetView*&	currentRTV		=	textureToName->m_renderTargetView;

	std::string debugNameUAV	=	std::string(debugName) + "_UAV";
	std::string debugNameSRV	=	std::string(debugName) + "_SRV";
	std::string debugNameDSV	=	std::string(debugName) + "_DSV";
	std::string debugNameRTV	=	std::string(debugName) + "_RTV";

	currentTexture->SetPrivateData(WKPDID_D3DDebugObjectName, debugNameSize, debugName);
	if (currentUAV)
	{
		currentUAV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameUAV.size(), debugNameUAV.c_str());
	}
	if (currentSRV)
	{
		currentSRV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameSRV.size(), debugNameSRV.c_str());
	}
	if (currentDSV)
	{
		currentDSV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameDSV.size(), debugNameDSV.c_str());
	}
	if (currentRTV)
	{
		currentRTV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameRTV.size(), debugNameRTV.c_str());
	}

	return;
#endif // ENGINE_DEBUG_RENDERER
	UNUSED(textureToName)
	UNUSED(debugNameSize)
	UNUSED(debugName)
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetDebugResourceName(D3D11_Resource*& resourceToName, unsigned int debugNameSize, char const* debugName)
{
#ifdef ENGINE_DEBUG_RENDERER
	switch (resourceToName->m_resourceType)
	{
		case ResourceType::TEXTURE2D:
		{
			ID3D11Texture2D*& currentTexture = resourceToName->m_texture2d;
			currentTexture->SetPrivateData(WKPDID_D3DDebugObjectName, debugNameSize, debugName);
			break;
		}
		case ResourceType::STRUCTURED_BUFFER:
		case ResourceType::RAW_BUFFER:
		{
			ID3D11Buffer*& currentBuffer = resourceToName->m_buffer;
			currentBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugNameSize, debugName);
			break;
		}
	default:
		break;
	}

	ID3D11UnorderedAccessView*&	currentUAV			=	resourceToName->m_unorderedAccessView;
	ID3D11ShaderResourceView*&	currentSRV			=	resourceToName->m_shaderResourceView;
	ID3D11DepthStencilView*&	currentDSV			=	resourceToName->m_depthStencilView;
	ID3D11DepthStencilView*&	currentReadOnlyDSV	=	resourceToName->m_readOnlyDepthStencilView;
	ID3D11RenderTargetView*&	currentRTV			=	resourceToName->m_renderTargetView;

	std::string debugNameUAV			=	std::string(debugName) + "_UAV";
	std::string debugNameSRV			=	std::string(debugName) + "_SRV";
	std::string debugNameDSV			=	std::string(debugName) + "_DSV";
	std::string debugNameReadOnlyDSV	=	std::string(debugName) + "_ReadOnly_DSV";
	std::string debugNameRTV			=	std::string(debugName) + "_RTV";

	if (currentUAV)
	{
		currentUAV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameUAV.size(), debugNameUAV.c_str());
	}
	if (currentSRV)
	{
		currentSRV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameSRV.size(), debugNameSRV.c_str());
	}
	if (currentDSV)
	{
		currentDSV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameDSV.size(), debugNameDSV.c_str());
	}
	if (currentReadOnlyDSV)
	{
		currentReadOnlyDSV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameReadOnlyDSV.size(), debugNameReadOnlyDSV.c_str());
	}
	if (currentRTV)
	{
		currentRTV->SetPrivateData(WKPDID_D3DDebugObjectName, (unsigned int)debugNameRTV.size(), debugNameRTV.c_str());
	}

	return;
#endif // ENGINE_DEBUG_RENDERER
	UNUSED(resourceToName)
	UNUSED(debugNameSize)
	UNUSED(debugName)
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetDebugResourceName(VertexBuffer*& bufferToName, unsigned int debugNameSize, char const* debugName)
{
	#ifdef ENGINE_DEBUG_RENDERER
	ID3D11Buffer*&	currentBuffer	=	bufferToName->m_buffer;
	currentBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugNameSize, debugName);
	return;
#endif // ENGINE_DEBUG_RENDERER
	UNUSED(bufferToName)
	UNUSED(debugNameSize)
	UNUSED(debugName)
}


//--------------------------------------------------------------------------------------------------
void Renderer::SetDebugResourceName(IndexBuffer*& bufferToName, unsigned int debugNameSize, char const* debugName)
{
	#ifdef ENGINE_DEBUG_RENDERER
	ID3D11Buffer*&	currentBuffer	=	bufferToName->m_buffer;
	currentBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugNameSize, debugName);
	return;
#endif // ENGINE_DEBUG_RENDERER
	UNUSED(bufferToName)
	UNUSED(debugNameSize)
	UNUSED(debugName)
}


//--------------------------------------------------------------------------------------------------
void Renderer::CopyResource(D3D11_Resource* resourceToCopy, D3D11_Resource* copyDestinationResource)
{
	switch (resourceToCopy->m_resourceType)
	{
		case ResourceType::TEXTURE1D:
		{
			m_d3d11DeviceContext->CopyResource(copyDestinationResource->m_texture1d, resourceToCopy->m_texture1d);
			break;
		}
		case ResourceType::TEXTURE2D:
		{
			m_d3d11DeviceContext->CopyResource(copyDestinationResource->m_texture2d, resourceToCopy->m_texture2d);
			break;
		}
		case ResourceType::TEXTURE3D:
		{
			m_d3d11DeviceContext->CopyResource(copyDestinationResource->m_texture3d, resourceToCopy->m_texture3d);
			break;
		}
		case ResourceType::STRUCTURED_BUFFER:
		{
			m_d3d11DeviceContext->CopyResource(copyDestinationResource->m_buffer, resourceToCopy->m_buffer);
			break;
		}
		case ResourceType::RAW_BUFFER:
		{

			break;
		}
		default:
			break;
	}
}


//--------------------------------------------------------------------------------------------------
void Renderer::CopyTextureResource(Texture const* dest, Texture const* src)
{
	m_d3d11DeviceContext->CopyResource(dest->m_texture, src->m_texture);
}


//--------------------------------------------------------------------------------------------------
Texture* Renderer::GetDefaultTexture() const
{
	return m_defaultTexture;
}


//--------------------------------------------------------------------------------------------------
D3D11_Resource::~D3D11_Resource()
{
	DX_SAFE_RELEASE(m_readOnlyDepthStencilView);
	DX_SAFE_RELEASE(m_unorderedAccessView);
	DX_SAFE_RELEASE(m_shaderResourceView);
	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_depthStencilView);
	DX_SAFE_RELEASE(m_texture3d);
	DX_SAFE_RELEASE(m_texture2d);
	DX_SAFE_RELEASE(m_texture1d);
	DX_SAFE_RELEASE(m_buffer);
}
