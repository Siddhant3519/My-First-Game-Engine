#pragma once

//--------------------------------------------------------------------------------------------------
#pragma warning(disable : 26812)	// disable prefer enum class over enum warning


//--------------------------------------------------------------------------------------------------
#include <string>


//--------------------------------------------------------------------------------------------------
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"


//--------------------------------------------------------------------------------------------------
class Player;


//--------------------------------------------------------------------------------------------------
class Clock;
class Shader;
class IndexBuffer;
class VertexBuffer;
class ConstantBuffer;
class D3D11_Resource;


//--------------------------------------------------------------------------------------------------
enum eGameMode : unsigned char
{
	eGAME_MODE_DEPTH_TEST = 0,
	eGAME_MODE_ALPHA_BLENDING,
	eGAME_MODE_UAV_WRITES,
	eGAME_MODE_OIT,
	eGAME_MODE_COUNT,
};


//--------------------------------------------------------------------------------------------------
enum eDepthTestMode : unsigned char
{
	eDEPTH_TEST_MODE_LESS = 0,
	eDEPTH_TEST_MODE_GREATER,
	eDEPTH_TEST_MODE_DISABLED,
	eDEPTH_TEST_MODE_COUNT,
	eDEPTH_TEST_MODE_EARLY_DEPTH_TEST,
};


//--------------------------------------------------------------------------------------------------	
enum eAlphaBlendingMode : unsigned char
{
	eALPHA_BLENDING_MODE_OVER = 0,
	eALPHA_BLENDING_MODE_UNDER,
	eALPHA_BLENDING_MODE_COUNT,
	eALPHA_BLENDING_MODE_PREMULTIPLIED_OVER,
	eALPHA_BLENDING_MODE_PREMULTIPLIED_UNDER,
};


//--------------------------------------------------------------------------------------------------
enum eUAVWritesMode : unsigned char
{
	eUAV_WRITE_MODE_WITHOUT_ROV = 0,
	eUAV_WRITE_MODE_WITH_ROV,
	eUAV_WRITE_MODE_COUNT,
};


//--------------------------------------------------------------------------------------------------
enum eOITMode : unsigned char
{
	eOIT_MODE_WORST_CASE = 0,
	eOIT_MODE_CPU_SORTED,
	eOIT_MODE_DEPTH_PEELING,
	eOIT_MODE_VIRTUAL_PIXEL_MAPS,
	eOIT_MODE_WEIGHTED_BLENDED,
	eOIT_MODE_PER_PIXEL_LINKED_LIST,
	eOIT_MODE_MULTI_LAYERED_ALPHA_BLENDING,
	eOIT_MODE_COUNT,
};

 
//--------------------------------------------------------------------------------------------------
enum eTranslucentMode : unsigned char
{
	eTRANSLUCENT_MODE_WORST_CASE = 0,
	eTRANSLUCENT_MODE_SORTED_DRAW_CALLS,
	eTRANSLUCENT_MODE_COUNT,
};


//--------------------------------------------------------------------------------------------------
enum eTranslucentScene : unsigned char
{
	eTRANSLUCENT_SCENE_TWO_QUADS = 0,
	eTRANSLUCENT_SCENE_MULTIPLE_QUADS,
	eTRANSLUCENT_SCENE_BUNNIES,
	eTRANSLUCENT_SCENE_TEAPOTS,
	eTRANSLUCENT_SCENE_FRIDGE,
	eTRANSLUCENT_SCENE_CAR,
	eTRANSLUCENT_SCENE_INTERSECTING_MESHES,
	eTRANSLUCENT_SCENE_FOG,
	eTRANSLUCENT_SCENE_COUNT,
};


//--------------------------------------------------------------------------------------------------
enum eSplitScreenQuadrant : unsigned char
{
	eSPLIT_SCREEN_QUADRANT_1 = 0,
	eSPLIT_SCREEN_QUADRANT_2,
	eSPLIT_SCREEN_QUADRANT_3,
	eSPLIT_SCREEN_QUADRANT_4,
};


//--------------------------------------------------------------------------------------------------
enum eMeshType : unsigned char
{
	eMESH_TYPE_QUAD = 0,
	eMESH_TYPE_UNI_TEXTURED_OBB,
	eMESH_TYPE_MULTI_TEXTURED_OBB,
	eMESH_TYPE_SPHERE,
	eMESH_TYPE_TEPOT,
	eMESH_TYPE_BUNNY,
	eMESH_TYPE_FRIDGE_DOOR,
	eMESH_TYPE_FRIDGE_BODY,
	eMESH_TYPE_BILLBOARDED_QUAD,
	eMESH_TYPE_CAR_MAIN_BODY,
	eMESH_TYPE_CAR_GLASS_1,
	eMESH_TYPE_CAR_GLASS_2,
	eMESH_TYPE_COUNT,
};


