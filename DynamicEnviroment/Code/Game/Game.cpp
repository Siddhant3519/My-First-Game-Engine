#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"


//--------------------------------------------------------------------------------------------------
#include "Engine/Renderer/RendererAnnotationJanitor.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/D3D11_Buffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Clock.hpp"


//--------------------------------------------------------------------------------------------------
extern InputSystem*		g_theInput;
extern BitmapFont*		g_theFont;
extern Renderer*		g_theRenderer;
extern Window*			g_theWindow;
extern App*				g_theApp;


//--------------------------------------------------------------------------------------------------
Game* g_theGame = nullptr;


//--------------------------------------------------------------------------------------------------
static unsigned int s_threadGroupX;
static unsigned int s_threadGroupY;


//--------------------------------------------------------------------------------------------------
Game::Game(float clientAspect) 
	: m_clientAspect(clientAspect)
{
	g_theGame = this;
	m_clock = new Clock(*g_theApp->m_clock);
}


//--------------------------------------------------------------------------------------------------
Game::~Game()
{
	delete m_clock;
	m_clock = nullptr;

	delete m_rayMarchingCanvasVB;
	m_rayMarchingCanvasVB = nullptr;
	delete m_rayMarchingCanvasIB;
	m_rayMarchingCanvasIB = nullptr;

	delete m_terrainVB;
	m_terrainVB = nullptr;
	delete m_terrainIB;
	m_terrainIB = nullptr;

	delete m_reflectionPlaneVB;
	m_reflectionPlaneVB = nullptr;
	delete m_reflectionPlaneIB;
	m_reflectionPlaneIB = nullptr;

	g_theRenderer->ReleaseBuffer(m_foliageInstanceDataVB);
	g_theRenderer->ReleaseBuffer(m_foliageInstanceDataIB);


	delete m_gameConstantsCB;
	m_gameConstantsCB = nullptr;

	delete m_foliageConstantsCB;
	m_foliageConstantsCB = nullptr;
}


//--------------------------------------------------------------------------------------------------
void Game::Startup()
{
	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(1920.f, 1080.f));
	m_player = new Player(this, Vec3(0.f, 0.f, 0.f), m_clientAspect);
	
	m_direcLightOrientationDegrees = EulerAngles(90.f, 0.f, 0.f);

	InitializeRayMarchingCanvas();
	InitializeConstantBuffers();
	InitializeResources();
	InitializeShaders();

	// For compute shader use
	IntVec2 clientDimensions	=	g_theWindow->GetClientDimensions();
	s_threadGroupX				=	clientDimensions.x / 8;
	s_threadGroupY				=	clientDimensions.y / 8;
}


//--------------------------------------------------------------------------------------------------
void Game::Shutdown()
{
}


