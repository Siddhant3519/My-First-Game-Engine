#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Image.hpp"


//--------------------------------------------------------------------------------------------------
#include "Game/EngineBuildPreferences.hpp"


//--------------------------------------------------------------------------------------------------
#include <vector>


//--------------------------------------------------------------------------------------------------
#define DX_SAFE_RELEASE(dxObject) { if ((dxObject) != nullptr) { (dxObject)->Release(); (dxObject) = nullptr; } }


//--------------------------------------------------------------------------------------------------
#if defined(OPAQUE)
#undef OPAQUE
#endif


//--------------------------------------------------------------------------------------------------
struct ID3DUserDefinedAnnotation;
struct ID3D11DepthStencilState;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11RasterizerState;
struct ID3D11DeviceContext;
struct ID3D11SamplerState;
struct ID3D11BlendState;
struct ID3D11Texture2D;
struct IDXGISwapChain;
struct ID3D11Device;


//--------------------------------------------------------------------------------------------------
class  Window;
class  Shader;
class  Texture;
class  BitmapFont;
class  IndexBuffer;
class  VertexBuffer;
class  D3D11_Buffer;
class  ConstantBuffer;
class  D3D11_Resource;
class  StructuredBuffer;
struct D3D11_ResourceConfig;


//--------------------------------------------------------------------------------------------------
enum class BlendMode : unsigned char
{
	INVALID,
	ALPHA, // Back to front composting (Over)
	ADDITIVE,
	OPAQUE,
	ACCUMULATION,
	REVEALAGE,
	ALPHA_UNDER, // Front to back Compositing (Under)
	COUNT,
};


//--------------------------------------------------------------------------------------------------
enum class SamplerMode : unsigned char
{
	POINT_CLAMP,				
	BILINEAR_WRAP,						// Averaging 
	COUNT,
};


//--------------------------------------------------------------------------------------------------
enum class RasterizerMode : unsigned char
{
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	SOLID_CULL_FRONT,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	COUNT,	
};


//--------------------------------------------------------------------------------------------------
enum class DepthMode : unsigned char
{
	DISABLED,
	ENABLED,
	GREATER,
	COUNT,
};


//--------------------------------------------------------------------------------------------------
enum class ResourceUsage : unsigned char
{
	GPU_READ_GPU_WRITE						=	0,	
	GPU_READ								=	1,
	GPU_READ_CPU_WRITE						=	2,
	GPU_READ_CPU_READ_GPU_WRITE_CPU_WRITE	=	3,
	GPU_READ_GPU_WRITE_CPU_READ,
};


//--------------------------------------------------------------------------------------------------
enum class BindingLocation : unsigned char
{
	VERTEX_SHADER,
	PIXEL_SHADER,
	COMPUTE_SHADER,
};