//--------------------------------------------------------------------------------------------------
enum eRenderPass : unsigned char
{
	eRENDER_PASS_OPAQUE = 0,
	eRENDER_PASS_TRANSLUCENT,
};


//--------------------------------------------------------------------------------------------------
enum eNodes : unsigned char
{
	eNODES_2,
	eNODES_4,
	eNODES_32,
	eNODES_COUNT,
};


//--------------------------------------------------------------------------------------------------	
struct SceneObject
{
	IndexBuffer*				m_indexBuffer				=	nullptr;
	VertexBuffer*				m_vertexBuffer				=	nullptr;
	D3D11_Resource*				m_textureResource			=	nullptr;
	wchar_t const*				m_debugName					=	nullptr;
	unsigned int				m_numOfTexturesBound		=	1;
	unsigned int				m_numOfIndexes				=	0;
	Vec2						m_billboardedQuadHalfDims;
	Mat44						m_transform;
	Vec3						m_center;
	Rgba8						m_color;
	eMeshType					m_type					=	eMESH_TYPE_QUAD;
};


//--------------------------------------------------------------------------------------------------
struct MeshInfo
{
	D3D11_Resource*		m_textureResource	  =		nullptr;
	unsigned int		m_numOfTexturesBound  =		1;
	Rgba8				m_color;
	bool				m_isOpaque			  =		false;
};


//--------------------------------------------------------------------------------------------------
struct Scene
{
	std::vector<unsigned int>	m_sceneIndexes;
	std::vector<Vertex_PCU>		m_sceneVertices;
	std::vector<MeshInfo>		m_meshInfoList;
	std::vector<SceneObject>	m_translucentObjects;
	std::vector<SceneObject>	m_opaqueObjects;
	std::vector<SceneObject>	m_sortedTranslucentObjects;
};


//--------------------------------------------------------------------------------------------------
class Game
{
public:
	Game(float clientAspect);
	~Game();

	void Startup();
	void Shutdown();
	void KeyboardUpdate();
	void Update();
	void Render() const;

	// Update helper methods
	void GameModeUpdate();

	// OIT Update
	void OITUpdate();
	void UpdateOITSubModeQuadrant(unsigned char subgroupMode);

	// Render helper methods
	void RenderDebugInfo()																const;
	void RenderGameModeInfo(eSplitScreenQuadrant quadrant = eSPLIT_SCREEN_QUADRANT_1)	const;
	void GameModeRender()																const;
	void DepthTestSubModeRender()														const;
	void AlphaBlendingSubModeRender()													const;
	void UAVWritesSubModeRender()														const;
	void OITSubModeRender()																const;
	void DepthTestSubModeQuandrantRender(unsigned char subgroupMode, eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)		const;
	void AlphaBlendingSubModeQuadrantRender(unsigned char subgroupMode, eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)	const;
	void UAVWritesSubModeQuadrantRender(unsigned char subgroupMode, eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)		const;
	void OITSubModeQuadrantRender(unsigned char subgroupMode, eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)				const;
	void RenderQuadrants() const;