//--------------------------------------------------------------------------------------------------
void Game::KeyboardUpdate()
{
	if (g_theInput->IsKeyDown_WasUp(KEYCODE_F1))
	{
		m_isDebug				=	!m_isDebug;
		if (!m_isDebug)
		{
			m_isDebugCameraLocked	=	false;
		}
	}

	// Since only two game modes are present let's stick to num keys to switch game modes
	if (g_theInput->IsKeyDown_WasUp('1'))
	{
		m_currentGameMode = GAME_MODE_RAY_MARCHING;
	}
	if (g_theInput->IsKeyDown_WasUp('2'))
	{
		m_currentGameMode = GAME_MODE_DYNAMIC_ENV;
		Vec3 playerNewPos(0.f, 0.f, 15.f);
		m_player->m_camera.SetPositionOnly(playerNewPos);
	}
	if (g_theInput->IsKeyDown_WasUp('3'))
	{
		m_currentGameMode = GAME_MODE_DEBUG_SDF_RAYMARCHER;
	}


	if (g_theInput->IsKeyDown_WasUp('L'))
	{
		m_isDebugCameraLocked = !m_isDebugCameraLocked;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::Update()
{
	KeyboardUpdate();
	m_player->Update();
	m_direcLightOrientationDegrees.m_pitchDegrees	=	fmodf((m_direcLightOrientationDegrees.m_pitchDegrees + m_clock->GetDeltaSeconds() * DEGREES_ROTATED_PER_SEC), 360.f);
	Vec3 directionalLightForward					=	m_direcLightOrientationDegrees.GetXForward();
	directionalLightForward							=	Vec3(-directionalLightForward.y, directionalLightForward.z, directionalLightForward.x);
	float totalSecondsElapsed						=	m_clock->GetTotalSeconds();

	// Update game constants constant buffer
	Camera	const& playerCam				=	m_player->m_camera;
	IntVec2 const& windowDims				=	g_theWindow->GetClientDimensions();
	Vec3  playerCamViewDir					=	playerCam.GetCameraOrientation().GetXForward();
	static GameConstants gameConstants		=	{ }; 
	gameConstants.m_elapsedTimeSeconds		=	totalSecondsElapsed;
	gameConstants.m_lightDir				=	-directionalLightForward;
	gameConstants.m_camRotationMatrix		=	playerCam.GetModelMatrix();;
	gameConstants.m_camPosition				=	playerCam.GetCameraPosition();
	gameConstants.m_camAspect				=	m_clientAspect;
	gameConstants.m_screenRes				=	Vec2((float)windowDims.x, (float)windowDims.y);
	gameConstants.m_fovScale				=	m_player->m_fovScale;
	gameConstants.m_nearPlaneDist			=	m_player->m_camera.GetCameraNearClipDist();
	gameConstants.m_skyColor				=	GetSkyColor();
	gameConstants.m_viewDir					=	Vec4(playerCamViewDir.x, playerCamViewDir.y, playerCamViewDir.z, 0.f);
	g_theRenderer->CopyCPUToGPU(&gameConstants, sizeof(GameConstants), m_gameConstantsCB);
}


//--------------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Screen clear color
	Rgba8 clearColor{ 0, 0, 0, 255 };
	g_theRenderer->ClearScreen(clearColor);

	switch (m_currentGameMode)
	{
		case GAME_MODE_RAY_MARCHING:
		{
			RenderRayMarchingMode();
			break;
		}
		case GAME_MODE_DYNAMIC_ENV:
		{
			RenderDynamicEnvMode();
			break;
		}
		case GAME_MODE_DEBUG_SDF_RAYMARCHER:
		{
			RenderDebugSDFRaymarcher();
			break;
		}
		default:
		{
			ERROR_AND_DIE(Stringf("Invalid Game Mode: %d\nPlease enter a valid game mode in range 0 - %d", m_currentGameMode, GameMode::GAME_MODE_COUNT - 1));
			break;
		}
	}
	
	g_theRenderer->EndCamera(m_player->m_camera);

	Camera skyboxCam = m_player->m_camera;
	skyboxCam.SetPositionOnly(Vec3::ZERO);

	// Screen camera (Orthographic camera)
	g_theRenderer->BeginCamera(m_screenCamera);

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants();

	// {
	// 	RendererAnnotationJanitor fullScreenQuadRenderJanitor(L"Render Full Screen Quad");
	// 	// g_theRenderer->BindReadableResources(&m_testUVTexture, 1, 0, BindingLocation::PIXEL_SHADER);
	// 	g_theRenderer->BindShader(m_raymarchingShader);
	// 	g_theRenderer->DrawIndexedBuffer(m_fullScreenQuadIB, m_fullScreenQuadVB, 6);
	// }
	
	// {
	// 	RendererAnnotationJanitor debugInfo(L"Render Debug Info");
	// 	RenderDebugInfo();
	// }
	
	g_theRenderer->EndCamera(m_screenCamera);
}


//--------------------------------------------------------------------------------------------------
void Game::RenderRayMarchingMode() const
{
	Camera const& playerCam = m_player->m_camera;
	Camera const& renderCamera = !m_isDebug ? m_player->m_camera : m_player->m_debugCamera;
	g_theRenderer->BeginCamera(renderCamera);
	
	{
		// RayMarching canvas render
		RendererAnnotationJanitor raymarchCanvasRender(L"RayMarching Canvas Render");

		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(m_rayMarchingShader);
		// g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetModelConstants(m_player->m_camera.GetModelMatrix()); // Move and orient the canvas
		static unsigned int rayMarchingCanvasIndexCount = (unsigned int)m_rayMarchingCanvasIB->m_size / m_rayMarchingCanvasIB->GetStride();
		g_theRenderer->DrawIndexedBuffer(m_rayMarchingCanvasIB, m_rayMarchingCanvasVB, rayMarchingCanvasIndexCount);
		g_theRenderer->SetModelConstants();
	}
	
	g_theRenderer->EndCamera(renderCamera);

	// Debug camera view
	if (m_isDebug)
	{
		g_theRenderer->BeginCamera(m_player->m_debugCamera);

		RendererAnnotationJanitor rayMarchDebugRender(L"RayMarching Debug Render");

		// Player cam related variables
		Vec3	camPos					=	playerCam.GetCameraPosition();
		float	camHalfFovDegrees		=	playerCam.GetPerspectiveCameraFov() * 0.5f;
		float	camAspect				=	playerCam.GetPerspectiveCameraAspect();
		float	nearPlaneDist			=	playerCam.GetCameraNearClipDist();
		float	farPlaneDist			=	playerCam.GetCameraFarClipDist();
		float	nearPlaneHalfHeight		=	nearPlaneDist * TanDegrees(camHalfFovDegrees);
		float	nearPlaneHalfWidth		=	nearPlaneHalfHeight * camAspect;
		// Using similar triangle property to fetch far plane half height and width
		static float distFarOverNear	=	farPlaneDist / nearPlaneDist;
		float	farPlaneHalfHeight		=	nearPlaneHalfHeight * distFarOverNear;
		float	farPlaneHalfWidth		=	farPlaneHalfHeight * camAspect;
		
		// Calculate player cam's near plane bounds
		Vec3	const&	nearPlaneMins	=	Vec3(nearPlaneDist, nearPlaneHalfWidth, -nearPlaneHalfHeight);
		Vec3	const&	nearPlaneMaxs	=	Vec3(nearPlaneDist, -nearPlaneHalfWidth, nearPlaneHalfHeight);
		Vec3	const&	nearPlaneBL		=	nearPlaneMins;
		Vec3	const&	nearPlaneBR		=	Vec3(nearPlaneMaxs.x, nearPlaneMaxs.y, nearPlaneMins.z);
		Vec3	const&	nearPlaneTR		=	nearPlaneMaxs;
		Vec3	const&	nearPlaneTL		=	Vec3(nearPlaneMaxs.x, nearPlaneMins.y, nearPlaneMaxs.z);
		// Calculate player cam's far plane bounds
		Vec3	const&	farPlaneMins	=	Vec3(farPlaneDist, farPlaneHalfWidth, -farPlaneHalfHeight);
		Vec3	const&	farPlaneMaxs	=	Vec3(farPlaneDist, -farPlaneHalfWidth, farPlaneHalfHeight);
		Vec3	const&	farPlaneBL		=	farPlaneMins;
		Vec3	const&	farPlaneBR		=	Vec3(farPlaneMaxs.x, farPlaneMaxs.y, farPlaneMins.z);
		Vec3	const&	farPlaneTR		=	farPlaneMaxs;
		Vec3	const&	farPlaneTL		=	Vec3(farPlaneMaxs.x, farPlaneMins.y, farPlaneMaxs.z);

		// Quick hack, Immediate mode rendering
		static std::vector<Vertex_PCU>		tempVerts;
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(nullptr);

		// Player cam position
		{
			RendererAnnotationJanitor cameraPosRender(L"Player cam location Debug Render");
			AddVertsForUVSphereZ3D(tempVerts, camPos, 0.01f, 4.f, 4.f, Rgba8::TURQUOISE);
		}

		// Player cam view frustum 
		{
			RendererAnnotationJanitor cameraFrustumRender(L"Player camera frustum Render");
			AddVertsForQuad3D(tempVerts, nearPlaneBL, nearPlaneBR, nearPlaneTR, nearPlaneTL, Rgba8::CRIMSON);
			AddVertsForQuad3D(tempVerts, farPlaneBL, farPlaneBR, farPlaneTR, farPlaneTL, Rgba8::CRIMSON);
			AddVertsForLineSegment3D(tempVerts, nearPlaneBL, farPlaneBL, 0.01f, Rgba8::CRIMSON);
			AddVertsForLineSegment3D(tempVerts, nearPlaneBR, farPlaneBR, 0.01f, Rgba8::CRIMSON);
			AddVertsForLineSegment3D(tempVerts, nearPlaneTR, farPlaneTR, 0.01f, Rgba8::CRIMSON);
			AddVertsForLineSegment3D(tempVerts, nearPlaneTL, farPlaneTL, 0.01f, Rgba8::CRIMSON);
		}

		g_theRenderer->SetModelConstants(playerCam.GetModelMatrix());
		g_theRenderer->DrawVertexArray(tempVerts);
		g_theRenderer->SetModelConstants();
		tempVerts.clear();

		g_theRenderer->EndCamera(m_player->m_debugCamera);
	}

}


//--------------------------------------------------------------------------------------------------
void Game::RenderDynamicEnvMode() const
{
	g_theRenderer->BeginCamera(m_player->m_camera);

	{
		RendererAnnotationJanitor terrainRender(L"Terrain Render");
		Mat44 translantionMat;
		translantionMat.SetTranslation3D(Vec3(0.f, 0.f, 0.f));
		g_theRenderer->SetModelConstants(translantionMat, Rgba8(54, 24, 4));
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(m_terrainShader);
		static unsigned int terrainIndexCount = (unsigned int)m_terrainIB->m_size / m_terrainIB->GetStride();
		g_theRenderer->DrawIndexedBuffer(m_terrainIB, m_terrainVB, terrainIndexCount);
		g_theRenderer->SetModelConstants();
	}
	
	// {
	// 	RendererAnnotationJanitor test(L"Test");
	// 	Mat44 translantionMat;
	// 	translantionMat.SetTranslation3D(Vec3(0.f, 0.f, 5.f));
	// 	g_theRenderer->SetModelConstants(translantionMat, Rgba8::BLUE);
	// 	g_theRenderer->BindTexture(nullptr);
	// 	g_theRenderer->BindShader(nullptr);
	// 	static unsigned int terrainIndexCount = (unsigned int)m_terrainIB->m_size / m_terrainIB->GetStride();
	// 	g_theRenderer->DrawIndexedBuffer(m_terrainIB, m_terrainVB, terrainIndexCount);
	// 	g_theRenderer->SetModelConstants();
	// }

	{
		RendererAnnotationJanitor reflectionJanitor(L"Reflection Plane Render");
		Mat44 translantionMat;
		translantionMat.SetTranslation3D(Vec3(0.f, 66.f, 0.f));
		// translantionMat.SetTranslation3D(Vec3(0.f, 0.f, 0.f));
		g_theRenderer->SetModelConstants(translantionMat);
		g_theRenderer->BindReadableResources(&m_skyBoxCubeMapResource, 1, 0, BindingLocation::PIXEL_SHADER);
		g_theRenderer->BindShader(m_reflectionPlaneShader);
		static unsigned int refPlaneIndexCount = (unsigned int)m_reflectionPlaneIB->m_size / m_reflectionPlaneIB->GetStride();
		g_theRenderer->DrawIndexedBuffer(m_reflectionPlaneIB, m_reflectionPlaneVB, refPlaneIndexCount);
		g_theRenderer->SetModelConstants();
	}

	{
		RendererAnnotationJanitor foliageJanitor(L"Foliage Render");
		
		D3D11_Resource* readableResourcesToBind[] = { m_foliageStiffnessResource };
		g_theRenderer->BindReadableResources(readableResourcesToBind, 1, 5, BindingLocation::VERTEX_SHADER);
		g_theRenderer->BindShader(m_foliageShader);
		g_theRenderer->DrawIndexedInstanced(m_foliageInstanceDataIB, m_foliageInstanceDataVB, nullptr, INDEX_COUNT_PER_GRASS_BLADE, GRASS_BLADE_INSTANCE_COUNT);
	}


	Camera skyBoxCam = m_player->m_camera;
	skyBoxCam.SetPositionOnly(Vec3::ZERO);
	g_theRenderer->BeginCamera(skyBoxCam);

	{
		RendererAnnotationJanitor cubeMapRenderJanitor(L"Cube Map Render");
		

		// Quick Hack: Immediate mode rendering (move to retained later)
		static std::vector<Vertex_PCU> tempVertsPCU;
		AABB3 bounds(Vec3::ZERO, 1.f, 1.f, 1.f);
		AddVertsForAABB3D(tempVertsPCU, bounds, Rgba8::WHITE);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_FRONT);
		g_theRenderer->BindReadableResources(&m_skyBoxCubeMapResource, 1, 0, BindingLocation::PIXEL_SHADER);
		g_theRenderer->BindShader(m_skyBoxShader, BindingLocation::PIXEL_SHADER);
		g_theRenderer->DrawVertexArray(tempVertsPCU);
		tempVertsPCU.clear();
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	}
	
	g_theRenderer->EndCamera(skyBoxCam);

	g_theRenderer->EndCamera(m_player->m_camera);
}