//--------------------------------------------------------------------------------------------------
// TODO: rename format types to something more compact
enum class ResourceViewFormat : unsigned char
{
	DXGI_FORMAT_UNKNOWN	                                =	0,
    DXGI_FORMAT_R32G32B32A32_TYPELESS                   =	1,
    DXGI_FORMAT_R32G32B32A32_FLOAT                      =	2,
    DXGI_FORMAT_R32G32B32A32_UINT                       =	3,
    DXGI_FORMAT_R32G32B32A32_SINT                       =	4,
    DXGI_FORMAT_R32G32B32_TYPELESS                      =	5,
    DXGI_FORMAT_R32G32B32_FLOAT                         =	6,
    DXGI_FORMAT_R32G32B32_UINT                          =	7,
    DXGI_FORMAT_R32G32B32_SINT                          =	8,
    DXGI_FORMAT_R16G16B16A16_TYPELESS                   =	9,
    DXGI_FORMAT_R16G16B16A16_FLOAT                      =	10,
    DXGI_FORMAT_R16G16B16A16_UNORM                      =	11,
    DXGI_FORMAT_R16G16B16A16_UINT                       =	12,
    DXGI_FORMAT_R16G16B16A16_SNORM                      =	13,
    DXGI_FORMAT_R16G16B16A16_SINT                       =	14,
    DXGI_FORMAT_R32G32_TYPELESS                         =	15,
    DXGI_FORMAT_R32G32_FLOAT                            =	16,
    DXGI_FORMAT_R32G32_UINT                             =	17,
    DXGI_FORMAT_R32G32_SINT                             =	18,
    DXGI_FORMAT_R32G8X24_TYPELESS                       =	19,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT                    =	20,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS                =	21,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT                 =	22,
    DXGI_FORMAT_R10G10B10A2_TYPELESS                    =	23,
    DXGI_FORMAT_R10G10B10A2_UNORM                       =	24,
    DXGI_FORMAT_R10G10B10A2_UINT                        =	25,
    DXGI_FORMAT_R11G11B10_FLOAT                         =	26,
    DXGI_FORMAT_R8G8B8A8_TYPELESS                       =	27,
    DXGI_FORMAT_R8G8B8A8_UNORM                          =	28,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB                     =	29,
    DXGI_FORMAT_R8G8B8A8_UINT                           =	30,
    DXGI_FORMAT_R8G8B8A8_SNORM                          =	31,
    DXGI_FORMAT_R8G8B8A8_SINT                           =	32,
    DXGI_FORMAT_R16G16_TYPELESS                         =	33,
    DXGI_FORMAT_R16G16_FLOAT                            =	34,
    DXGI_FORMAT_R16G16_UNORM                            =	35,
    DXGI_FORMAT_R16G16_UINT                             =	36,
    DXGI_FORMAT_R16G16_SNORM                            =	37,
    DXGI_FORMAT_R16G16_SINT                             =	38,
    DXGI_FORMAT_R32_TYPELESS                            =	39,
    DXGI_FORMAT_D32_FLOAT                               =	40,
    DXGI_FORMAT_R32_FLOAT                               =	41,
    DXGI_FORMAT_R32_UINT                                =	42,
    DXGI_FORMAT_R32_SINT                                =	43,
    DXGI_FORMAT_R24G8_TYPELESS                          =	44,
    DXGI_FORMAT_D24_UNORM_S8_UINT                       =	45,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS                   =	46,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT                    =	47,
    DXGI_FORMAT_R8G8_TYPELESS                           =	48,
    DXGI_FORMAT_R8G8_UNORM                              =	49,
    DXGI_FORMAT_R8G8_UINT                               =	50,
    DXGI_FORMAT_R8G8_SNORM                              =	51,
    DXGI_FORMAT_R8G8_SINT                               =	52,
    DXGI_FORMAT_R16_TYPELESS                            =	53,
    DXGI_FORMAT_R16_FLOAT                               =	54,
    DXGI_FORMAT_D16_UNORM                               =	55,
    DXGI_FORMAT_R16_UNORM                               =	56,
    DXGI_FORMAT_R16_UINT                                =	57,
    DXGI_FORMAT_R16_SNORM                               =	58,
    DXGI_FORMAT_R16_SINT                                =	59,
    DXGI_FORMAT_R8_TYPELESS                             =	60,
    DXGI_FORMAT_R8_UNORM                                =	61,
    DXGI_FORMAT_R8_UINT                                 =	62,
    DXGI_FORMAT_R8_SNORM                                =	63,
    DXGI_FORMAT_R8_SINT                                 =	64,
    DXGI_FORMAT_A8_UNORM                                =	65,
    DXGI_FORMAT_R1_UNORM                                =	66,
    DXGI_FORMAT_R9G9B9E5_SHAREDEXP                      =	67,
    DXGI_FORMAT_R8G8_B8G8_UNORM                         =	68,
    DXGI_FORMAT_G8R8_G8B8_UNORM                         =	69,
    DXGI_FORMAT_BC1_TYPELESS                            =	70,
    DXGI_FORMAT_BC1_UNORM                               =	71,
    DXGI_FORMAT_BC1_UNORM_SRGB                          =	72,
    DXGI_FORMAT_BC2_TYPELESS                            =	73,
    DXGI_FORMAT_BC2_UNORM                               =	74,
    DXGI_FORMAT_BC2_UNORM_SRGB                          =	75,
    DXGI_FORMAT_BC3_TYPELESS                            =	76,
    DXGI_FORMAT_BC3_UNORM                               =	77,
    DXGI_FORMAT_BC3_UNORM_SRGB                          =	78,
    DXGI_FORMAT_BC4_TYPELESS                            =	79,
    DXGI_FORMAT_BC4_UNORM                               =	80,
    DXGI_FORMAT_BC4_SNORM                               =	81,
    DXGI_FORMAT_BC5_TYPELESS                            =	82,
    DXGI_FORMAT_BC5_UNORM                               =	83,
    DXGI_FORMAT_BC5_SNORM                               =	84,
    DXGI_FORMAT_B5G6R5_UNORM                            =	85,
    DXGI_FORMAT_B5G5R5A1_UNORM                          =	86,
    DXGI_FORMAT_B8G8R8A8_UNORM                          =	87,
    DXGI_FORMAT_B8G8R8X8_UNORM                          =	88,
    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM              =	89,
    DXGI_FORMAT_B8G8R8A8_TYPELESS                       =	90,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB                     =	91,
    DXGI_FORMAT_B8G8R8X8_TYPELESS                       =	92,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB                     =	93,
    DXGI_FORMAT_BC6H_TYPELESS                           =	94,
    DXGI_FORMAT_BC6H_UF16                               =	95,
    DXGI_FORMAT_BC6H_SF16                               =	96,
    DXGI_FORMAT_BC7_TYPELESS                            =	97,
    DXGI_FORMAT_BC7_UNORM                               =	98,
    DXGI_FORMAT_BC7_UNORM_SRGB                          =	99,
    DXGI_FORMAT_AYUV                                    =	100,
    DXGI_FORMAT_Y410                                    =	101,
    DXGI_FORMAT_Y416                                    =	102,
    DXGI_FORMAT_NV12                                    =	103,
    DXGI_FORMAT_P010                                    =	104,
    DXGI_FORMAT_P016                                    =	105,
    DXGI_FORMAT_420_OPAQUE                              =	106,
    DXGI_FORMAT_YUY2                                    =	107,
    DXGI_FORMAT_Y210                                    =	108,
    DXGI_FORMAT_Y216                                    =	109,
    DXGI_FORMAT_NV11                                    =	110,
    DXGI_FORMAT_AI44                                    =	111,
    DXGI_FORMAT_IA44                                    =	112,
    DXGI_FORMAT_P8                                      =	113,
    DXGI_FORMAT_A8P8                                    =	114,
    DXGI_FORMAT_B4G4R4A4_UNORM                          =	115,