	// Depth Test sub mode render helper methods
	void LessThanEqualDepthTestSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)		const;
	void GreaterThanEqualDepthTestSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)	const;
	void DisabledDepthTestSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)			const;
	void EarlyDepthTestSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)				const;

	// Alpha Blending sub mode render helper methods
	void OverAlphaBlendingSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)				const;
	void UnderAlphaBlendingSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)				const;
	void PremultipliedOverAlphaBlendingSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)	const;
	void PremultipliedUnderAlphaBlendingSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)	const;

	// UAV Writes sub mode render helper methods
	void WithoutROVUAVWritesSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)	const;
	void WithROVUAVWritesSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)	const;

	// OIT Sub mode render helper methods
	void WorstCaseOITSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)				const;
	void CPUSortedOITSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)				const;
	void DepthPeelingOITSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)				const;
	void VirtualPixelMapsOITSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)			const;
	void WeightedBlendedOITSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)			const;
	void PerPixelLinkedListOITSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)		const;
	void MultiLayerAlphaBlendingOITSubModeRender(eSplitScreenQuadrant quadrantToRender = eSPLIT_SCREEN_QUADRANT_1)	const;
	
	// Depth Peeling helper methods
	void CompositeDepthPeelingDepth(D3D11_Resource* depthResource)	const;
	void CompositeDepthPeelingColor(D3D11_Resource* colorTarget)	const;

	// Virtual Pixel Maps helper methods
	void CompositeVPMDepth(D3D11_Resource*& depthResource)	const;
	void CompositeVPMColor(D3D11_Resource*& colorTarget)	const;

	// Scene render helper methods
	void DrawScene(eRenderPass renderPass)						const;
	void DrawSortedTranslucentScene()							const;
	// void OpaqueRenderPass(eTranslucentScene sceneToDraw)		const;
	// void TranslucentRenderPass(eTranslucentScene sceneToDraw)	const;

	// General utility methods
	char*				GetGameModeAsText()																				const;
	char*				GetTranslucentModeAsText()																		const;
	char*				GetTranslucentSceneAsText()																		const;
	char*				GetControlledQuadrantAsText()																	const;
	char*				GetDepthTestSubGroupModeAsText(unsigned char subGroupMode = 0)									const;
	std::string			GetGameModeAndGroupModeAsText(eSplitScreenQuadrant quadrant = eSPLIT_SCREEN_QUADRANT_1)			const;
	char*				GetDepthTestSubGroupModeAsText(eSplitScreenQuadrant quadrant = eSPLIT_SCREEN_QUADRANT_1)		const;
	char*				GetAlphaBlendingSubGroupModeAsText(eSplitScreenQuadrant quadrant = eSPLIT_SCREEN_QUADRANT_1)	const;
	char*				GetAlphaBlendingSubGroupModeAsText(unsigned char subGroupMode = 0)								const;
	char*				GetUAVWritesSubGroupModeAsText(eSplitScreenQuadrant quadrant = eSPLIT_SCREEN_QUADRANT_1)		const;
	char*				GetUAVWritesSubGroupModeAsText(unsigned char subGroupMode = 0)									const;
	char*				GetOITSubGroupModeAsText(eSplitScreenQuadrant quadrant = eSPLIT_SCREEN_QUADRANT_1)				const;
	char*				GetOITSubGroupModeAsText(unsigned char subGroupMode = 0)										const;
	char*				GetTranslucentModeAsText(eTranslucentMode translucentMode)										const;
	D3D11_Resource*		GetRenderTargetFromQuadrantInfo(eSplitScreenQuadrant quadrant)									const;
	D3D11_Resource*		GetDepthResourceFromQuadrantInfo(eSplitScreenQuadrant quadrant)									const;
	eTranslucentScene	GetSceneFromName(std::string const& sceneName)													const;
	eMeshType			GetMeshTypeFromText(std::string const& meshTypeAsText)											const;
	eTranslucentScene	GetSceneFromQuadrant(eSplitScreenQuadrant quadrantToRender)										const;
	bool				IsNodeBasedOITMode(eSplitScreenQuadrant quadrant = eSPLIT_SCREEN_QUADRANT_1)					const;
	bool				IsNodeBasedOITMode(unsigned char subGroupMode = 0)												const;
	eNodes				GetNodesFromQuadrant(eSplitScreenQuadrant quadrant)												const;
	void				AddSceneObject(SceneObject* sceneObjectToAdd);
	void				UpdateNodeCount(eNodes& nodesToUpdate);

	void GoToPreviousValidMode(eSplitScreenQuadrant quadrantInControl, unsigned char subgroupMaxElement);
	void GoToNextValidMode(eSplitScreenQuadrant quadrantInControl, unsigned char subgroupMaxElement);
	void GoToPreviousValidMode(unsigned char& quadrantToUpdate, unsigned char subgroupMaxElement);
	void GoToNextValidMode(unsigned char& quadrantToUpdate, unsigned char subgroupMaxElement);
	void GoToPreviousTranslucentMode(eTranslucentMode& translucentMode);
	void GoToNextTranslucentMode(eTranslucentMode& translucentMode);
	void CreateMeshOfType(SceneObject*& out_createdMesh, eMeshType meshTypeToCreate);

	// Commands
	static bool Command_SetDepthPeelCount(EventArgs& args);


	// Initialization methods
	void InitializeSceneFromElement(XmlElement const& sceneDef);
	void InitializeDepthTestModeSceneObjects();
	void InitializeVertexAndIndexBuffers();
	void InitializeScenesFromDefinition();
	void InitializeCustomBlendModes();
	void InitializeGridBuffer();
	void InitializeResources();
	void InitializeShaders();
	void InitializeMeshes();