//--------------------------------------------------------------------------------------------------
void Game::RenderDebugSDFRaymarcher() const
{
	RendererAnnotationJanitor debugRender(L"SDF Raymarcher Debug Render");

	g_theRenderer->BeginCamera(m_player->m_camera);

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(m_debugSDFRaymarchingShader);
	g_theRenderer->SetModelConstants(m_player->m_camera.GetModelMatrix(), Rgba8(166, 191, 230));
	static unsigned int rayMarchingCanvasIndexCount = (unsigned int)m_rayMarchingCanvasIB->m_size / m_rayMarchingCanvasIB->GetStride();
	g_theRenderer->DrawIndexedBuffer(m_rayMarchingCanvasIB, m_rayMarchingCanvasVB, rayMarchingCanvasIndexCount);
	g_theRenderer->SetModelConstants();

	g_theRenderer->EndCamera(m_player->m_camera);
}


//--------------------------------------------------------------------------------------------------
void Game::RenderDebugInfo() const
{
	// Fetch the screen bounds
	static AABB2 screenCameraBounds =	m_screenCamera.GetCameraAABB();
	static float screenCameraWidth	=	screenCameraBounds.m_maxs.x - screenCameraBounds.m_mins.x; 
	static float screenCameraHeight	=	screenCameraBounds.m_maxs.y - screenCameraBounds.m_mins.y;
	
	// Calculate padding and text bounds
	static float widthPadding		=	screenCameraWidth	* 0.005f;
	static float heightPadding		=	screenCameraHeight	* 0.0005f;
	static AABB2 textBounds			=	AABB2(screenCameraBounds.m_mins.x + widthPadding, screenCameraBounds.m_mins.y + heightPadding,
											  screenCameraBounds.m_maxs.x - widthPadding, screenCameraBounds.m_maxs.y - heightPadding);
	textBounds.m_mins.y				=	textBounds.m_maxs.y * 0.95f;
	static float textCellHeight		=	(textBounds.m_maxs.y - textBounds.m_mins.y) * 0.45f;

	// Calculate debug frame information
	float	deltaSeconds	=	m_clock->GetDeltaSeconds();
	float	frameMS			=	deltaSeconds * 1000.f;
	float	fps				=	1.f / deltaSeconds;
	std::string frameInfo = Stringf("msPerFrame = %5.02f (%5.0f FPS)", frameMS, fps);

	// Immediate mode rendering for debug text
	static std::vector<Vertex_PCU> textVerts;
	textVerts.reserve(100);
	g_theFont->AddVertsForTextInBox2D(textVerts, textBounds, textCellHeight, frameInfo, Rgba8::WHITE, 0.5f, Vec2(1.f, 0.f));
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&g_theFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);
	textVerts.clear();
}