    DXGI_FORMAT_P208                                    =	130,
    DXGI_FORMAT_V208                                    =	131,
    DXGI_FORMAT_V408                                    =	132,


    DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE         =	189,
    DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE =	190,
};


//--------------------------------------------------------------------------------------------------
enum class PrimitiveTopology : unsigned char
{
	D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED				=	0,
	D3D11_PRIMITIVE_TOPOLOGY_POINTLIST				=	1,
	D3D11_PRIMITIVE_TOPOLOGY_LINELIST				=	2,
	D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP				=	3,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST			=	4,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP			=	5,
	D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ			=	10,
	D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ			=	11,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ		=	12,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ		=	13,
};


//--------------------------------------------------------------------------------------------------
enum class ResourceBindFlag : unsigned char
{
	VERTEX_BUFFER		=	0x1L,
	INDEX_BUFFER		=	0x2L,
	CONSTANT_BUFFER		=	0x4L,
	SHADER_RESOURCE		=	0x8L,
	STREAM_OUTPUT		=	0x10L,
	RENDER_TARGET		=	0x20L,
	DEPTH_STENCIL		=	0x40L,
	UNORDERED_ACCESS	=	0x80L,
};


//--------------------------------------------------------------------------------------------------
enum class UAVResourceFlag : unsigned char
{
	RAW		=	0x1,
    APPEND	=	0x2,
    COUNTER	=	0x4
};


//--------------------------------------------------------------------------------------------------
enum class InputLayout : unsigned char
{
	VERTEX_NONE,
	VERTEX_PCU,
	VERTEX_PCUTBN,
};


//--------------------------------------------------------------------------------------------------
enum class ResourceType : unsigned char
{
	INVALID,
	TEXTURE1D,
	TEXTURE2D,
	TEXTURE3D,
	TEXTURE2D_CUBEMAP,
	STRUCTURED_BUFFER,
	RAW_BUFFER,
};

	
//--------------------------------------------------------------------------------------------------
struct RendererConfig
{
	Window* m_window = nullptr;
};


//--------------------------------------------------------------------------------------------------
struct TextureConfig
{
	ResourceType			m_type							=	ResourceType::TEXTURE2D;
	ResourceUsage			m_usageFlag						=	ResourceUsage::GPU_READ_GPU_WRITE;
	ResourceViewFormat		m_format						=	ResourceViewFormat::DXGI_FORMAT_R32G32B32A32_FLOAT;
	ResourceBindFlag		m_bindFlags						=	ResourceBindFlag::SHADER_RESOURCE;
	unsigned int			m_width							=	(unsigned int)-1;
	unsigned int			m_height						=	(unsigned int)-1;
	unsigned int			m_depth							=	(unsigned int)-1;
	unsigned int			m_mipLevels						=	1;
	unsigned int			m_numOfSlices					=	1;
	unsigned int			m_multiSampleCount				=	1;
	unsigned int			m_multiSampleQuality			=	0;
	unsigned int			m_sizeOfTexelInBytes			=	0;
	unsigned int			m_debugNameSize					=	0;
	char const*				m_debugName						=	"None";
	void*					m_defaultInitializationData		=	nullptr;
};


//--------------------------------------------------------------------------------------------------
enum LightType
{
	LIGHT_TYPE_INVALID = -1,

	LIGHT_TYPE_POINT_LIGHT,
	LGIHT_TYPE_SPOT_LIGHT,

	LIGHT_TYPE_COUNT,
};