private:
	D3D11_Resource*				m_quadrant1RenderTarget									=	nullptr;
	D3D11_Resource*				m_quadrant2RenderTarget									=	nullptr;
	D3D11_Resource*				m_quadrant3RenderTarget									=	nullptr;
	D3D11_Resource*				m_quadrant4RenderTarget									=	nullptr;
	D3D11_Resource*				m_quadrant1DepthTarget									=	nullptr;
	D3D11_Resource*				m_quadrant2DepthTarget									=	nullptr;
	D3D11_Resource*				m_quadrant3DepthTarget									=	nullptr;
	D3D11_Resource*				m_quadrant4DepthTarget									=	nullptr;
	D3D11_Resource*				m_intermediateTarget									=	nullptr;
	D3D11_Resource*				m_intermediateRenderTarget								=	nullptr;
	D3D11_Resource*				m_opaqueBackgroundQuadrant1RenderTarget					=	nullptr;
	D3D11_Resource*				m_depthPeelingIntermediateDepthTarget					=	nullptr;
	D3D11_Resource*				m_depthPeelingIntermediateRenderTarget					=	nullptr;
	D3D11_Resource*				m_fragmentsPerPixelStructuredBuffer_32					=	nullptr;
	D3D11_Resource*				m_headNodeByteAddressBuffer_32							=	nullptr;
	D3D11_Resource*				m_fragmentsPerPixelStructuredBuffer_4					=	nullptr;
	D3D11_Resource*				m_headNodeByteAddressBuffer_4							=	nullptr;
	D3D11_Resource*				m_fragmentsPerPixelStructuredBuffer_2					=	nullptr;
	D3D11_Resource*				m_headNodeByteAddressBuffer_2							=	nullptr;
	D3D11_Resource*				m_accumulationTargetWeightedBlended						=	nullptr;
	D3D11_Resource*				m_revealageTargetWeightedBlended						=	nullptr;
	D3D11_Resource*				m_unoptimizedMLABUntouchedFragmentMask					=	nullptr;
	D3D11_Resource*				m_unoptimizedMLABStructuredBuffer_2						=	nullptr;
	D3D11_Resource*				m_unoptimizedMLABStructuredBuffer_4						=	nullptr;
	D3D11_Resource*				m_unoptimizedMLABStructuredBuffer_32					=	nullptr;

	VertexBuffer*				m_fullScreenQuadVB										=	nullptr;
	IndexBuffer*				m_fullScreenQuadIB										=	nullptr;
	VertexBuffer*				m_quadrant1QuadVB										=	nullptr;
	IndexBuffer*				m_quadrant1QuadIB										=	nullptr;
	VertexBuffer*				m_quadrant2QuadVB										=	nullptr;
	IndexBuffer*				m_quadrant2QuadIB										=	nullptr;
	VertexBuffer*				m_quadrant3QuadVB										=	nullptr;
	IndexBuffer*				m_quadrant3QuadIB										=	nullptr;
	VertexBuffer*				m_quadrant4QuadVB										=	nullptr;
	IndexBuffer*				m_quadrant4QuadIB										=	nullptr;
	VertexBuffer*				m_quadrant1Split2QuadVB									=	nullptr;
	IndexBuffer*				m_quadrant1Split2QuadIB									=	nullptr;
	VertexBuffer*				m_quadrant2Split2QuadVB									=	nullptr;
	IndexBuffer*				m_quadrant2Split2QuadIB									=	nullptr;
	ConstantBuffer*				m_gameConstantBuffer									=	nullptr;
	ConstantBuffer*				m_screenConstantBuffer									=	nullptr;

	Scene*						m_scenes[eTRANSLUCENT_SCENE_COUNT]						=	{ };

	VertexBuffer*				m_gridVertexBuffer										=	nullptr;
	Shader*						m_defaultShader											=	nullptr;
	Shader*						m_vpmPeelingShader										=	nullptr;
	Shader*						m_vpmDepthCompositeShader								=	nullptr;
	Shader*						m_vpmColorCompositeShader								=	nullptr;
	Shader*						m_depthPeelingShader									=	nullptr;
	Shader*						m_depthPeelingDepthCompositeShader						=	nullptr;
	Shader*						m_depthPeelingColorCompositeShader						=	nullptr;
	Shader*						m_underCompositeShader									=	nullptr;
	Shader*						m_uavWriteWithROVShader									=	nullptr;
	Shader*						m_uavWriteWithoutROVShader								=	nullptr;
	Shader*						m_defaultPremultipliedAlphaShader						=	nullptr;
	Shader*						m_depthPeelingFinalBackgroundPassShader					=	nullptr;
	Shader*						m_populatePerPixelLinkedListShader						=	nullptr;
	Shader*						m_perPixelLinkedListCompositePassShader_32				=	nullptr;
	Shader*						m_perPixelLinkedListClearBufferShader_32				=	nullptr;
	Shader*						m_populatePerPixelLinkedListShader_4					=	nullptr;
	Shader*						m_perPixelLinkedListCompositePassShader_4				=	nullptr;
	Shader*						m_perPixelLinkedListClearBufferShader_4					=	nullptr;
	Shader*						m_populatePerPixelLinkedListShader_2					=	nullptr;
	Shader*						m_perPixelLinkedListCompositePassShader_2				=	nullptr;
	Shader*						m_perPixelLinkedListClearBufferShader_2					=	nullptr;
	Shader*						m_populateAccumRevealRTsShader							=	nullptr;
	Shader*						m_weightedBlendedCompositeShader						=	nullptr;
	Shader*						m_unoptimizedMLABPopulateBlendingArrayShader_2			=	nullptr;
	Shader*						m_unoptimizedMLABPopulateRenderTargetShader_2			=	nullptr;
	Shader*						m_unoptimizedMLABPopulateBlendingArrayShader_4			=	nullptr;
	Shader*						m_unoptimizedMLABPopulateRenderTargetShader_4			=	nullptr;
	Shader*						m_unoptimizedMLABPopulateBlendingArrayShader_32			=	nullptr;
	Shader*						m_unoptimizedMLABPopulateRenderTargetShader_32			=	nullptr;

	Camera						m_screenCamera											=	{ };
	Vec3						m_prevPlayerCamPos;
	EulerAngles					m_prevPlayerCamOrientation;
	float						m_clientAspect;
	unsigned int				m_numOfDepthPeelingPasses								=	40;
	eGameMode					m_gameMode												=	eGAME_MODE_OIT;
	bool						m_isDebug												=	false;
	bool						m_isTwoSplitScreen										=	false;
	bool						m_isFourSplitScreen										=	false;
	bool						m_isGameModeLocked										=	false;
	bool						m_isTextured											=	true;
	bool						m_isDepthTestEnabled									=	false;
	bool						m_isRetainedModeRendering								=	true;
	eTranslucentScene			m_sceneIndex											=	eTRANSLUCENT_SCENE_TWO_QUADS;
	eTranslucentMode			m_translucentModeQuadrant1								=	eTRANSLUCENT_MODE_WORST_CASE;
	eTranslucentMode			m_translucentModeQuadrant2								=	eTRANSLUCENT_MODE_WORST_CASE;
	eTranslucentMode			m_translucentModeQuadrant3								=	eTRANSLUCENT_MODE_WORST_CASE;
	eTranslucentMode			m_translucentModeQuadrant4								=	eTRANSLUCENT_MODE_WORST_CASE;

	eSplitScreenQuadrant		m_quadrantInControl										=	eSPLIT_SCREEN_QUADRANT_1;
	unsigned char				m_quadrant1SubgroupMode									=	0;
	unsigned char				m_quadrant2SubgroupMode									=	0;
	unsigned char				m_quadrant3SubgroupMode									=	0;
	unsigned char				m_quadrant4SubgroupMode									=	0;
	eNodes						m_quadrant1Nodes										=	eNODES_2;
	eNodes						m_quadrant2Nodes										=	eNODES_2;
	eNodes						m_quadrant3Nodes										=	eNODES_2;
	eNodes						m_quadrant4Nodes										=	eNODES_2;
public:
	Clock*		m_clock		=	nullptr;
	Player*		m_player	=	nullptr;
};


//--------------------------------------------------------------------------------------------------
struct Fragment
{
	unsigned int m_color			=	UINT32_MAX;
	unsigned int m_transmission		=	UINT32_MAX; // (1 - opacity)
	unsigned int m_depth			=	UINT32_MAX;
	unsigned int m_nextIndex		=	UINT32_MAX;
};


//--------------------------------------------------------------------------------------------------
struct GameConstants
{
	float			m_screenWidth;
	float			pad0;
	unsigned int	m_uintMax;
	float			m_cameraFar;
};
static int const g_gameConstantsSlot = 8;


//--------------------------------------------------------------------------------------------------
struct ScreenConstants
{
	float m_screenWidth;
	Vec3  m_padding;
};
static int const g_screenConstantsSlot = 5;