//--------------------------------------------------------------------------------------------------
Vec4 Game::GetSkyColor() const
{
	float timeElapsedSeconds	=	m_clock->GetTotalSeconds();
	float elapsedPhaseSeconds	=	fmodf(timeElapsedSeconds, SEC_PER_FULL_ROTATION);
	Rgba8 skyColor;

	if (elapsedPhaseSeconds < DAWN_DURATION_SEC)
	{
		// Dawn
		float interpolator = elapsedPhaseSeconds / DAWN_DURATION_SEC;
		skyColor = Interpolate(NIGHT_COLOR, DAWN_COLOR, interpolator);
	}
	else if (elapsedPhaseSeconds < DAWN_DURATION_SEC + DAY_DURATION_SEC)
	{
		// Day
		float interpolator = (elapsedPhaseSeconds - DAWN_DURATION_SEC) / DAY_DURATION_SEC;
		skyColor = Interpolate(DAWN_COLOR, DAY_COLOR, interpolator);
	}
	else if (elapsedPhaseSeconds < DAWN_DURATION_SEC + DAY_DURATION_SEC + DUSK_DURATION_SEC)
	{
		// Dusk
		float interpolator = (elapsedPhaseSeconds - (DAWN_DURATION_SEC + DAY_DURATION_SEC)) / DUSK_DURATION_SEC;
		skyColor = Interpolate(DAY_COLOR, DUSK_COLOR, interpolator);
	}
	else
	{
		// Night
		float interpolator = (elapsedPhaseSeconds - (DAWN_DURATION_SEC + DAY_DURATION_SEC + DUSK_DURATION_SEC)) / NIGHT_DURATION_SEC;
		skyColor = Interpolate(DUSK_COLOR, NIGHT_COLOR, interpolator);
	}
	
	float skyColorAsFloats[4] = { };
	skyColor.GetAsFloats(skyColorAsFloats);

	return Vec4(skyColorAsFloats[0], skyColorAsFloats[1], skyColorAsFloats[2], skyColorAsFloats[3]);
}