//------------------------------------------------------------------------------------------------
struct Light
{
	bool		Enabled = false;						 // 4  bytes
	Vec3		Position = Vec3::ZERO;					 // 12 bytes
	//-----------------------------------				(16 byte boundary)
	Vec3		Direction = Vec3::ZERO;					 // 12 bytes
	int         LightType = LIGHT_TYPE_POINT_LIGHT;      // 4  bytes
	//-----------------------------------				(16 byte boundary)
	float		Color[4];								 // 16 bytes
	//-----------------------------------				(16 byte boundary)
	float       SpotAngle;								 // 4  bytes
	float       ConstantAttenuation;					 // 4  bytes
	float       LinearAttenuation;						 // 4  bytes
	float       QuadraticAttenuation;					 // 4  bytes
	//-----------------------------------				(16 byte boundary)
};


//--------------------------------------------------------------------------------------------------
constexpr int MAX_LIGHTS = 8;


//--------------------------------------------------------------------------------------------------
class Renderer
{
public:
	Renderer(RendererConfig const& config);
	~Renderer();

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void ClearScreen(Rgba8 const& clearColor);
	void BeginCamera(Camera const& camera);
	void EndCamera(Camera const& camera);
	void DrawVertexArray(int numVertexes, Vertex_PCU const* vertexes);
	void DrawVertexArray(std::vector<Vertex_PCU> const& vertexes, PrimitiveTopology const& primitiveTopology = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	void DrawIndexedArray(int numVertexes, Vertex_PCU const* vertexes, int numIndexes, unsigned int const* indexes);
	void DrawIndexedArray(int numVertexes, Vertex_PCUTBN const* vertexes, int numIndexes, unsigned int const* indexes);
	
	RendererConfig const& GetConfig() const { return m_config; }

	Texture*	CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, unsigned char* texelData);
	void		BindTexture(Texture const* texture, BindingLocation bindingLocation = BindingLocation::PIXEL_SHADER);
	Texture*	CreateOrGetTextureFromFile(char const* imageFilePath);
	BitmapFont*	CreateOrGetBitmapFont(char const* bitmapFontFilePathWithNoExtension);
	Shader*		CreateOrGetShader(char const* shaderFilePath, InputLayout const& inputLayout = InputLayout::VERTEX_PCU, bool isUsedForInstancedRendering = false);

	Texture*	GetTextureForFileName	(char const* imageFilePath);
	BitmapFont* GetBitmapFontForFileName(char const* bitmapFontFilePathWithNoExtension);

