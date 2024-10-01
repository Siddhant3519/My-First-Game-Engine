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
#include "Engine/Math/Vec4.hpp"


//--------------------------------------------------------------------------------------------------
class	Player;
struct	Quad;


//--------------------------------------------------------------------------------------------------
class Clock;
class Shader;
class IndexBuffer;
class VertexBuffer;
class D3D11_Buffer;
class ConstantBuffer;
class D3D11_Resource;


//--------------------------------------------------------------------------------------------------
enum GameMode : unsigned char
{
	GAME_MODE_RAY_MARCHING	=	0, 
	GAME_MODE_DYNAMIC_ENV,
	GAME_MODE_DEBUG_SDF_RAYMARCHER,
	GAME_MODE_COUNT,
};


//--------------------------------------------------------------------------------------------------
// Constants/Global Variables
constexpr float			DEGREES_ROTATED_PER_SEC			=	10.f;
constexpr float			INVERSE_DEGREE_ROTATED_PER_SEC	=	1.f / DEGREES_ROTATED_PER_SEC;
constexpr float			SEC_PER_FULL_ROTATION			=	360.f * INVERSE_DEGREE_ROTATED_PER_SEC;
constexpr float			DAWN_DURATION_SEC				=	SEC_PER_FULL_ROTATION * 0.1667f;
constexpr float			DAY_DURATION_SEC				=	SEC_PER_FULL_ROTATION * 0.3333f;
constexpr float			DUSK_DURATION_SEC				=	SEC_PER_FULL_ROTATION * 0.1667f;
// constexpr float POST_DUSK_DURATION_SEC				=	SEC_PER_FULL_ROTATION * 0.125f;
constexpr float NIGHT_DURATION_SEC						=	SEC_PER_FULL_ROTATION * 0.3333f;
// constexpr float PRE_DAWN_DURATION_SEC				=	SEC_PER_FULL_ROTATION * 0.125f;

// Foliage related constants
constexpr unsigned int  GRASS_BLADE_INSTANCE_COUNT		=	uint32_t(64 * 1024 * 4 * 2);
constexpr unsigned int  NUM_GRASS_SEGMENTS				=	6;
constexpr uint8_t		INDEX_COUNT_PER_TRI				=	3;
constexpr uint8_t		INDEX_COUNT_PER_SEGMENT_FACE	=	INDEX_COUNT_PER_TRI * 2;
constexpr uint32_t		INDEX_COUNT_PER_GRASS_BLADE		=	NUM_GRASS_SEGMENTS * INDEX_COUNT_PER_SEGMENT_FACE * 2; // Multiply by 2 to take front and back side into account
constexpr uint32_t		UNIQUE_VERTS_PER_SEGMENT_FACE	=	4;
constexpr uint32_t		UNIQUE_VERTS_PER_FACE			=	UNIQUE_VERTS_PER_SEGMENT_FACE + ((NUM_GRASS_SEGMENTS - 1) * 2);
constexpr float			GRASS_WIDTH						=	0.25f;	
constexpr float			GRASS_HEIGHT					=	2.f;	
constexpr float			GRASS_PATCH_SIZE				=	47.5f;
constexpr int			TERRAIN_RES						=	96;


static Rgba8 DAWN_COLOR			=	Rgba8(255, 180, 100);
static Rgba8 DAY_COLOR			=	Rgba8(135, 206, 250);
static Rgba8 DUSK_COLOR			=	Rgba8(255, 150, 100);
static Rgba8 POST_DUSK_COLOR	=	Rgba8(255, 200, 150);
static Rgba8 NIGHT_COLOR		=	Rgba8(25, 25, 112);
static Rgba8 PRE_DAWN_COLOR		=	Rgba8(50, 50, 100);


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
	void Render()						const;
	void RenderRayMarchingMode()		const;
	void RenderDynamicEnvMode()			const;
	void RenderDebugSDFRaymarcher()		const;
	void RenderDebugInfo()				const;
	
	// General (const)
	Vec4  GetSkyColor()																const;
	Rgba8 GetVertexColorBasedOnHeight(float terrainHeight, float terrainMaxHeight)	const;

	// Initialization methods
	void InitializeConstantBuffers();
	void InitializeRayMarchingCanvas();
	void InitializeResources();
	void InitializeShaders();
	void CreateAndPopulateTerrainBuffers(IntVec2 const& terrainRes);
	void CreateAndPopulateReflectionPlaneBuffers(IntVec2 const& reflectionPlaneRes);
	void CreateAndPopulateFoliageBuffers();