//--------------------------------------------------------------------------------------------------
Rgba8 Game::GetVertexColorBasedOnHeight(float terrainHeight, float terrainMaxHeight) const
{
	static float inverseTerrainMaxHeight	= 1.f / terrainMaxHeight;	
	float terrainHeightZeroToOne			= terrainHeight * inverseTerrainMaxHeight;
	if (terrainHeightZeroToOne <= 0.1f)
	{
		return Rgba8::BLUE;
	}
	else if (terrainHeightZeroToOne <= 0.3f)
	{
		return Rgba8::GREEN;
	}
	else if (terrainHeightZeroToOne <= 0.65f)
	{
		return Rgba8(150, 70, 0);
	}
	else
	{
		return Rgba8::WHITE;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeConstantBuffers()
{
	m_gameConstantsCB = g_theRenderer->CreateConstantBuffer(sizeof(GameConstants));
	g_theRenderer->BindConstantBuffer(g_gameConstantsSlot, m_gameConstantsCB, BindingLocation::PIXEL_SHADER);

	FoliageConstants foliageConstants = { };
	m_foliageConstantsCB = g_theRenderer->CreateConstantBuffer(sizeof(FoliageConstants), true, true, &foliageConstants);
	g_theRenderer->BindConstantBuffer(g_foliageConstantsSlot, m_foliageConstantsCB, BindingLocation::PIXEL_SHADER);
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeRayMarchingCanvas()
{
	// Create canvas to raymarch on (full screen quad that moves along the with the camera)
	// Calculate canvas bounds that is 10 units away from camera
	constexpr float canvasDistToCam			=	10.f;
	Camera	const&	playerCam				=	m_player->m_camera;
	float			playerCamFovDegrees		=	playerCam.GetPerspectiveCameraFov();
	float			playerCamAspect			=	playerCam.GetPerspectiveCameraAspect();
	float			canvasHalfHeight		=	canvasDistToCam * TanDegrees(playerCamFovDegrees * 0.5f);
	float			canvasHalfWidth			=	canvasHalfHeight * playerCamAspect;
	Vec3	const&	rayMarchingCanvasMins	=	Vec3(canvasDistToCam, canvasHalfWidth, -canvasHalfHeight);
	Vec3	const&	rayMarchingCanvasMaxs	=	Vec3(canvasDistToCam, -canvasHalfWidth, canvasHalfHeight);
	Vec3	const&	bottomLeft				=	rayMarchingCanvasMins;
	Vec3	const&	bottomRight				=	Vec3(rayMarchingCanvasMaxs.x, rayMarchingCanvasMaxs.y, rayMarchingCanvasMins.z);
	Vec3	const&	topRight				=	Vec3(rayMarchingCanvasMaxs.x, rayMarchingCanvasMaxs.y, rayMarchingCanvasMaxs.z);
	Vec3	const&	topLeft					=	Vec3(rayMarchingCanvasMaxs.x, rayMarchingCanvasMins.y, rayMarchingCanvasMaxs.z);

	// Create and populate raymarching canvas's VBO and IBO
	std::vector<Vertex_PCU>		tempVerts;
	std::vector<unsigned int>	tempIndexes;
	AddVertsForQuad3D(tempVerts, tempIndexes, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE);
	m_rayMarchingCanvasVB	=	g_theRenderer->CreateVertexBuffer(tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
	m_rayMarchingCanvasIB	=	g_theRenderer->CreateIndexBuffer(tempIndexes.size(), ResourceUsage::GPU_READ, tempIndexes.data());
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeResources()
{
	m_testUVTexture				=	g_theRenderer->CreateTextureResourceFromFile("Data/Textures/TestUV.png", "TestUV", sizeof("TestUV") - 1);
	m_skyBoxCubeMapResource		=	g_theRenderer->CreateTextureCubeResourceFromFile("Data/ColdSunsetSkybox_png/", "Skybox", sizeof("Skybox"));
	m_foliageStiffnessResource	=	g_theRenderer->CreateTextureResourceFromFile("Data/Textures/FoliageStiffness.png", "FoliageStiffness", sizeof("FoliageStiffness") - 1);

	// Terrain vertex and Index buffer
	CreateAndPopulateTerrainBuffers(IntVec2(TERRAIN_RES, TERRAIN_RES));
	// Reflection plane vertex and index buffer
	CreateAndPopulateReflectionPlaneBuffers(IntVec2(64, 64));

	// Create foliage instance and index buffers
	CreateAndPopulateFoliageBuffers();
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeShaders()
{
	m_rayMarchingShader				=	g_theRenderer->CreateOrGetShader("Data/Shaders/RaymarchingShader");
	m_skyBoxShader					=	g_theRenderer->CreateOrGetShader("Data/Shaders/SkyBoxShader");
	m_reflectionPlaneShader			=	g_theRenderer->CreateOrGetShader("Data/Shaders/ReflectionPlaneShader");
	m_foliageShader					=	g_theRenderer->CreateOrGetShader("Data/Shaders/FoliageShader", InputLayout::VERTEX_NONE, true);
	m_terrainShader					=	g_theRenderer->CreateOrGetShader("Data/Shaders/TerrainShader", InputLayout::VERTEX_PCU);
	m_debugSDFRaymarchingShader		=	g_theRenderer->CreateOrGetShader("Data/Shaders/DebugSDFRaymarchingShader", InputLayout::VERTEX_PCU);
}


//--------------------------------------------------------------------------------------------------
void Game::CreateAndPopulateTerrainBuffers(IntVec2 const& terrainRes)
{
	constexpr float terrainMaxHeight = 5.f;
	std::vector<Vertex_PCU>		tempVerts;
	std::vector<unsigned int>	tempIndexes;

	IntVec2 terrainHalfRes;
	terrainHalfRes.x	=	int(terrainRes.x * 0.5f);
	terrainHalfRes.y	=	int(terrainRes.y * 0.5f);
	float quadRes		=	1.f;

	for (int yCoord = -terrainHalfRes.y; yCoord < terrainHalfRes.y; ++yCoord)
	{
		for (int xCoord = -terrainHalfRes.x; xCoord < terrainHalfRes.x; ++xCoord)
		{
			// Construct a quad along the xy plane with a height of zero (z = 0)
			Vec2 currentQuadCenter	=	Vec2(float(xCoord), float(yCoord));
			AABB2 currentQuad		=	AABB2(currentQuadCenter, quadRes, quadRes);
			Vec2 currentQuadMins	=	currentQuad.m_mins;
			Vec2 currentQuadMaxs	=	currentQuad.m_maxs;
			Vec3 currentBL			=	Vec3(currentQuadMins.x, currentQuadMins.y, 0.f);
			Vec3 currentBR			=	Vec3(currentQuadMaxs.x, currentQuadMins.y, 0.f);
			Vec3 currentTR			=	Vec3(currentQuadMaxs.x, currentQuadMaxs.y, 0.f);
			Vec3 currentTL			=	Vec3(currentQuadMins.x, currentQuadMaxs.y, 0.f);
						
			// // currentBL.z = terrainMaxHeight * ABS_FLOAT(Compute2dPerlinNoise(currentBL.x, currentBL.y, 40.f, 10, 0.5f));
			// // currentBR.z = terrainMaxHeight * ABS_FLOAT(Compute2dPerlinNoise(currentBR.x, currentBR.y, 40.f, 10, 0.5f));
			// // currentTR.z = terrainMaxHeight * ABS_FLOAT(Compute2dPerlinNoise(currentTR.x, currentTR.y, 40.f, 10, 0.5f));
			// // currentTL.z = terrainMaxHeight * ABS_FLOAT(Compute2dPerlinNoise(currentTL.x, currentTL.y, 40.f, 10, 0.5f));
			// 
			// // Vertex colors based on height
			// Rgba8 currentBLColor = Rgba8::WHITE; // GetVertexColorBasedOnHeight(currentBL.z, terrainMaxHeight);
			// Rgba8 currentBRColor = Rgba8::WHITE; // GetVertexColorBasedOnHeight(currentBR.z, terrainMaxHeight);
			// Rgba8 currentTRColor = Rgba8::WHITE; // GetVertexColorBasedOnHeight(currentTR.z, terrainMaxHeight);
			// Rgba8 currentTLColor = Rgba8::WHITE; // GetVertexColorBasedOnHeight(currentTL.z, terrainMaxHeight);
			// 
			// // Add verts for terrain
			// tempVerts.emplace_back(currentBL, currentBLColor, Vec2());
			// tempVerts.emplace_back(currentBR, currentBRColor, Vec2());
			// tempVerts.emplace_back(currentTR, currentTRColor, Vec2());
			// tempVerts.emplace_back(currentTL, currentTLColor, Vec2());
			// 
			// int startIndex = (int)tempVerts.size();
			// tempIndexes.emplace_back(startIndex);
			// tempIndexes.emplace_back(startIndex + 1);
			// tempIndexes.emplace_back(startIndex + 2);
			// tempIndexes.emplace_back(startIndex);
			// tempIndexes.emplace_back(startIndex + 2);
			// tempIndexes.emplace_back(startIndex + 3);
			AddVertsForQuad3D(tempVerts, tempIndexes, currentBL, currentBR, currentTR, currentTL);
		}
	}

	m_terrainVB = g_theRenderer->CreateVertexBuffer(tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
	m_terrainIB = g_theRenderer->CreateIndexBuffer(tempIndexes.size(), ResourceUsage::GPU_READ, tempIndexes.data());
}


//--------------------------------------------------------------------------------------------------
void Game::CreateAndPopulateReflectionPlaneBuffers(IntVec2 const& reflectionPlaneRes)
{
	std::vector<Vertex_PCU>	tempVerts;
	std::vector<unsigned int>	tempIndexes;

	Vec2 refPlaneHalfRes;
	refPlaneHalfRes.x			=	float(reflectionPlaneRes.x * 0.5f);
	refPlaneHalfRes.y			=	float(reflectionPlaneRes.y * 0.5f);
	// float quadRes				=	1.f;
	// Vec2 const& planeOffset		=	Vec2(0.f, float(reflectionPlaneRes.y) + 2.f);

	// Quick hack
	AABB2 refPlane = AABB2(Vec2(0.f, 0.f), refPlaneHalfRes.x, refPlaneHalfRes.y);
	AddVertsForAABB2D(tempVerts, tempIndexes, refPlane, Rgba8::WHITE);

	
	m_reflectionPlaneVB = g_theRenderer->CreateVertexBuffer(tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
	m_reflectionPlaneIB = g_theRenderer->CreateIndexBuffer(tempIndexes.size(), ResourceUsage::GPU_READ, tempIndexes.data());
}


//--------------------------------------------------------------------------------------------------
void Game::CreateAndPopulateFoliageBuffers()
{
	uint32_t tempIndexes[INDEX_COUNT_PER_GRASS_BLADE]		=	{};

	// Create a grass blade
	for (uint32_t segmentIndex = 0; segmentIndex < NUM_GRASS_SEGMENTS; ++segmentIndex)
	{
		uint32_t currentIndexLocation			=	segmentIndex * INDEX_COUNT_PER_SEGMENT_FACE * 2; // multiplying by two to take both front and back side into account
		uint32_t currentPrimitiveIndexStart		=	segmentIndex * 2;

		// Front face, current segment first triangle
		tempIndexes[currentIndexLocation + 0]	=	currentPrimitiveIndexStart + 0;
		tempIndexes[currentIndexLocation + 1]	=	currentPrimitiveIndexStart + 1;
		tempIndexes[currentIndexLocation + 2]	=	currentPrimitiveIndexStart + 2;
		
		// Front face, current segment second triangle
		tempIndexes[currentIndexLocation + 3]	=	currentPrimitiveIndexStart + 2;
		tempIndexes[currentIndexLocation + 4]	=	currentPrimitiveIndexStart + 1;
		tempIndexes[currentIndexLocation + 5]	=	currentPrimitiveIndexStart + 3;

		// Back face, current segment first triangle
		currentIndexLocation					+=	6;
		currentPrimitiveIndexStart				+=	UNIQUE_VERTS_PER_FACE;
		tempIndexes[currentIndexLocation + 0]	=	currentPrimitiveIndexStart + 2;
		tempIndexes[currentIndexLocation + 1]	=	currentPrimitiveIndexStart + 1;
		tempIndexes[currentIndexLocation + 2]	=	currentPrimitiveIndexStart + 0;

		// Back face, current segment second triangle
		tempIndexes[currentIndexLocation + 3]	=	currentPrimitiveIndexStart + 3;
		tempIndexes[currentIndexLocation + 4]	=	currentPrimitiveIndexStart + 1;
		tempIndexes[currentIndexLocation + 5]	=	currentPrimitiveIndexStart + 2;
	}

	g_theRenderer->CreateIndexBuffer(m_foliageInstanceDataIB, INDEX_COUNT_PER_GRASS_BLADE, ResourceUsage::GPU_READ, tempIndexes);
	g_theRenderer->CreateVertexBuffer(m_foliageInstanceDataVB, UNIQUE_VERTS_PER_FACE * 2 * GRASS_BLADE_INSTANCE_COUNT, sizeof(Vec3), ResourceUsage::GPU_READ_CPU_WRITE);
}