	void	BindShader(Shader* shader, BindingLocation bindingLocation = BindingLocation::PIXEL_SHADER);
	void	UnbindShaders();
	Shader* CreateShader(char const* shaderName, char const* shaderSource, InputLayout const& inputLayout = InputLayout::VERTEX_PCU, bool isUsedForInstancedRendering = false);
	Shader* CreateShader(char const* shaderName, InputLayout const& inputLayout = InputLayout::VERTEX_PCU, bool isUsedForInstancedRendering = false);
	Shader* CreateComputeShader(char const* shaderName);
	bool	CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target);

	VertexBuffer*	CreateVertexBuffer(size_t const size);
	VertexBuffer*	CreateVertexBuffer(size_t const size, unsigned int stride);
	VertexBuffer*	CreateVertexBuffer(size_t const size, unsigned int stride, ResourceUsage bufferUsage, void* defaultInitializationData = nullptr, bool isStreamOut = false); // ResourceBindFlag bindFlag = ResourceBindFlag::VERTEX_BUFFER
	void			CreateVertexBuffer(D3D11_Buffer*& out_buffer, unsigned int numOfElements, unsigned int elementSize, ResourceUsage const& bufferUsage, void* defaultDataToBePopulated = nullptr, bool isStreamOut = false);
	IndexBuffer*	CreateIndexBuffer(size_t const size);
	IndexBuffer*	CreateIndexBuffer(size_t const size, ResourceUsage bufferUsage, void* defaultInitializationData = nullptr); // ResourceBindFlag bindFlag = ResourceBindFlag::VERTEX_BUFFER
	void			CreateIndexBuffer(D3D11_Buffer*& out_indexBuffer, size_t const numOfElements, ResourceUsage bufferUsage, void* defaultInitializationData = nullptr);
	void			CreateStructuredBuffer(D3D11_Buffer*& out_buffer, unsigned int numOfElements, unsigned int elementSize, ResourceUsage const& bufferUsage, void* defaultDataToBePopulated = nullptr, bool isStandard = false, 
										   ResourceViewFormat const resourceViewFormat = ResourceViewFormat::DXGI_FORMAT_R32G32B32_FLOAT, UAVResourceFlag const uavResourceFlag = UAVResourceFlag::COUNTER);
	void			CreateAppendConsumeBuffer(D3D11_Buffer*& out_buffer, unsigned int numOfElements, unsigned int elementSize, void* defaultDataToBePopulated = nullptr);
	void			CreateRawBuffer(D3D11_Buffer*& out_buffer, unsigned int numOfElements, ResourceUsage const& bufferUsage, void* defaultDataToBePopulated = nullptr);

	void CopyCPUToGPU(void const* data, size_t size, VertexBuffer*& vbo);
	void CopyCPUToGPU(void const* data, size_t size, IndexBuffer*& ibo);
	void CopyGPUToCPU(D3D11_Resource const* resourceToCopy, void*& out_data);
	void BindVertexBuffer(VertexBuffer* vbo, PrimitiveTopology primitiveTopology = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// void BindVertexBuffers(VertexBuffer** arrayOfVertexBuffers, unsigned int* arrayOfStrides = nullptr, unsigned int* arrayOfOffsets = nullptr, unsigned int numOfVertexBuffersToBind = 1, unsigned int startOffset = 0);
	void BindIndexBuffer(IndexBuffer* ibo);
	void BindIndexBuffer(D3D11_Buffer* ibo);
	void BindInstanceBuffer(D3D11_Buffer* instanceBuffer, D3D11_Buffer* vertexBuffer, PrimitiveTopology primitiveTopology = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ConstantBuffer* CreateConstantBuffer(size_t const size);
	ConstantBuffer* CreateConstantBuffer(size_t const& size, ResourceUsage bufferUsage, void* defaultInitializationData = nullptr);
	ConstantBuffer* CreateConstantBuffer(unsigned int const& size, bool isDynamic, bool isCPUUpdate, void* defaultInitializationData = nullptr);
	void			CopyCPUToGPU(void const* data, size_t size, ConstantBuffer*& cbo);
	void			BindConstantBuffer(int slot, ConstantBuffer* cbo, BindingLocation bindingLocation = BindingLocation::PIXEL_SHADER);

	void DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, PrimitiveTopology const& primitiveTopology = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, int vertexOffset = 0);
	void DrawVertexAndIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset = 0, int vertexOffset = 0);
	void DrawIndexedBuffer(IndexBuffer* ibo, VertexBuffer* vbo, unsigned int indexCount, unsigned int indexOffset = 0, int vertexOffset = 0);
	void DrawIndexed(int indexCount, int indexOffset = 0, int vertexOffset = 0);
	void DrawIndexedInstanced(D3D11_Buffer* indexBuffer, D3D11_Buffer* instanceBuffer, D3D11_Buffer* vertexBuffer, unsigned int indexCountPerInstance, 
							  unsigned int instanceCount, unsigned int startIndexLocation = 0, unsigned int baseVertexLocation = 0, unsigned int startInstanceLocation = 0);

	void ComputeShaderDispatch(unsigned int threadGroupX, unsigned int threadGroupY, unsigned int threadGroupZ);

	void CreateUserDefinedDebugAnnotation();
	void CreateAndInitializeBlendModes();
	void CreateSamplerModes();
	void CreateRasterizerMode();
	void CreateDepthMode();
	void SetBlendMode(BlendMode blendMode);
	void CreateBlendModes(BlendMode const* blendModes, unsigned short numOfBlendModes); // Unoptimized: #FIXME
	void SetCustomBlendMode();
	void SetSamplerMode(SamplerMode samplerMode);
	void SetRasterizerMode(RasterizerMode rasterizerMode);
	void SetDepthMode(DepthMode depthMode);
	void SetStatesIfChanged();
	void SetModelConstants(Mat44 const& modelMatrix = Mat44(), Rgba8 const& modelColor = Rgba8::WHITE);
	void SetLightingConstants(Vec3 const& sunDirection, float sunIntensity, float ambientIntensity, Vec3 worldEyePosition = Vec3::ZERO, int normalMode = 0, int specularMode = 0, float specularIntensity = 0.f, float specularPower = 0.f);
	void SetLightAt(Light const& lightToSet, int indexToSet);

	void SetCustomAnnotationMarker(wchar_t const* annotationText);
	void BeginAnnotationEvent(wchar_t const* annotationText);
	void EndAnnotationEvent();

	void			CreateDefaultDepthTextureAndView();
	void			CreateNewDepthTextureAndView(Texture*& depthTexture, unsigned int debugResourceNameSize = 0, char const* debugResourceName = "None", IntVec2 const& textureDims = IntVec2(-1, -1));
	void			CreateWritableRenderTarget(Texture*& renderTarget, unsigned int debugResourceNameSize = 0, char const* debugResourceName = "None", IntVec2 const& textureDims = IntVec2(-1, -1));
	void			CreateNewRenderTextureShaderResourceAndView(Texture*& renderTexture)	const;
	void			CreateNewShaderResourceView(Texture*& textureContainingSRV)				const;
	void			CreateReadOnlyDepthStencilView(Texture*& texture)						const;
	void			CreateWritableTextureAndView(Texture*& textureContainingUAV);
	void			CreateTextureFromConfig(TextureConfig const& config, Texture*& texture);
	void			CreateResourceFromConfig(D3D11_ResourceConfig& config, D3D11_Resource*& resource);
	D3D11_Resource* CreateTextureResourceFromFile(char const* imageFilePath, char const* debugResourceName = nullptr, unsigned int debugResourceNameSize = 0);
	D3D11_Resource* CreateTextureCubeResourceFromFile(char const* cubeMapDir, char const* debugResourceName = nullptr, unsigned int debugResourceNameSize = 0);
	void			CreateDepthResource(D3D11_Resource*& depthResource, char const* debugResourceName = "None", unsigned int debugResourceNameSize = 0, bool canBeReadOnly = false, IntVec2 const& textureDims = IntVec2(-1, -1));
	void			CreateRenderTargetResource(D3D11_Resource*& renderTargetResource, bool isWritable, char const* debugResourceName = nullptr, unsigned int debugResourceNameSize = 0, IntVec2 const& textureDims = IntVec2(-1, -1));


	void BindRenderTargetOnly(Texture* renderTarget);
	void BindRenderAndDepthTargetViewsToThePipeline(Texture* renderTexture = nullptr, Texture* depthTexture = nullptr, D3D11_Buffer* buffer = nullptr);
	void BindRenderAndDepthTargetViews(Texture* renderTarget = nullptr, Texture* depthStencil = nullptr, bool isDepthReadOnly = false);
	void BindRenderAndDepthResources(D3D11_Resource* renderTarget = nullptr, D3D11_Resource* depthResource = nullptr, bool isDepthReadOnly = false, bool bindDefault = false);
	void BindRenderAndDepthTargetViewsToThePipeline(unsigned int numOfRenderTargetViews, Texture* const* renderTargets = nullptr, Texture* depthTarget = nullptr, bool isDepthReadOnly = false);
	void BindRenderAndDepthResources(unsigned int numOfRenderTargetViews, D3D11_Resource* const* renderTargets = nullptr, D3D11_Resource* depthTarget = nullptr, bool isDepthReadOnly = false);
	void UnbindRenderAndDepthTargets();
	void BindWritableBuffersToThePipeline(D3D11_Buffer* const* writableBuffers, unsigned int numOfUAVs, unsigned int uavStartSlot = 0, Texture const* renderTargetTexture = nullptr, Texture const* depthStencilTexture = nullptr, bool isReadOnlyDepth = false);
	void BindWritableBufferToThePipeline(D3D11_Buffer* writableBuffer, unsigned int uavStartSlot = 0);
	void BindReadOnlyBufferToThePipeleine(D3D11_Buffer* readableBuffer, unsigned int startSlot, unsigned int numOfUAVs = 1, unsigned int uavStartSlot = 0);
	void BindWritableBufferToTheComputeShader(D3D11_Buffer* writableBuffers, unsigned int startSlot = 1, unsigned int numOfUAVs = 1, unsigned int startOffset = 0);
	void BindWritableTextureToTheComputeShader(Texture* writableTexture, unsigned int startSlot = 3, unsigned int numOfUAVs = 1, unsigned int startOffset = 0);
	void BindWritableBuffersToTheComputeShader(D3D11_Buffer* const* writableBuffers, unsigned int numOfUAVs = 1, unsigned int startSlot = 0, unsigned int const* appendConsumeOffsets = nullptr);
	void BindWritableTexturesToTheComputeShader(Texture* const* writableTextures, unsigned int numOfUAVs = 1, unsigned int startSlot = 0, unsigned int const* appendConsumeOffsets = nullptr);
	void BindWritableResourcesToComputeShader(D3D11_Resource* const* writableResources, unsigned int numOfUavs = 1, unsigned int startSlot = 0, unsigned int const* appendConsumeOffsets = nullptr);
	void BindReadableBuffersToTheComputeShader(D3D11_Buffer* const* readOnlyBuffers, unsigned int numOfSRVs = 1, unsigned int startSlot = 0);
	void BindReadableResources(D3D11_Resource* const* readableResourcesToBind, unsigned int numOfSRVs, unsigned int startSlot, BindingLocation bindingLocation);
	void BindReadableTexturesToTheComputeShader(Texture* const* readOnlyTextures, unsigned int numOfSRVs = 1, unsigned int startSlot = 0);
	void BindReadableTexture(Texture* const* readableTextures, unsigned int numOfSRVs = 1, unsigned int startSlot = 0, BindingLocation bindingLocation = BindingLocation::PIXEL_SHADER);
	void BindUAVsRenderAndDepthTargets(unsigned int numOfUAVs, unsigned int uavStartSlot, Texture* const* uavTexture, unsigned int numOfRenderTargets = 1, Texture* const* renderTargetTexture = nullptr, Texture* depthTexture = nullptr, bool isDepthReadOnly = false);
	void BindUAVsRenderAndDepthTargets(unsigned int numOfUAVs, unsigned int uavStartSlot, D3D11_Resource* const* uavResources, unsigned int numOfRenderTargets, D3D11_Resource* const* renderTargetResources, D3D11_Resource* depthTargetResource, bool isDepthReadOnly = false, bool isDepthBufferBound = true);
	void BindUAVsRenderAndDepthTargets(unsigned int numOfUAVs, unsigned int uavStartSlot, D3D11_Resource* const* uavResources, unsigned int numOfRenderTargets = 1, Texture* const* renderTargetTexture = nullptr, Texture* depthTexture = nullptr, bool isDepthReadOnly = false);
	void UnbindUAVsRenderAndDepthTargets(unsigned int numOfUAVs, unsigned int uavStartSlot, unsigned int numOfRenderTargets = 1);

	void UnbindWritableBuffers(unsigned int numOfUAVs = 1, unsigned int startSlot = 0);
	void UnbindWritableResources(unsigned int numOfUAVs = 1, unsigned int uavStartSlot = 0, BindingLocation unblindingLocation = BindingLocation::PIXEL_SHADER);
	void UnbindReadableBuffers(unsigned int numOfSRVs = 1, unsigned int startSlot = 0, BindingLocation unbindingLocation = BindingLocation::PIXEL_SHADER);
	void UnbindReadableResources(unsigned int numOfSRVs = 1, unsigned int startSlot = 0, BindingLocation unbindingLocation = BindingLocation::PIXEL_SHADER);
	void UnbindWritableTextures(unsigned int numOfUAVs = 1, unsigned int startSlot = 0);
	void UnbindReadableTextures(unsigned int numOfSRVs = 1, unsigned int startSlot = 0, BindingLocation unbindingLocation = BindingLocation::PIXEL_SHADER);

	void ClearRenderTargetAndDepthStencil(Texture* const& renderTarget = nullptr, Texture* const& depthStencil = nullptr, Rgba8 const& clearColor = Rgba8::BLACK);
	void ClearDepthTextureAndView(Texture* textureToClear)	const;
	void ClearDepthResource(D3D11_Resource* depthResourceToClear, float depthValueToClearTo = 1.f) const;
	void ClearRenderTargetTextureAndView(Texture* textureToClear, Rgba8 const& clearColor) const;
	void ClearRenderTargetResource(D3D11_Resource* rtToClear, Rgba8 const& clearColor) const;
	void ClearTextureView(Texture* textureToClear, Rgba8 const& clearColor) const;
	void ReleaseTexture(Texture*& textureToRelease)			const;

	void ReleaseBuffer(D3D11_Buffer*& bufferToRelease) const;

	void SetDebugResourceName(D3D11_Buffer*&	bufferToName,	unsigned int debugNameSize, char const* debugName);
	void SetDebugResourceName(VertexBuffer*&	bufferToName,	unsigned int debugNameSize, char const* debugName);
	void SetDebugResourceName(IndexBuffer*&		bufferToName,	unsigned int debugNameSize, char const* debugName);
	void SetDebugResourceName(Texture*&			textureToName,	unsigned int debugNameSize, char const* debugName);
	void SetDebugResourceName(D3D11_Resource*&	resourceToName, unsigned int debugNameSize, char const* debugName);

	void		CopyResource(D3D11_Resource* resourceToCopy, D3D11_Resource* copyDestinationResource);
	void		CopyTextureResource(Texture const* dest, Texture const* src);
	Texture*	GetDefaultTexture() const;