public:
	Clock*		m_clock		=	nullptr;
	Player*		m_player	=	nullptr;
	
private:
	// Vertex and Index Buffers
	VertexBuffer*	m_rayMarchingCanvasVB		=	nullptr;
	IndexBuffer*	m_rayMarchingCanvasIB		=	nullptr;
	VertexBuffer*	m_terrainVB					=	nullptr;
	IndexBuffer*	m_terrainIB					=	nullptr;
	VertexBuffer*	m_reflectionPlaneVB			=	nullptr;
	IndexBuffer*	m_reflectionPlaneIB			=	nullptr;
	D3D11_Buffer*	m_foliageInstanceDataVB		=	nullptr;
	D3D11_Buffer*	m_foliageInstanceDataIB		=	nullptr;

	// Constant Buffers
	ConstantBuffer*		m_gameConstantsCB				=	nullptr;
	ConstantBuffer*		m_foliageConstantsCB			=	nullptr;

	// Shaders
	Shader*				m_rayMarchingShader				=	nullptr;
	Shader*				m_skyBoxShader					=	nullptr;
	Shader*				m_reflectionPlaneShader			=	nullptr;
	Shader*				m_foliageShader					=	nullptr;
	Shader*				m_terrainShader					=	nullptr;
	Shader*				m_debugSDFRaymarchingShader		=	nullptr;

	// Resources
	D3D11_Resource*		m_testUVTexture					=	nullptr;
	D3D11_Resource*		m_skyBoxCubeMapResource			=	nullptr;
	D3D11_Resource*		m_foliageStiffnessResource		=	nullptr;

	// General
	Camera				m_screenCamera					=	{ };
	EulerAngles			m_direcLightOrientationDegrees;
	float				m_clientAspect;
	GameMode			m_currentGameMode				=	GameMode::GAME_MODE_RAY_MARCHING;
	
public:
	bool m_isDebug				=	false;
	bool m_isDebugCameraLocked	=	false;

};


//--------------------------------------------------------------------------------------------------
struct Quad
{
	Vec3 m_mins;
	Vec3 m_maxs;
};


//--------------------------------------------------------------------------------------------------
struct GameConstants
{
	float	m_elapsedTimeSeconds	=	0.f;
	Vec3	m_lightDir;
	// 16 byte aligned
	Mat44	m_camRotationMatrix;	
	// 16 byte aligned
	Vec3	m_camPosition;
	float	m_camAspect				=	0.f;
	// 16 byte aligned
	Vec2	m_screenRes;
	float	m_fovScale				=	1.f;
	float	m_nearPlaneDist			=	0.f;
	// 16 byte aligned
	Vec4	m_skyColor;
	Vec4	m_viewDir;
	// 16 byte aligned
};
static int const g_gameConstantsSlot = 6;


//--------------------------------------------------------------------------------------------------
struct FoliageConstants
{
	float m_numGrassSegments		=	NUM_GRASS_SEGMENTS;
	float m_grassPatchSize			=	GRASS_PATCH_SIZE;
	float m_grassWidth				=	GRASS_WIDTH;
	float m_grassHeight				=	GRASS_HEIGHT;
	// 16 byte aligned
};
static int const g_foliageConstantsSlot = 7;