private:
	Texture*	CreateTextureFromFile	(char const* imageFilePath);
	Texture*	CreateTextureFromImage	(Image const& image);
	BitmapFont*	CreateBitmapFont		(char const* imageFilePathWithNoExtension);

protected:
	void* m_dxgiDebugModule = nullptr;
	void* m_dxgiDebug		= nullptr;

	ID3DUserDefinedAnnotation*	m_debugAnnotation		= nullptr;
	ID3D11DepthStencilState*	m_depthStencilState		= nullptr;
	ID3D11DepthStencilView*		m_depthStencilView		= nullptr;
	ID3D11RenderTargetView*		m_renderTargetView		= nullptr;	// Object that holds all the information about the render target. In this case it's the back-buffer
	ID3D11RasterizerState*		m_d3d11RasterizeState	= nullptr;
	ID3D11DeviceContext*		m_d3d11DeviceContext	= nullptr;	// contains information about the drawing capabilities of a device, generates rendering commands
	ID3D11SamplerState*			m_d3d11SamplerState		= nullptr;
	ID3D11BlendState*			m_blendState			= nullptr;
	ID3D11Texture2D*			m_depthStencilTexture	= nullptr;
	IDXGISwapChain*				m_swapChain				= nullptr;	// implements one of more buffers to store rendered data before presenting it to the output
	ID3D11Device*				m_d3d11Device			= nullptr;	// Virtual adapter used to create resources

	BlendMode		m_desiredBlendMode			= BlendMode::ALPHA;
	SamplerMode		m_desiredSamplerMode		= SamplerMode::POINT_CLAMP;
	RasterizerMode	m_desiredRasterizedMode		= RasterizerMode::SOLID_CULL_BACK;
	DepthMode		m_desiredDepthMode			= DepthMode::ENABLED;

	ID3D11BlendState*			m_blendStates[(int)(BlendMode::COUNT)]				=	{};
	ID3D11BlendState*			m_customBlendState									=	nullptr;
	ID3D11SamplerState*			m_samplerStates[(int)(SamplerMode::COUNT)]			=	{};
	ID3D11RasterizerState*		m_rasterizerStates[(int)(RasterizerMode::COUNT)]	=	{};
	ID3D11DepthStencilState*	m_depthStencilStates[(int)(DepthMode::COUNT)]		=	{};

	
	std::vector<Shader*>	m_loadedShaders;
	Shader const*			m_currentShader = nullptr;
	Shader*					m_defaultShader = nullptr;
	
	VertexBuffer*	m_immediateVBO		= nullptr;
	VertexBuffer*	m_immediatePNCUVBO	= nullptr;
	IndexBuffer*	m_immediateIBO		= nullptr;
	ConstantBuffer* m_cameraCBO			= nullptr;
	ConstantBuffer* m_modelCBO			= nullptr;
	ConstantBuffer* m_lightingCBO		= nullptr;

	RendererConfig m_config;

	Texture*						m_defaultTexture = nullptr;
	std::vector<Texture*>			m_loadedTextures;
	std::vector<BitmapFont*>		m_loadedFonts;
	std::vector<D3D11_Resource*>	m_loadedResources;

private:
	// Should this be a pointer
	Light m_lights[MAX_LIGHTS];
};


//--------------------------------------------------------------------------------------------------
struct ID3D11UnorderedAccessView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
struct ID3D11Texture1D;
struct ID3D11Texture2D;
struct ID3D11Texture3D;
struct ID3D11Buffer;


//--------------------------------------------------------------------------------------------------
struct D3D11_ResourceConfig
{
	ResourceViewFormat		m_format						=	ResourceViewFormat::DXGI_FORMAT_R32G32B32A32_FLOAT;
	ResourceBindFlag		m_bindFlags						=	ResourceBindFlag::SHADER_RESOURCE;
	UAVResourceFlag 		m_uavResourceFlag				=	UAVResourceFlag::COUNTER;
	ResourceUsage			m_usageFlag						=	ResourceUsage::GPU_READ_GPU_WRITE;
	ResourceType			m_type							=	ResourceType::TEXTURE2D;
	bool					m_isStandard					=	false;
	bool					m_canDepthBeReadOnly			=	false;
	unsigned int			m_width							=	(unsigned int)-1;
	unsigned int			m_height						=	(unsigned int)-1;
	unsigned int			m_depth							=	(unsigned int)-1;
	unsigned int			m_numOfElements					=	0;
	unsigned int			m_elementStride					=	0;
	unsigned int			m_mipLevels						=	1;
	unsigned int			m_numOfSlices					=	1;
	unsigned int			m_multiSampleCount				=	1;
	unsigned int			m_multiSampleQuality			=	0;
	unsigned int			m_sizeOfTexelInBytes			=	0;
	unsigned int			m_debugNameSize					=	0;
	char const*				m_debugName						=	"None";
	void*					m_defaultInitializationData		=	nullptr;
};


//--------------------------------------------------------------------------------------------------
class D3D11_Resource
{
	friend class Renderer;
private:
	D3D11_Resource() = default;
	~D3D11_Resource();

protected:
	ResourceType m_resourceType	= ResourceType::INVALID;
	
	// Potentially try using anonymous unions
	IntVec3			m_textureDims;
	unsigned int	m_numOfElements = 0;
	unsigned int	m_elementStride = 0;
	
	// Another Anonymous union?
	ID3D11Texture1D*	m_texture1d		=	nullptr;
	ID3D11Texture2D*	m_texture2d		=	nullptr;
	ID3D11Texture3D*	m_texture3d		=	nullptr;
	ID3D11Buffer*		m_buffer		=	nullptr;
	
	ID3D11RenderTargetView*		m_renderTargetView				=	nullptr;
	ID3D11DepthStencilView*		m_depthStencilView				=	nullptr;
	ID3D11DepthStencilView*		m_readOnlyDepthStencilView		=	nullptr;
	ID3D11ShaderResourceView*	m_shaderResourceView			=	nullptr;
	ID3D11UnorderedAccessView*	m_unorderedAccessView			=	nullptr;
};