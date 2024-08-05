#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"


//--------------------------------------------------------------------------------------------------
#include "Engine/Renderer/RendererAnnotationJanitor.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
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

	g_theEventSystem->SubscribeEventCallbackFunction("PeelCount", Command_SetDepthPeelCount);
}


//--------------------------------------------------------------------------------------------------
Game::~Game()
{
	delete m_clock;
	m_clock = nullptr;

	delete m_gridVertexBuffer;
	m_gridVertexBuffer = nullptr;

	// Delete vertex and index buffer for all meshes in all scenes
	for (unsigned char sceneIndex = 0; sceneIndex < eTRANSLUCENT_SCENE_COUNT; ++sceneIndex)
	{
		Scene*& currentScene					=	m_scenes[sceneIndex];
		if (!currentScene)
		{
			continue;
		}
		std::vector<SceneObject>& currentSceneTranslucentObjects	=	currentScene->m_translucentObjects;
		for (unsigned int meshIndex = 0; meshIndex < (unsigned int)currentSceneTranslucentObjects.size(); ++meshIndex)
		{
			SceneObject& currentMesh = currentSceneTranslucentObjects[meshIndex];
			delete currentMesh.m_vertexBuffer;
			currentMesh.m_vertexBuffer = nullptr;

			delete currentMesh.m_indexBuffer;
			currentMesh.m_indexBuffer = nullptr;
		}

		
		std::vector<SceneObject>& currentSceneOpaqueObjects	=	currentScene->m_opaqueObjects;
		for (unsigned int meshIndex = 0; meshIndex < (unsigned int)currentSceneOpaqueObjects.size(); ++meshIndex)
		{
			SceneObject& currentMesh = currentSceneOpaqueObjects[meshIndex];
			delete currentMesh.m_vertexBuffer;
			currentMesh.m_vertexBuffer = nullptr;

			delete currentMesh.m_indexBuffer;
			currentMesh.m_indexBuffer = nullptr;
		}
	}

	delete m_fullScreenQuadVB;
	m_fullScreenQuadVB = nullptr;
	delete m_fullScreenQuadIB;
	m_fullScreenQuadIB = nullptr;

	delete m_quadrant1QuadVB ;
	m_quadrant1QuadVB  = nullptr;
	delete m_quadrant1QuadIB ;
	m_quadrant1QuadIB  = nullptr;

	delete m_quadrant2QuadVB ;
	m_quadrant2QuadVB  = nullptr;
	delete m_quadrant2QuadIB ;
	m_quadrant2QuadIB  = nullptr;

	delete m_quadrant3QuadVB ;
	m_quadrant3QuadVB  = nullptr;
	delete m_quadrant3QuadIB ;
	m_quadrant3QuadIB  = nullptr;

	delete m_quadrant4QuadVB ;
	m_quadrant4QuadVB  = nullptr;
	delete m_quadrant4QuadIB ;
	m_quadrant4QuadIB  = nullptr;

	delete m_quadrant1Split2QuadVB;
	m_quadrant1Split2QuadVB = nullptr;
	delete m_quadrant1Split2QuadIB;
	m_quadrant1Split2QuadIB = nullptr;

	delete m_quadrant2Split2QuadVB;
	m_quadrant2Split2QuadVB = nullptr;
	delete m_quadrant2Split2QuadIB;
	m_quadrant2Split2QuadIB = nullptr;

	delete m_gameConstantBuffer;
	m_gameConstantBuffer = nullptr;

	delete m_screenConstantBuffer;
	m_gameConstantBuffer = nullptr;
}


//--------------------------------------------------------------------------------------------------
void Game::Startup()
{
	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));
	m_player = new Player(this, Vec3(-2.f, 0.f, 0.f), m_clientAspect);

	// Meshes needed to be initialized before initializing textures
	InitializeMeshes();
	InitializeShaders();
	InitializeResources();
	InitializeGridBuffer();
	InitializeCustomBlendModes();
	InitializeScenesFromDefinition();
	InitializeVertexAndIndexBuffers();
	
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
		m_isDebug = !m_isDebug;
	}

	if (g_theInput->IsKeyDown_WasUp('1'))
	{
		if (m_isTwoSplitScreen || m_isFourSplitScreen)
		{
			m_quadrantInControl = eSPLIT_SCREEN_QUADRANT_1;
		}
	}

	if (g_theInput->IsKeyDown_WasUp('2'))
	{
		if (m_isTwoSplitScreen || m_isFourSplitScreen)
		{
			m_quadrantInControl = eSPLIT_SCREEN_QUADRANT_2;
		}
	}

	if (g_theInput->IsKeyDown_WasUp('3'))
	{
		if (m_isFourSplitScreen)
		{
			m_quadrantInControl = eSPLIT_SCREEN_QUADRANT_3;
		}
	}

	if (g_theInput->IsKeyDown_WasUp('4'))
	{
		if (m_isFourSplitScreen)
		{
			m_quadrantInControl = eSPLIT_SCREEN_QUADRANT_4;
		}
	}

	// Switch game modes, Left arrow key to go to the previous game mode
	if (g_theInput->IsKeyDown_WasUp(KEYCODE_LEFT))
	{
		// Go to previous game mode if the game mode is not locked
		if (!m_isGameModeLocked)
		{
			m_gameMode = eGameMode(m_gameMode - 1);
			// Game mode has a type of unsigned char, hence no -ve numbers, 0 - 1 wraps around to be the max value (255)
			if (m_gameMode == unsigned char(-1))
			{
				m_gameMode = eGameMode(eGAME_MODE_COUNT - 1);
			}
			// Reset all quadrantSubgroup modes
			m_quadrant1SubgroupMode = 0;
			m_quadrant2SubgroupMode = 0;
			m_quadrant3SubgroupMode = 0;
			m_quadrant4SubgroupMode = 0;
		}
		else // Go to the previous translucent scene, when game mode is locked
		{
			m_sceneIndex = eTranslucentScene(m_sceneIndex - 1);
			// Translucent scene is of type unsigned char, -ve numbers wrap around (-1 == 255)
			if (m_sceneIndex == unsigned char(-1))
			{
				m_sceneIndex = eTranslucentScene(eTRANSLUCENT_SCENE_COUNT - 1);
			}
		}
	}

	// Switch game modes, Right arrow key to go to the next game mode
	if (g_theInput->IsKeyDown_WasUp(KEYCODE_RIGHT))
	{
		// Go to the next game mode while the game mode is not locked
		if (!m_isGameModeLocked)
		{
			m_gameMode = eGameMode(m_gameMode + 1);
			if (m_gameMode == eGAME_MODE_COUNT)
			{
				m_gameMode = eGameMode(0);
			}
			// Reset all quadrantSubgroup modes
			m_quadrant1SubgroupMode = 0;
			m_quadrant2SubgroupMode = 0;
			m_quadrant3SubgroupMode = 0;
			m_quadrant4SubgroupMode = 0;
		}
		else // Go to the next translucent scene, when game mode is locked
		{
			m_sceneIndex = eTranslucentScene(m_sceneIndex + 1);
			if (m_sceneIndex == eTRANSLUCENT_SCENE_COUNT)
			{
				m_sceneIndex = eTranslucentScene(0);
			}
		}
	}

	// Cycle through Subgroup Modes (game modes within game modes)
	// // Go to the previous sub mode
	if (g_theInput->IsKeyDown_WasUp(KEYCODE_UP))
	{
		switch (m_gameMode)
		{
			case eGAME_MODE_DEPTH_TEST:
			{
				GoToPreviousValidMode(m_quadrantInControl, eDEPTH_TEST_MODE_COUNT);
				break;
			}
			case eGAME_MODE_ALPHA_BLENDING:
			{
				GoToPreviousValidMode(m_quadrantInControl, eALPHA_BLENDING_MODE_COUNT);
				break;
			}
			case eGAME_MODE_OIT:
			{
				GoToPreviousValidMode(m_quadrantInControl, eOIT_MODE_COUNT);
				break;
			}
			case eGAME_MODE_UAV_WRITES:
			{
				GoToPreviousValidMode(m_quadrantInControl, eUAV_WRITE_MODE_COUNT);
				break;
			}
			default:
			{
				ERROR_AND_DIE("Error switching to the previous mode: Invalid game mode provided");
			}
		}
	}

	// // Go to the next sub mode
	if (g_theInput->IsKeyDown_WasUp(KEYCODE_DOWN))
	{
		switch (m_gameMode)
		{
			case eGAME_MODE_DEPTH_TEST:
			{
				GoToNextValidMode(m_quadrantInControl, eDEPTH_TEST_MODE_COUNT);
				break;
			}
			case eGAME_MODE_ALPHA_BLENDING:
			{
				GoToNextValidMode(m_quadrantInControl, eALPHA_BLENDING_MODE_COUNT);

				break;
			}
			case eGAME_MODE_OIT:
			{
				GoToNextValidMode(m_quadrantInControl, eOIT_MODE_COUNT);
				break;
			}
			case eGAME_MODE_UAV_WRITES:
			{
				GoToNextValidMode(m_quadrantInControl, eUAV_WRITE_MODE_COUNT);
				break;
			}
			default:
			{
				ERROR_AND_DIE("Error switching to the previous mode: Invalid game mode provided");
			}
		}
	}

	// Toggle between no split screen, two split screen, four split screens
	if (g_theInput->IsKeyDown_WasUp('N'))
	{
		if (m_isFourSplitScreen) // Turn off split screen if we are have four split screens
		{
			m_isFourSplitScreen = false;
			m_quadrantInControl = eSPLIT_SCREEN_QUADRANT_1;
		}
		else if (m_isTwoSplitScreen) // Turn to four split screens
		{
			m_isTwoSplitScreen				=	false;
			m_isFourSplitScreen				=	true;
			m_translucentModeQuadrant2		=	m_translucentModeQuadrant1;
			m_translucentModeQuadrant3		=	m_translucentModeQuadrant1;
			m_translucentModeQuadrant4		=	m_translucentModeQuadrant1;
		}
		else // Turn to two split screens
		{
			m_isTwoSplitScreen				=	true;
			m_translucentModeQuadrant2		=	m_translucentModeQuadrant1;
		}
	}

	// Lock the game mode if the current game mode is OIT
	if (g_theInput->IsKeyDown_WasUp('L'))
	{
		m_isGameModeLocked = !m_isGameModeLocked;
	}

	// Toggle texturing
	if (g_theInput->IsKeyDown_WasUp('T'))
	{
		m_isTextured = !m_isTextured;
	}

	// Depth mode Debug toggle ground truth and result
	if (g_theInput->IsKeyDown_WasUp('Z'))
	{
		m_isDepthTestEnabled = !m_isDepthTestEnabled;
	}

	// toggle between drawing individual meshes vs batching draw calls into one draw call
	if (g_theInput->IsKeyDown_WasUp('V'))
	{
		m_isRetainedModeRendering = !m_isRetainedModeRendering;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::Update()
{
	KeyboardUpdate();
	m_player->Update();
	GameModeUpdate();
}


//--------------------------------------------------------------------------------------------------
void Game::Render() const
{
	Rgba8 clearColor{ 255, 255, 255, 255 };
	if (m_isTwoSplitScreen)
	{
		g_theRenderer->ClearScreen(Rgba8::BLACK);
	}
	else
	{
		g_theRenderer->ClearScreen(clearColor);
	}
	
	g_theRenderer->BeginCamera(m_player->m_camera);

	GameModeRender();

	g_theRenderer->EndCamera(m_player->m_camera);

	g_theRenderer->BeginCamera(m_screenCamera);
	{
		RenderQuadrants();
	}
	g_theRenderer->EndCamera(m_screenCamera);
}


//--------------------------------------------------------------------------------------------------
void Game::GameModeUpdate()
{
	switch (m_gameMode)
	{
		case eGAME_MODE_DEPTH_TEST:
		{

			break;
		}
		case eGAME_MODE_ALPHA_BLENDING:
		{

			break;
		}
		case eGAME_MODE_UAV_WRITES:
		{

			break;
		}
		case eGAME_MODE_OIT:
		{
			OITUpdate();
			break;
		}
		default:
		{
			ERROR_AND_DIE("Invalid Game Mode: Please prove a valid game mode to update");
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::OITUpdate()
{
	// if (!m_isTwoSplitScreen && !m_isFourSplitScreen)
	// {
	// 	UpdateOITSubModeQuadrant(m_quadrant1SubgroupMode);
	// }
	// else if (m_isTwoSplitScreen)
	// {
	// 	UpdateOITSubModeQuadrant(m_quadrant1SubgroupMode);
	// 	UpdateOITSubModeQuadrant(m_quadrant2SubgroupMode);
	// }
	// else
	// {
	// 	UpdateOITSubModeQuadrant(m_quadrant1SubgroupMode);
	// 	UpdateOITSubModeQuadrant(m_quadrant2SubgroupMode);
	// 	UpdateOITSubModeQuadrant(m_quadrant3SubgroupMode);
	// 	UpdateOITSubModeQuadrant(m_quadrant4SubgroupMode);
	// }

	if (g_theInput->IsKeyDown_WasUp('J'))
	{
		switch (m_quadrantInControl)
		{
			case eSPLIT_SCREEN_QUADRANT_1:
			{
				UpdateNodeCount(m_quadrant1Nodes);
				break;
			}
			case eSPLIT_SCREEN_QUADRANT_2:
			{
				UpdateNodeCount(m_quadrant2Nodes);
				break;
			}
			case eSPLIT_SCREEN_QUADRANT_3:
			{
				UpdateNodeCount(m_quadrant3Nodes);
				break;
			}
			case eSPLIT_SCREEN_QUADRANT_4:
			{
				UpdateNodeCount(m_quadrant4Nodes);
				break;
			}
			default:
			{
				ERROR_AND_DIE("Invalid quadrant");
			}
		}
	}

	if (m_sceneIndex == eTRANSLUCENT_SCENE_FOG)
	{
		Scene*&						currentScene			=	m_scenes[eTRANSLUCENT_SCENE_FOG];
		std::vector<SceneObject>&	currentSceneMeshList	=	currentScene->m_translucentObjects;
		Camera&						playerCamera			=	m_player->m_camera;
		Mat44						playerCameraTransform	=	playerCamera.GetCameraOrientation().GetAsMatrix_XFwd_YLeft_ZUp();
		
		playerCameraTransform.SetTranslation3D(playerCamera.GetCameraPosition());
		for (unsigned int meshIndex = 0; meshIndex < (unsigned int)currentSceneMeshList.size(); ++meshIndex)
		{
			SceneObject& currentMesh = currentSceneMeshList[meshIndex];
			if (currentMesh.m_type != eMESH_TYPE_BILLBOARDED_QUAD)
			{
				continue;
			}

			currentMesh.m_transform = GetBillboardMatrix(BillboardType::FULL_CAMERA_OPPOSING, playerCameraTransform, currentMesh.m_center, currentMesh.m_billboardedQuadHalfDims);
		}
	}

	if (m_quadrant1SubgroupMode != eOIT_MODE_CPU_SORTED && m_quadrant2SubgroupMode != eOIT_MODE_CPU_SORTED && m_quadrant3SubgroupMode != eOIT_MODE_CPU_SORTED && m_quadrant4SubgroupMode != eOIT_MODE_CPU_SORTED)
	{
		return;
	}

	// Sort translucent scene meshes
	Camera const&			playerCamera			=	m_player->m_camera;
	Vec3					playerCamPos			=	playerCamera.GetCameraPosition();
	EulerAngles				playerCamOrientation	=	playerCamera.GetCameraOrientation();

	// if (m_prevPlayerCamPos == playerCamPos && m_prevPlayerCamOrientation == playerCamOrientation)
	// {
	// 	return;
	// }

	
	Scene* currentScene	= m_scenes[m_sceneIndex];
	GUARANTEE_OR_DIE(currentScene, "Invalid Translucent Scene: Please add trasnlucent objects to the scene");
	currentScene->m_sortedTranslucentObjects		=	currentScene->m_translucentObjects;
	std::vector<SceneObject>&	currentMeshList		=	currentScene->m_sortedTranslucentObjects;
	unsigned int				numOfMeshesInScene	=	(unsigned int)currentMeshList.size();
	
	std::vector<float> distancesFromCam;
	distancesFromCam.reserve(numOfMeshesInScene);
	for (unsigned int meshIndex = 0; meshIndex < numOfMeshesInScene; ++meshIndex)
	{
		SceneObject const&	currentMesh				=	currentMeshList[meshIndex];
		Vec3 const&			currentMeshCenter		=	currentMesh.m_center;
		Vec3				dispFromCamToMesh		=	currentMeshCenter - playerCamPos;
		float				meshDistSqrdFromCamera	=	dispFromCamToMesh.GetLengthSquared();
		distancesFromCam.emplace_back(meshDistSqrdFromCamera);
	}

	unsigned int maxIndex = 0;
	for (unsigned int unsortedElementIndex = 0; unsortedElementIndex < numOfMeshesInScene - 1; ++unsortedElementIndex)
	{
		maxIndex = unsortedElementIndex;
		for (unsigned int remainingElementIndex = unsortedElementIndex + 1; remainingElementIndex < numOfMeshesInScene; ++remainingElementIndex)
		{
			if (distancesFromCam[remainingElementIndex] > distancesFromCam[maxIndex])
			{
				maxIndex = remainingElementIndex;
			}
		}

		if (maxIndex != unsortedElementIndex)
		{
			float tempDist							=	distancesFromCam[maxIndex];
			distancesFromCam[maxIndex]				=	distancesFromCam[unsortedElementIndex];
			distancesFromCam[unsortedElementIndex]	=	tempDist;

			SceneObject tempMesh							=	currentMeshList[maxIndex];
			currentMeshList[maxIndex]				=	currentMeshList[unsortedElementIndex];
			currentMeshList[unsortedElementIndex]	=	tempMesh;
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::UpdateOITSubModeQuadrant(unsigned char subgroupMode)
{
	switch ((eOITMode)subgroupMode)
	{
		case eOIT_MODE_WORST_CASE:
		{
			// m_isNodeBasedOITMode = false;
			break;
		}
		case eOIT_MODE_CPU_SORTED:
		{
			// m_isNodeBasedOITMode = false;
			break;
		}
		case eOIT_MODE_DEPTH_PEELING:
		{
			// m_isNodeBasedOITMode = false;
			break;
		}
		case eOIT_MODE_VIRTUAL_PIXEL_MAPS:
		{
			// m_isNodeBasedOITMode = false;
			break;
		}
		case eOIT_MODE_WEIGHTED_BLENDED:
		{
			// m_isNodeBasedOITMode = false;
			break;
		}
		case eOIT_MODE_PER_PIXEL_LINKED_LIST:
		{
			// m_isNodeBasedOITMode = true;
			break;
		}
		case eOIT_MODE_MULTI_LAYERED_ALPHA_BLENDING:
		{
			// m_isNodeBasedOITMode = true;
			break;
		}
		default:
			break;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::RenderDebugInfo() const
{
	static AABB2 screenCameraBounds =	m_screenCamera.GetCameraAABB();
	static float screenCameraWidth	=	screenCameraBounds.m_maxs.x - screenCameraBounds.m_mins.x; 
	static float screenCameraHeight	=	screenCameraBounds.m_maxs.y - screenCameraBounds.m_mins.y;
	static float widthPadding		=	screenCameraWidth	* 0.005f;
	static float heightPadding		=	screenCameraHeight	* 0.0005f;

	// static AABB2 textBounds			=	screenCameraBounds;
	static AABB2 textBounds = AABB2(screenCameraBounds.m_mins.x + widthPadding, screenCameraBounds.m_mins.y + heightPadding,
									screenCameraBounds.m_maxs.x - widthPadding, screenCameraBounds.m_maxs.y - heightPadding);
	textBounds.m_mins.y				=	textBounds.m_maxs.y * 0.95f;
	static float textCellHeight		=	(textBounds.m_maxs.y - textBounds.m_mins.y) * 0.45f;

	float	deltaSeconds	=	m_clock->GetDeltaSeconds();
	float	frameMS			=	deltaSeconds * 1000.f;
	float	fps				=	1.f / deltaSeconds;
	char*	gameModeAsText	=	GetGameModeAsText();
	std::string frameInfo = "";
	if (m_gameMode != eGAME_MODE_OIT)
	{
		frameInfo =	Stringf("GameMode = %s, msPerFrame = %5.02f (%5.0f FPS)", gameModeAsText, frameMS, fps);
	}
	else
	{
		char* translucentSceneAsText	=	GetTranslucentSceneAsText();
		char* quadrantInControlAsText	=	GetControlledQuadrantAsText();
		char* translucentModeAsText		=	nullptr;
		translucentModeAsText			=	GetTranslucentModeAsText();
		if (translucentModeAsText)
		{
			frameInfo = Stringf("GameMode = %s, TranslucentScene = %s, msPerFrame = %5.02f (%5.0f FPS)\nQuadrant = %s, TranslucentMode = %s", gameModeAsText, translucentSceneAsText, frameMS, fps, quadrantInControlAsText, translucentModeAsText);
		}
		else
		{
			frameInfo = Stringf("GameMode = %s, TranslucentScene = %s, msPerFrame = %5.02f (%5.0f FPS)\nQuadrant = %s", gameModeAsText, translucentSceneAsText, frameMS, fps, quadrantInControlAsText);
		}
	}

	static std::vector<Vertex_PCU> textVerts;
	textVerts.reserve(100);
	g_theFont->AddVertsForTextInBox2D(textVerts, textBounds, textCellHeight, frameInfo, Rgba8::WHITE, 0.5f, Vec2(1.f, 0.f));
	
	g_theRenderer->BindTexture(&g_theFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);
	textVerts.clear();
}


//--------------------------------------------------------------------------------------------------
void Game::RenderGameModeInfo(eSplitScreenQuadrant quadrant /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor gameModeInfoRender(L"Game Mode Info");

	g_theRenderer->EndCamera(m_player->m_camera);
	g_theRenderer->BeginCamera(m_screenCamera);
	
	static AABB2 screenCameraBounds =	m_screenCamera.GetCameraAABB();
	static float screenCameraWidth	=	screenCameraBounds.m_maxs.x - screenCameraBounds.m_mins.x; 
	static float screenCameraHeight	=	screenCameraBounds.m_maxs.y - screenCameraBounds.m_mins.y;
	static float widthPadding		=	screenCameraWidth	* 0.005f;
	static float heightPadding		=	screenCameraHeight	* 0.0005f;

	static AABB2 textBounds = AABB2(screenCameraBounds.m_mins.x + widthPadding, screenCameraBounds.m_mins.y + heightPadding,
									screenCameraBounds.m_maxs.x - widthPadding, screenCameraBounds.m_maxs.y - heightPadding);
	textBounds.m_mins.y				=	textBounds.m_maxs.y * 0.95f;
	static float textCellHeight		=	(textBounds.m_maxs.y - textBounds.m_mins.y) * 0.45f;

	std::string gameModeAndSubGroupModeAsText = GetGameModeAndGroupModeAsText(quadrant);
	char* isGameLockedAsText	=	!m_isGameModeLocked ? (char*)"Locked: False" : (char*)"Locked: True";
	char* gameModeSceneAsText	=	GetTranslucentSceneAsText();
	bool isNodeBasedOITMode		=	IsNodeBasedOITMode(quadrant);
	eNodes nodesUsed			=	GetNodesFromQuadrant(quadrant);
	std::string gameModeInfo = Stringf("%s, Scene = %s, %s", gameModeAndSubGroupModeAsText.c_str(), gameModeSceneAsText, isGameLockedAsText);
	if (m_gameMode == eGAME_MODE_OIT && isNodeBasedOITMode)
	{
		if (nodesUsed == eNODES_32)
		{
			gameModeInfo = Stringf("%s, Nodes = 32, Scene = %s, %s", gameModeAndSubGroupModeAsText.c_str(), gameModeSceneAsText, isGameLockedAsText);
		}
		else if (nodesUsed == eNODES_4)
		{
			gameModeInfo = Stringf("%s, Nodes = 4, Scene = %s, %s", gameModeAndSubGroupModeAsText.c_str(), gameModeSceneAsText, isGameLockedAsText);
		}
		else
		{
			gameModeInfo = Stringf("%s, Nodes = 2, Scene = %s, %s", gameModeAndSubGroupModeAsText.c_str(), gameModeSceneAsText, isGameLockedAsText);
		}
	}

	static std::vector<Vertex_PCU> textVerts;
	textVerts.reserve(100);
	g_theFont->AddVertsForTextInBox2D(textVerts, textBounds, textCellHeight, gameModeInfo, Rgba8::WHITE, 0.75f, Vec2(0.5f, 0.f));
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindTexture(&g_theFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);
	textVerts.clear();

	g_theRenderer->EndCamera(m_screenCamera);
	g_theRenderer->BeginCamera(m_player->m_camera);
}


//--------------------------------------------------------------------------------------------------
void Game::GameModeRender() const
{
	RendererAnnotationJanitor gameModeRender(L"Game Mode Render");
	switch (m_gameMode)
	{
		case eGAME_MODE_DEPTH_TEST:
		{
			DepthTestSubModeRender();
			break;
		}
		case eGAME_MODE_ALPHA_BLENDING:
		{
			AlphaBlendingSubModeRender();
			break;
		}
		case eGAME_MODE_UAV_WRITES:
		{
			UAVWritesSubModeRender();
			break;
		}
		case eGAME_MODE_OIT:
		{
			OITSubModeRender();
			break;
		}
		default:
		{
			ERROR_AND_DIE("Game Mode Render Error: Please provide a valid game mode");
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::DepthTestSubModeRender() const
{
	if (!m_isTwoSplitScreen && !m_isFourSplitScreen)
	{
		DepthTestSubModeQuandrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		return;
	}

	if (m_isTwoSplitScreen)
	{
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-1 Render");
			DepthTestSubModeQuandrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-2 Render");
			DepthTestSubModeQuandrantRender(m_quadrant2SubgroupMode, eSPLIT_SCREEN_QUADRANT_2);
		}
		return;
	}

	if (m_isFourSplitScreen)
	{
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-1 Render");
			DepthTestSubModeQuandrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-2 Render");
			DepthTestSubModeQuandrantRender(m_quadrant2SubgroupMode, eSPLIT_SCREEN_QUADRANT_2);
		}
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-3 Render");
			DepthTestSubModeQuandrantRender(m_quadrant3SubgroupMode, eSPLIT_SCREEN_QUADRANT_3);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-4 Render");
			DepthTestSubModeQuandrantRender(m_quadrant4SubgroupMode, eSPLIT_SCREEN_QUADRANT_4);
		}
		return;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::AlphaBlendingSubModeRender() const
{
	if (!m_isTwoSplitScreen && !m_isFourSplitScreen)
	{
		AlphaBlendingSubModeQuadrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		return;
	}

	if (m_isTwoSplitScreen)
	{
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-1 Render");
			AlphaBlendingSubModeQuadrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-2 Render");
			AlphaBlendingSubModeQuadrantRender(m_quadrant2SubgroupMode, eSPLIT_SCREEN_QUADRANT_2);
		}
		return;
	}

	if (m_isFourSplitScreen)
	{
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-1 Render");
			AlphaBlendingSubModeQuadrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-2 Render");
			AlphaBlendingSubModeQuadrantRender(m_quadrant2SubgroupMode, eSPLIT_SCREEN_QUADRANT_2);
		}
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-3 Render");
			AlphaBlendingSubModeQuadrantRender(m_quadrant3SubgroupMode, eSPLIT_SCREEN_QUADRANT_3);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-4 Render");
			AlphaBlendingSubModeQuadrantRender(m_quadrant4SubgroupMode, eSPLIT_SCREEN_QUADRANT_4);
		}
		return;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::UAVWritesSubModeRender() const
{
	if (!m_isTwoSplitScreen && !m_isFourSplitScreen)
	{
		UAVWritesSubModeQuadrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		return;
	}

	if (m_isTwoSplitScreen)
	{
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-1 Render");
			UAVWritesSubModeQuadrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-2 Render");
			UAVWritesSubModeQuadrantRender(m_quadrant2SubgroupMode, eSPLIT_SCREEN_QUADRANT_2);
		}
		return;
	}

	if (m_isFourSplitScreen)
	{
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-1 Render");
			UAVWritesSubModeQuadrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-2 Render");
			UAVWritesSubModeQuadrantRender(m_quadrant2SubgroupMode, eSPLIT_SCREEN_QUADRANT_2);
		}
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-3 Render");
			UAVWritesSubModeQuadrantRender(m_quadrant3SubgroupMode, eSPLIT_SCREEN_QUADRANT_3);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-4 Render");
			UAVWritesSubModeQuadrantRender(m_quadrant4SubgroupMode, eSPLIT_SCREEN_QUADRANT_4);
		}
		return;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::OITSubModeRender() const
{
	if (!m_isTwoSplitScreen && !m_isFourSplitScreen)
	{
		OITSubModeQuadrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		return;
	}

	if (m_isTwoSplitScreen)
	{
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-1 Render");
			OITSubModeQuadrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-2 Render");
			OITSubModeQuadrantRender(m_quadrant2SubgroupMode, eSPLIT_SCREEN_QUADRANT_2);
		}
		return;
	}

	if (m_isFourSplitScreen)
	{
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-1 Render");
			OITSubModeQuadrantRender(m_quadrant1SubgroupMode, eSPLIT_SCREEN_QUADRANT_1);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-2 Render");
			OITSubModeQuadrantRender(m_quadrant2SubgroupMode, eSPLIT_SCREEN_QUADRANT_2);
		}
		{
			RendererAnnotationJanitor quadrant1(L"Quadrant-3 Render");
			OITSubModeQuadrantRender(m_quadrant3SubgroupMode, eSPLIT_SCREEN_QUADRANT_3);
		}
		{
			RendererAnnotationJanitor quadrant2(L"Quadrant-4 Render");
			OITSubModeQuadrantRender(m_quadrant4SubgroupMode, eSPLIT_SCREEN_QUADRANT_4);
		}
		return;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::DepthTestSubModeQuandrantRender(unsigned char subgroupMode, eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor subgroupModeRender(L"Depth Test Mode");
	switch ((eDepthTestMode)subgroupMode)
	{
		case eDEPTH_TEST_MODE_LESS:
		{
			LessThanEqualDepthTestSubModeRender(quadrantToRender);
			break;
		}
		case eDEPTH_TEST_MODE_GREATER:
		{
			GreaterThanEqualDepthTestSubModeRender(quadrantToRender);
			break;
		}
		case eDEPTH_TEST_MODE_DISABLED:
		{
			DisabledDepthTestSubModeRender(quadrantToRender);
			break;
		}
		case eDEPTH_TEST_MODE_EARLY_DEPTH_TEST:
		{
			EarlyDepthTestSubModeRender(quadrantToRender);
			break;
		}
		default:
		{
			ERROR_AND_DIE("Depth Test Sub Mode Render Error: Please provide a valid depth test mode");
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::AlphaBlendingSubModeQuadrantRender(unsigned char subgroupMode, eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor subgroupModeRender(L"Alpha Blending Mode");
	switch ((eAlphaBlendingMode)subgroupMode)
	{
		case eALPHA_BLENDING_MODE_OVER:
		{
			OverAlphaBlendingSubModeRender(quadrantToRender);
			break;
		}
		case eALPHA_BLENDING_MODE_UNDER:
		{
			UnderAlphaBlendingSubModeRender(quadrantToRender);
			break;
		}
		case eALPHA_BLENDING_MODE_PREMULTIPLIED_OVER:
		{
			PremultipliedOverAlphaBlendingSubModeRender(quadrantToRender);
			break;
		}
		case eALPHA_BLENDING_MODE_PREMULTIPLIED_UNDER:
		{
			PremultipliedUnderAlphaBlendingSubModeRender(quadrantToRender);
			break;
		}
		default:
		{
			ERROR_AND_DIE("Alpha Blending Sub Mode Render Error: Please provide a valid alpha blending mode");
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::UAVWritesSubModeQuadrantRender(unsigned char subgroupMode, eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor subgroupModeRender(L"UAV Writes Mode");
	switch ((eUAVWritesMode)subgroupMode)
	{
		case eUAV_WRITE_MODE_WITHOUT_ROV:
		{
			WithoutROVUAVWritesSubModeRender(quadrantToRender);
			break;
		}
		case eUAV_WRITE_MODE_WITH_ROV:
		{
			WithROVUAVWritesSubModeRender(quadrantToRender);
			break;
		}
		default:
		{
			ERROR_AND_DIE("UAV Writes Sub Mode Render Error: Please provide a UAV writing mode");
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::OITSubModeQuadrantRender(unsigned char subgroupMode, eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor subgroupModeRender(L"OIT Mode");
	switch ((eOITMode)subgroupMode)
	{
		case eOIT_MODE_WORST_CASE:
		{
			WorstCaseOITSubModeRender(quadrantToRender);
			break;
		}
		case eOIT_MODE_CPU_SORTED:
		{
			CPUSortedOITSubModeRender(quadrantToRender);
			break;
		}
		case eOIT_MODE_DEPTH_PEELING:
		{
			DepthPeelingOITSubModeRender(quadrantToRender);
			break;
		}
		case eOIT_MODE_VIRTUAL_PIXEL_MAPS:
		{
			VirtualPixelMapsOITSubModeRender(quadrantToRender);
			break;
		}
		case eOIT_MODE_WEIGHTED_BLENDED:
		{
			WeightedBlendedOITSubModeRender(quadrantToRender);
			break;
		}
		case eOIT_MODE_PER_PIXEL_LINKED_LIST:
		{
			PerPixelLinkedListOITSubModeRender(quadrantToRender);
			break;
		}
		case eOIT_MODE_MULTI_LAYERED_ALPHA_BLENDING:
		{
			MultiLayerAlphaBlendingOITSubModeRender(quadrantToRender);
			break;
		}
		default:
		{
			ERROR_AND_DIE("OIT Sub Mode Render Error: Please provide a valid OIT mode");
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::RenderQuadrants() const
{
	RendererAnnotationJanitor screenspacePass(L"Screen Pass");
	g_theRenderer->BindRenderAndDepthTargetViewsToThePipeline();
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	if (!m_isTwoSplitScreen && !m_isFourSplitScreen)
	{
		RendererAnnotationJanitor fullScreenQuad(L"Full Screen Quad");
		g_theRenderer->BindReadableResources(&m_quadrant1RenderTarget, 1, 0, BindingLocation::PIXEL_SHADER);
		g_theRenderer->DrawIndexedBuffer(m_fullScreenQuadIB, m_fullScreenQuadVB, 6);
		g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::PIXEL_SHADER);
	}
	else if (m_isTwoSplitScreen)
	{
		{
			RendererAnnotationJanitor quadrant1Quad(L"Quadrant1 Quad");
			g_theRenderer->BindReadableResources(&m_quadrant1RenderTarget, 1, 0, BindingLocation::PIXEL_SHADER);
			g_theRenderer->DrawIndexedBuffer(m_quadrant1Split2QuadIB, m_quadrant1Split2QuadVB, 6);
			g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::PIXEL_SHADER);
		}

		{
			RendererAnnotationJanitor quadrant1Quad(L"Quadrant2 Quad");
			g_theRenderer->BindReadableResources(&m_quadrant2RenderTarget, 1, 0, BindingLocation::PIXEL_SHADER);
			g_theRenderer->DrawIndexedBuffer(m_quadrant2Split2QuadIB, m_quadrant2Split2QuadVB, 6);
			g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::PIXEL_SHADER);
		}
	}
	else if (m_isFourSplitScreen)
	{
		{
			RendererAnnotationJanitor quadrant1Quad(L"Quadrant1 Quad");
			g_theRenderer->BindReadableResources(&m_quadrant1RenderTarget, 1, 0, BindingLocation::PIXEL_SHADER);
			g_theRenderer->DrawIndexedBuffer(m_quadrant1QuadIB, m_quadrant1QuadVB, 6);
			g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::PIXEL_SHADER);
		}

		{
			RendererAnnotationJanitor quadrant1Quad(L"Quadrant2 Quad");
			g_theRenderer->BindReadableResources(&m_quadrant2RenderTarget, 1, 0, BindingLocation::PIXEL_SHADER);
			g_theRenderer->DrawIndexedBuffer(m_quadrant2QuadIB, m_quadrant2QuadVB, 6);
			g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::PIXEL_SHADER);
		}

		{
			RendererAnnotationJanitor quadrant1Quad(L"Quadrant3 Quad");
			g_theRenderer->BindReadableResources(&m_quadrant3RenderTarget, 1, 0, BindingLocation::PIXEL_SHADER);
			g_theRenderer->DrawIndexedBuffer(m_quadrant3QuadIB, m_quadrant3QuadVB, 6);
			g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::PIXEL_SHADER);
		}

		{
			RendererAnnotationJanitor quadrant1Quad(L"Quadrant4 Quad");
			g_theRenderer->BindReadableResources(&m_quadrant4RenderTarget, 1, 0, BindingLocation::PIXEL_SHADER);
			g_theRenderer->DrawIndexedBuffer(m_quadrant4QuadIB, m_quadrant4QuadVB, 6);
			g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::PIXEL_SHADER);
		}
	}
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}


//--------------------------------------------------------------------------------------------------
void Game::LessThanEqualDepthTestSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor lessThanEqualJanitor(L"Less than or equal");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::CRIMSON);
	g_theRenderer->ClearDepthResource(depthResource);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor translucentRenderPass(L"Translucent Render Pass");
		if (!m_isDepthTestEnabled)
		{
			g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource, true);
		}
		else
		{
			g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
			g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		}
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		DrawScene(eRENDER_PASS_TRANSLUCENT);
	}

	RenderGameModeInfo(quadrantToRender);
}


//--------------------------------------------------------------------------------------------------
void Game::GreaterThanEqualDepthTestSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor lessThanEqualJanitor(L"Greater than or equal");
	D3D11_Resource* rtResource = GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource = GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::TURQUOISE);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		if (!m_isDepthTestEnabled)
		{
			g_theRenderer->ClearDepthResource(depthResource);
		}
		else
		{
			g_theRenderer->ClearDepthResource(depthResource, 0);
			g_theRenderer->SetDepthMode(DepthMode::GREATER);
		}
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor translucentRenderPass(L"Translucent Render Pass");
		if (!m_isDepthTestEnabled)
		{
			g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource, true);
		}
		else
		{
			g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
		}
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		DrawScene(eRENDER_PASS_TRANSLUCENT);
	}

	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	RenderGameModeInfo(quadrantToRender);
}


//--------------------------------------------------------------------------------------------------
void Game::DisabledDepthTestSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor disabledJanitor(L"Depth Test Disabled");
	D3D11_Resource* rtResource = GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource = GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::ORANGE);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor translucentRenderPass(L"Translucent Render Pass");
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		DrawScene(eRENDER_PASS_TRANSLUCENT);
	}
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	RenderGameModeInfo(quadrantToRender);
}


//--------------------------------------------------------------------------------------------------
void Game::EarlyDepthTestSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor earlyDepthTestJanitor(L"Early Depth Stencil");
	D3D11_Resource* rtResource = GetRenderTargetFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::MAGENTA);

	RenderGameModeInfo(quadrantToRender);
}


//--------------------------------------------------------------------------------------------------
void Game::OverAlphaBlendingSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor overAlphaBlendJanitor(L"Over Alpha Blending");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLACK);
	g_theRenderer->ClearDepthResource(depthResource);

	{
		RendererAnnotationJanitor opaquePass(L"Opaque Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor translucentPass(L"Translucent Render Pass");
		g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource, true);
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		DrawScene(eRENDER_PASS_TRANSLUCENT);
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	RenderGameModeInfo(quadrantToRender);
}


//--------------------------------------------------------------------------------------------------
void Game::UnderAlphaBlendingSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor overAlphaBlendJanitor(L"Under Alpha Blending");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(m_opaqueBackgroundQuadrant1RenderTarget, depthResource);
	g_theRenderer->ClearRenderTargetResource(m_opaqueBackgroundQuadrant1RenderTarget, Rgba8(0, 0, 0, 0));
	g_theRenderer->ClearDepthResource(depthResource);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants();
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor translucentPass(L"Translucent Render Pass");
		g_theRenderer->BindShader(m_defaultPremultipliedAlphaShader);
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA_UNDER);
		g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLACK);
		g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource, true);
		DrawScene(eRENDER_PASS_TRANSLUCENT);
	}

	// Composite accumulated color with the background/opaque color
	{
		RendererAnnotationJanitor compositePass(L"Accumulated Color Composite");
		g_theRenderer->BindRenderAndDepthTargetViewsToThePipeline();
		g_theRenderer->BindReadableResources(&m_opaqueBackgroundQuadrant1RenderTarget, 1, 0, BindingLocation::COMPUTE_SHADER);
		g_theRenderer->BindWritableResourcesToComputeShader(&rtResource, 1, 1);
		g_theRenderer->BindShader(m_underCompositeShader, BindingLocation::COMPUTE_SHADER);
		g_theRenderer->ComputeShaderDispatch(s_threadGroupX, s_threadGroupY, 1);
		g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::COMPUTE_SHADER);
		g_theRenderer->UnbindWritableResources(1, 1, BindingLocation::COMPUTE_SHADER);
	}

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	RenderGameModeInfo(quadrantToRender);
}


//--------------------------------------------------------------------------------------------------
void Game::PremultipliedOverAlphaBlendingSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor overAlphaBlendJanitor(L"Premultiplied Over Alpha Blending");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLUE);
	g_theRenderer->ClearDepthResource(depthResource);

	RenderGameModeInfo(quadrantToRender);
}


//--------------------------------------------------------------------------------------------------
void Game::PremultipliedUnderAlphaBlendingSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor overAlphaBlendJanitor(L"Premultiplied Under Alpha Blending");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::YELLOW);
	g_theRenderer->ClearDepthResource(depthResource);

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource, true);
	RenderGameModeInfo(quadrantToRender);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}


//--------------------------------------------------------------------------------------------------
void Game::WithoutROVUAVWritesSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor withoutROVUAVWriteJanitor(L"Without ROV UAV Write");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLACK);
	g_theRenderer->ClearDepthResource(depthResource);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor translucentRenderPass(L"Translucent Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->BindUAVsRenderAndDepthTargets(1, 1, &rtResource, 0, nullptr, depthResource, true);
		g_theRenderer->BindShader(m_uavWriteWithoutROVShader);
		DrawScene(eRENDER_PASS_TRANSLUCENT);
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindRenderAndDepthResources(rtResource);
	RenderGameModeInfo(quadrantToRender);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}


//--------------------------------------------------------------------------------------------------
void Game::WithROVUAVWritesSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor withROVUAVWriteJanitor(L"With ROV UAV Write");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLACK);
	g_theRenderer->ClearDepthResource(depthResource);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		DrawScene(eRENDER_PASS_OPAQUE);
	}


	{
		RendererAnnotationJanitor opaqueRenderPass(L"Translucent Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->BindUAVsRenderAndDepthTargets(1, 1, &rtResource, 0, nullptr, depthResource, true);
		g_theRenderer->BindShader(m_uavWriteWithROVShader);
		DrawScene(eRENDER_PASS_TRANSLUCENT);
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindRenderAndDepthResources(rtResource);
	RenderGameModeInfo(quadrantToRender);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}


//--------------------------------------------------------------------------------------------------
void Game::WorstCaseOITSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor overAlphaBlendJanitor(L"Worst Case OIT");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLACK);
	g_theRenderer->ClearDepthResource(depthResource);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor translucentRenderPass(L"Translucent Render Pass");
		g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource, true);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		DrawScene(eRENDER_PASS_TRANSLUCENT);
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	RenderGameModeInfo(quadrantToRender);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}


//--------------------------------------------------------------------------------------------------
void Game::CPUSortedOITSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor overAlphaBlendJanitor(L"CPU Sorted OIT");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLACK);
	g_theRenderer->ClearDepthResource(depthResource);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor translucentRenderPass(L"Translucent Render Pass");
		g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource, true);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		DrawSortedTranslucentScene();
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	RenderGameModeInfo(quadrantToRender);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}


//--------------------------------------------------------------------------------------------------
void Game::DepthPeelingOITSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor depthPeelingJanitor(L"Depth Peeling OIT");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLACK);
	g_theRenderer->ClearRenderTargetResource(m_intermediateTarget, Rgba8::BLACK);
	g_theRenderer->ClearRenderTargetResource(m_depthPeelingIntermediateRenderTarget, Rgba8::BLACK);
	g_theRenderer->ClearDepthResource(depthResource);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor depthPeelingPasses(L"Translucent Render Pass");
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		for (unsigned int passNo = 0; passNo < m_numOfDepthPeelingPasses; ++passNo)
		{
#if defined(_DEBUG)
			std::string pass = Stringf("PassNo: %u", passNo);
			std::wstring wstringPass;
			wstringPass.resize(pass.size() + 1);
			MultiByteToWideChar(GetACP(), 0, pass.c_str(), -1, (LPWSTR)wstringPass.c_str(), (int)(pass.size() + 1));
			RendererAnnotationJanitor passNoAnnotator(wstringPass.c_str());
#endif
			g_theRenderer->ClearRenderTargetResource(m_intermediateRenderTarget, Rgba8::BLACK);
			g_theRenderer->ClearDepthResource(m_depthPeelingIntermediateDepthTarget);
			g_theRenderer->BindRenderAndDepthResources(m_intermediateRenderTarget, m_depthPeelingIntermediateDepthTarget);
			D3D11_Resource* depthResources[2] = { m_intermediateTarget, depthResource };
			g_theRenderer->BindReadableResources(depthResources, 2, 1, BindingLocation::PIXEL_SHADER);
			g_theRenderer->BindShader(m_depthPeelingShader);
			DrawScene(eRENDER_PASS_TRANSLUCENT);
			g_theRenderer->UnbindReadableResources(2, 1, BindingLocation::PIXEL_SHADER);

			CompositeDepthPeelingDepth(m_depthPeelingIntermediateDepthTarget);
			CompositeDepthPeelingColor(m_depthPeelingIntermediateRenderTarget);
		}
	}

	// Composite the accumulated color with the background/Opaque texture
	{
		RendererAnnotationJanitor finalComposite(L"Final Composite Pass");
		g_theRenderer->BindReadableResources(&m_depthPeelingIntermediateRenderTarget, 1, 0, BindingLocation::COMPUTE_SHADER);
		g_theRenderer->BindWritableResourcesToComputeShader(&rtResource, 1, 0);
		g_theRenderer->BindShader(m_depthPeelingFinalBackgroundPassShader, BindingLocation::COMPUTE_SHADER);
		g_theRenderer->ComputeShaderDispatch(s_threadGroupX, s_threadGroupY, 1);
		g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::COMPUTE_SHADER);
		g_theRenderer->UnbindWritableResources(1, 0, BindingLocation::COMPUTE_SHADER);
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->BindRenderAndDepthResources(rtResource);
	RenderGameModeInfo(quadrantToRender);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}


//--------------------------------------------------------------------------------------------------
void Game::VirtualPixelMapsOITSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor vpmJanitor(L"Virtual Pixel Maps OIT");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLACK);
	g_theRenderer->ClearRenderTargetResource(m_intermediateTarget, Rgba8::WHITE);
	g_theRenderer->ClearDepthResource(depthResource);

	{
		RendererAnnotationJanitor depthPeelingPasses(L"Opaque Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		DrawScene(eRENDER_PASS_OPAQUE);
		CompositeVPMDepth(depthResource);
	}

	{
		RendererAnnotationJanitor translucentPass(L"Translucent Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::GREATER);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		for (unsigned int passNo = 0; passNo < m_numOfDepthPeelingPasses; ++passNo)
		{
#if defined(_DEBUG)
			std::string pass = Stringf("PassNo: %u", passNo);
			std::wstring wstringPass;
			wstringPass.resize(pass.size() + 1);
			MultiByteToWideChar(GetACP(), 0, pass.c_str(), -1, (LPWSTR)wstringPass.c_str(), (int)(pass.size() + 1));
			RendererAnnotationJanitor passNoAnnotator(wstringPass.c_str());
#endif
			g_theRenderer->ClearDepthResource(depthResource, 0.f);
			g_theRenderer->ClearRenderTargetResource(m_intermediateRenderTarget, Rgba8::BLACK);
			g_theRenderer->BindRenderAndDepthResources(m_intermediateRenderTarget, depthResource);
			g_theRenderer->BindReadableResources(&m_intermediateTarget, 1, 1, BindingLocation::PIXEL_SHADER);
			g_theRenderer->BindShader(m_vpmPeelingShader);
			DrawScene(eRENDER_PASS_TRANSLUCENT);
			g_theRenderer->UnbindReadableResources(1, 1, BindingLocation::PIXEL_SHADER);

			CompositeVPMDepth(depthResource);
			CompositeVPMColor(rtResource);
		}
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->BindRenderAndDepthResources(rtResource);
	RenderGameModeInfo(quadrantToRender);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}


//--------------------------------------------------------------------------------------------------
void Game::WeightedBlendedOITSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor overAlphaBlendJanitor(L"Weighted Blended OIT");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLACK);
	g_theRenderer->ClearDepthResource(depthResource);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor translucentRenderPass(L"Translucent Render Pass");
		{
			RendererAnnotationJanitor populateAccumulationAndRevealage(L"Populate Accumulation and Revealage Targets");
			g_theRenderer->ClearRenderTargetResource(m_accumulationTargetWeightedBlended, Rgba8::BLACK);
			g_theRenderer->ClearRenderTargetResource(m_revealageTargetWeightedBlended, Rgba8::WHITE);
			D3D11_Resource* weightedBlendedRTs[2] = { m_accumulationTargetWeightedBlended, m_revealageTargetWeightedBlended };
			g_theRenderer->BindRenderAndDepthResources(2, weightedBlendedRTs, depthResource, true);
			g_theRenderer->BindShader(m_populateAccumRevealRTsShader);
			g_theRenderer->SetCustomBlendMode();
			DrawScene(eRENDER_PASS_TRANSLUCENT);
			g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		}

		{
			RendererAnnotationJanitor compositePass(L"Composite Pass");
			g_theRenderer->BeginCamera(m_screenCamera);
			D3D11_Resource* readableTexs[2] = { m_accumulationTargetWeightedBlended, m_revealageTargetWeightedBlended };
			g_theRenderer->BindRenderAndDepthResources(rtResource);
			g_theRenderer->SetDepthMode(DepthMode::DISABLED);
			g_theRenderer->BindShader(m_weightedBlendedCompositeShader);
			g_theRenderer->BindReadableResources(readableTexs, 2, 1, BindingLocation::PIXEL_SHADER);
			g_theRenderer->SetModelConstants();
			g_theRenderer->DrawIndexedBuffer(m_fullScreenQuadIB, m_fullScreenQuadVB, 6);
			g_theRenderer->UnbindReadableResources(2, 1);
			g_theRenderer->EndCamera(m_screenCamera);
		}
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	RenderGameModeInfo(quadrantToRender);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}


//--------------------------------------------------------------------------------------------------
void Game::PerPixelLinkedListOITSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor overAlphaBlendJanitor(L"Per Pixel Linked List OIT");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	eNodes			numOfNodesToUse =	GetNodesFromQuadrant(quadrantToRender);
	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLACK);
	g_theRenderer->ClearDepthResource(depthResource);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor translucentRenderPass(L"Translucent Render Pass");
		D3D11_Resource* linkedListRelatedResources[2] = { m_headNodeByteAddressBuffer_2, m_fragmentsPerPixelStructuredBuffer_2 };
		if (numOfNodesToUse == eNODES_4)
		{
			linkedListRelatedResources[0] = m_headNodeByteAddressBuffer_4;
			linkedListRelatedResources[1] = m_fragmentsPerPixelStructuredBuffer_4;
		}
		if (numOfNodesToUse == eNODES_32)
		{
			linkedListRelatedResources[0] = m_headNodeByteAddressBuffer_32;
			linkedListRelatedResources[1] = m_fragmentsPerPixelStructuredBuffer_32;
		}

		{
			RendererAnnotationJanitor clearBuffers(L"Clear Buffers");
			g_theRenderer->UnbindRenderAndDepthTargets();
			g_theRenderer->BindWritableResourcesToComputeShader(linkedListRelatedResources, 2, 1);
			if (numOfNodesToUse == eNODES_2)
			{
				g_theRenderer->BindShader(m_perPixelLinkedListClearBufferShader_2, BindingLocation::COMPUTE_SHADER);
			}
			else if (numOfNodesToUse == eNODES_4)
			{
				g_theRenderer->BindShader(m_perPixelLinkedListClearBufferShader_4, BindingLocation::COMPUTE_SHADER);
			}
			else
			{
				g_theRenderer->BindShader(m_perPixelLinkedListClearBufferShader_32, BindingLocation::COMPUTE_SHADER);
			}
			g_theRenderer->ComputeShaderDispatch(s_threadGroupX, s_threadGroupY, 1);
			g_theRenderer->UnbindWritableResources(2, 1, BindingLocation::COMPUTE_SHADER);
		}

		{
			RendererAnnotationJanitor populateLinkedList(L"Populate Fragment Linked List");
			g_theRenderer->BindUAVsRenderAndDepthTargets(2, 1, linkedListRelatedResources, 0, nullptr, depthResource, true);
			g_theRenderer->BindShader(m_populatePerPixelLinkedListShader);
			g_theRenderer->BindConstantBuffer(g_gameConstantsSlot, m_gameConstantBuffer);
			DrawScene(eRENDER_PASS_TRANSLUCENT);
			g_theRenderer->UnbindWritableResources(2, 1);
		}

		{
			RendererAnnotationJanitor composite(L"Populating the RT");
			g_theRenderer->BindWritableResourcesToComputeShader(&rtResource, 1, 1);
			g_theRenderer->BindReadableResources(linkedListRelatedResources, 2, 1, BindingLocation::COMPUTE_SHADER);
			if (numOfNodesToUse == eNODES_2)
			{
				g_theRenderer->BindShader(m_perPixelLinkedListCompositePassShader_2, BindingLocation::COMPUTE_SHADER);
			}
			else if (numOfNodesToUse == eNODES_4)
			{
				g_theRenderer->BindShader(m_perPixelLinkedListCompositePassShader_4, BindingLocation::COMPUTE_SHADER);
			}
			else
			{
				g_theRenderer->BindShader(m_perPixelLinkedListCompositePassShader_32, BindingLocation::COMPUTE_SHADER);
			}
			g_theRenderer->ComputeShaderDispatch(s_threadGroupX, s_threadGroupY, 1);
			g_theRenderer->UnbindWritableResources(1, 1, BindingLocation::COMPUTE_SHADER);
			g_theRenderer->UnbindReadableResources(2, 1, BindingLocation::COMPUTE_SHADER);
		}
	}

	g_theRenderer->BindRenderAndDepthResources(rtResource);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	RenderGameModeInfo(quadrantToRender);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}


//--------------------------------------------------------------------------------------------------
void Game::MultiLayerAlphaBlendingOITSubModeRender(eSplitScreenQuadrant quadrantToRender /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	RendererAnnotationJanitor overAlphaBlendJanitor(L"Multi Layered Alpha Blending");
	D3D11_Resource* rtResource		=	GetRenderTargetFromQuadrantInfo(quadrantToRender);
	D3D11_Resource* depthResource	=	GetDepthResourceFromQuadrantInfo(quadrantToRender);
	eNodes			nodesToUse		=	GetNodesFromQuadrant(quadrantToRender);

	g_theRenderer->BindRenderAndDepthResources(rtResource, depthResource);
	g_theRenderer->ClearRenderTargetResource(rtResource, Rgba8::BLACK);
	g_theRenderer->ClearDepthResource(depthResource);
	g_theRenderer->ClearRenderTargetResource(m_intermediateRenderTarget, Rgba8::BLACK);

	{
		RendererAnnotationJanitor opaqueRenderPass(L"Opaque Render Pass");
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);
		DrawScene(eRENDER_PASS_OPAQUE);
	}

	{
		RendererAnnotationJanitor translucentRenderPass(L"Translucent Render Pass");
		{
			RendererAnnotationJanitor populateBlendingArray(L"Populate Blending Array");
			D3D11_Resource* uavResources[2] = { m_unoptimizedMLABUntouchedFragmentMask, m_unoptimizedMLABStructuredBuffer_2 };
			if (nodesToUse == eNODES_4)
			{
				uavResources[1] = m_unoptimizedMLABStructuredBuffer_4;
				g_theRenderer->BindShader(m_unoptimizedMLABPopulateBlendingArrayShader_4);
			}
			else if (nodesToUse == eNODES_32)
			{
				uavResources[1] = m_unoptimizedMLABStructuredBuffer_32;
				g_theRenderer->BindShader(m_unoptimizedMLABPopulateBlendingArrayShader_32);
			}
			else
			{
				g_theRenderer->BindShader(m_unoptimizedMLABPopulateBlendingArrayShader_2);
			}
			g_theRenderer->BindUAVsRenderAndDepthTargets(2, 1, uavResources, 0, nullptr, depthResource, true);
			g_theRenderer->BindConstantBuffer(g_screenConstantsSlot, m_screenConstantBuffer);
			DrawScene(eRENDER_PASS_TRANSLUCENT);
			g_theRenderer->UnbindUAVsRenderAndDepthTargets(2, 1, 0);
		}

		{
			RendererAnnotationJanitor populateCustomRT(L"Populate Custom RT");
			if (nodesToUse == eNODES_2)
			{
				g_theRenderer->BindReadableResources(&m_unoptimizedMLABStructuredBuffer_2, 1, 0, BindingLocation::COMPUTE_SHADER);
				g_theRenderer->BindShader(m_unoptimizedMLABPopulateRenderTargetShader_2, BindingLocation::COMPUTE_SHADER);
			}
			else if (nodesToUse == eNODES_4)
			{
				g_theRenderer->BindReadableResources(&m_unoptimizedMLABStructuredBuffer_4, 1, 0, BindingLocation::COMPUTE_SHADER);
				g_theRenderer->BindShader(m_unoptimizedMLABPopulateRenderTargetShader_4, BindingLocation::COMPUTE_SHADER);
			}
			else
			{
				g_theRenderer->BindReadableResources(&m_unoptimizedMLABStructuredBuffer_32, 1, 0, BindingLocation::COMPUTE_SHADER);
				g_theRenderer->BindShader(m_unoptimizedMLABPopulateRenderTargetShader_32, BindingLocation::COMPUTE_SHADER);
			}
			D3D11_Resource* const writableResources[2] = { rtResource, m_unoptimizedMLABUntouchedFragmentMask };
			g_theRenderer->BindWritableResourcesToComputeShader(writableResources, 2, 0);
			g_theRenderer->BindConstantBuffer(g_screenConstantsSlot, m_screenConstantBuffer, BindingLocation::COMPUTE_SHADER);
			g_theRenderer->ComputeShaderDispatch(s_threadGroupX, s_threadGroupY, 1);
			g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::COMPUTE_SHADER);
			g_theRenderer->UnbindWritableResources(2, 0, BindingLocation::COMPUTE_SHADER);
		}
	}

	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->BindRenderAndDepthResources(rtResource);
	RenderGameModeInfo(quadrantToRender);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}


//--------------------------------------------------------------------------------------------------
void Game::CompositeDepthPeelingDepth(D3D11_Resource* depthResource) const
{
	RendererAnnotationJanitor depthComposite(L"Depth Composite");
	g_theRenderer->UnbindRenderAndDepthTargets();
	g_theRenderer->BindReadableResources(&depthResource, 1, 0, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->BindWritableResourcesToComputeShader(&m_intermediateTarget, 1, 0);
	g_theRenderer->BindShader(m_depthPeelingDepthCompositeShader, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->ComputeShaderDispatch(s_threadGroupX, s_threadGroupY, 1);
	g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->UnbindWritableResources(1, 0, BindingLocation::COMPUTE_SHADER);
}


//--------------------------------------------------------------------------------------------------
void Game::CompositeDepthPeelingColor(D3D11_Resource* colorTarget) const
{
	RendererAnnotationJanitor depthComposite(L"Color Composite");
	g_theRenderer->UnbindRenderAndDepthTargets();
	g_theRenderer->BindReadableResources(&m_intermediateRenderTarget, 1, 0, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->BindWritableResourcesToComputeShader(&colorTarget, 1, 0);
	g_theRenderer->BindShader(m_depthPeelingColorCompositeShader, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->ComputeShaderDispatch(s_threadGroupX, s_threadGroupY, 1);
	g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->UnbindWritableResources(1, 0, BindingLocation::COMPUTE_SHADER);
}


//--------------------------------------------------------------------------------------------------
void Game::CompositeVPMDepth(D3D11_Resource*& depthResource) const
{
	RendererAnnotationJanitor depthComposite(L"Depth Composite");
	g_theRenderer->UnbindRenderAndDepthTargets();
	g_theRenderer->BindReadableResources(&depthResource, 1, 0, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->BindWritableResourcesToComputeShader(&m_intermediateTarget, 1, 0);
	g_theRenderer->BindShader(m_vpmDepthCompositeShader, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->ComputeShaderDispatch(s_threadGroupX, s_threadGroupY, 1);
	g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->UnbindWritableResources(1, 0, BindingLocation::COMPUTE_SHADER);
}


//--------------------------------------------------------------------------------------------------
void Game::CompositeVPMColor(D3D11_Resource*& colorTarget) const
{
	RendererAnnotationJanitor depthComposite(L"Color Composite");
	g_theRenderer->UnbindRenderAndDepthTargets();
	g_theRenderer->BindReadableResources(&m_intermediateRenderTarget, 1, 0, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->BindWritableResourcesToComputeShader(&colorTarget, 1, 0);
	g_theRenderer->BindShader(m_vpmColorCompositeShader, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->ComputeShaderDispatch(s_threadGroupX, s_threadGroupY, 1);
	g_theRenderer->UnbindReadableResources(1, 0, BindingLocation::COMPUTE_SHADER);
	g_theRenderer->UnbindWritableResources(1, 0, BindingLocation::COMPUTE_SHADER);
}


//--------------------------------------------------------------------------------------------------
void Game::DrawScene(eRenderPass renderPass) const
{
	Scene*					 const& scene					=	m_scenes[m_sceneIndex];
	bool							isTranslucentPass		=	renderPass == eRENDER_PASS_TRANSLUCENT;
	std::vector<SceneObject> const& sceneObjectsToRender	=	isTranslucentPass ? scene->m_translucentObjects : scene->m_opaqueObjects;

	if (isTranslucentPass)
	{
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	}

	if (m_isRetainedModeRendering || renderPass == eRENDER_PASS_OPAQUE)
	{
		static Mat44 identityTransform;
		for (unsigned int meshIndex = 0; meshIndex < sceneObjectsToRender.size(); ++meshIndex)
		{
			SceneObject const& currentMesh = sceneObjectsToRender[meshIndex];
			RendererAnnotationJanitor meshRender(currentMesh.m_debugName);

			g_theRenderer->SetModelConstants(identityTransform, Rgba8(255, 255, 255, currentMesh.m_color.a));
			g_theRenderer->BindReadableResources(&currentMesh.m_textureResource, currentMesh.m_numOfTexturesBound, 0, BindingLocation::PIXEL_SHADER);
			if (!m_isTextured)
			{
				g_theRenderer->BindTexture(nullptr, BindingLocation::PIXEL_SHADER);
				g_theRenderer->SetModelConstants(identityTransform, currentMesh.m_color);
			}
			if (m_sceneIndex == eTRANSLUCENT_SCENE_FOG && currentMesh.m_type == eMESH_TYPE_BILLBOARDED_QUAD && currentMesh.m_color.a != 255)
			{
				if (!m_isTextured)
				{
					g_theRenderer->SetModelConstants(currentMesh.m_transform, currentMesh.m_color);
				}
				else
				{
					g_theRenderer->SetModelConstants(currentMesh.m_transform, Rgba8(255, 255, 255, currentMesh.m_color.a));
				}
			}

			g_theRenderer->DrawIndexedBuffer(currentMesh.m_indexBuffer, currentMesh.m_vertexBuffer, currentMesh.m_numOfIndexes);
		}
		return;
	}

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawIndexedArray((int)scene->m_sceneVertices.size(), scene->m_sceneVertices.data(), (int)scene->m_sceneIndexes.size(), scene->m_sceneIndexes.data());
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
}


//--------------------------------------------------------------------------------------------------
void Game::DrawSortedTranslucentScene() const
{
	Scene*					 const& scene					=	m_scenes[m_sceneIndex];
	std::vector<SceneObject> const& sceneObjectsToRender	=	scene->m_sortedTranslucentObjects;

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	if (m_isRetainedModeRendering)
	{
		static Mat44 identityTransform;
		for (unsigned int meshIndex = 0; meshIndex < sceneObjectsToRender.size(); ++meshIndex)
		{
			SceneObject const& currentMesh = sceneObjectsToRender[meshIndex];
			RendererAnnotationJanitor meshRender(currentMesh.m_debugName);

			g_theRenderer->SetModelConstants(identityTransform, Rgba8(255, 255, 255, currentMesh.m_color.a));
			g_theRenderer->BindReadableResources(&currentMesh.m_textureResource, currentMesh.m_numOfTexturesBound, 0, BindingLocation::PIXEL_SHADER);
			if (!m_isTextured)
			{
				g_theRenderer->BindTexture(nullptr, BindingLocation::PIXEL_SHADER);
				g_theRenderer->SetModelConstants(identityTransform, currentMesh.m_color);
			}
			if (m_sceneIndex == eTRANSLUCENT_SCENE_FOG && currentMesh.m_type == eMESH_TYPE_BILLBOARDED_QUAD && currentMesh.m_color.a != 255)
			{
				if (!m_isTextured)
				{
					g_theRenderer->SetModelConstants(currentMesh.m_transform, currentMesh.m_color);
				}
				else
				{
					g_theRenderer->SetModelConstants(currentMesh.m_transform, Rgba8(255, 255, 255, currentMesh.m_color.a));
				}
			}

			g_theRenderer->DrawIndexedBuffer(currentMesh.m_indexBuffer, currentMesh.m_vertexBuffer, currentMesh.m_numOfIndexes);
		}
		return;
	}

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawIndexedArray((int)scene->m_sceneVertices.size(), scene->m_sceneVertices.data(), (int)scene->m_sceneIndexes.size(), scene->m_sceneIndexes.data());
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
}


//--------------------------------------------------------------------------------------------------
char* Game::GetGameModeAsText() const
{
	switch (m_gameMode)
	{
		case eGAME_MODE_DEPTH_TEST:
		{
			return (char*)("Depth Test");
		}
		case eGAME_MODE_ALPHA_BLENDING:
		{
			return (char*)("Alpha Blending");
		}
		case eGAME_MODE_OIT:
		{
			return (char*)("OIT");
		}
		case eGAME_MODE_UAV_WRITES:
		{
			return (char*)("UAV Writes");
		}
		default:
		{
			ERROR_AND_DIE("Please provide a valid game mode");
		}
	}
}


//--------------------------------------------------------------------------------------------------
char* Game::GetUAVWritesSubGroupModeAsText(eSplitScreenQuadrant quadrant /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	switch (quadrant)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			return GetUAVWritesSubGroupModeAsText(m_quadrant1SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			return GetUAVWritesSubGroupModeAsText(m_quadrant2SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			return GetUAVWritesSubGroupModeAsText(m_quadrant3SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			return GetUAVWritesSubGroupModeAsText(m_quadrant4SubgroupMode);
		}
		default:
		{
			ERROR_AND_DIE("Invalid Quadrant: Please provide a valid quadrant to fetch UAV Writes subgroup game mode as text");
		}
	}
}


//--------------------------------------------------------------------------------------------------
char* Game::GetUAVWritesSubGroupModeAsText(unsigned char subGroupMode /*= 0*/) const
{
	switch (eUAVWritesMode(subGroupMode))
	{
		case eUAV_WRITE_MODE_WITHOUT_ROV:
		{
			return (char*)("Sub Group Mode: Without ROV ");
		}
		case eUAV_WRITE_MODE_WITH_ROV:
		{
			return (char*)("Sub Group Mode: With ROV ");
		}
		default:
		{
			ERROR_AND_DIE("Invalid UAV writes subgroup mode")
		}
	}
}


//--------------------------------------------------------------------------------------------------
char* Game::GetOITSubGroupModeAsText(eSplitScreenQuadrant quadrant /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	switch (quadrant)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			return GetOITSubGroupModeAsText(m_quadrant1SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			return GetOITSubGroupModeAsText(m_quadrant2SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			return GetOITSubGroupModeAsText(m_quadrant3SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			return GetOITSubGroupModeAsText(m_quadrant4SubgroupMode);
		}
		default:
		{
			ERROR_AND_DIE("Invalid Quadrant: Please provide a valid quadrant to fetch OIT subgroup game mode as text");
		}
	}
}


//--------------------------------------------------------------------------------------------------
char* Game::GetOITSubGroupModeAsText(unsigned char subGroupMode /*= 0*/) const
{
	switch (eOITMode(subGroupMode))
	{
		case eOIT_MODE_WORST_CASE:
		{
			return (char*)("Sub Group Mode: Worst Case ");
		}
		case eOIT_MODE_CPU_SORTED:
		{
			return (char*)("Sub Group Mode: CPU Sorted ");
		}
		case eOIT_MODE_DEPTH_PEELING:
		{
			return (char*)("Sub Group Mode: Depth Peeling ");
		}
		case eOIT_MODE_VIRTUAL_PIXEL_MAPS:
		{
			return (char*)("Sub Group Mode: Virtual Pixel Maps ");
		}
		case eOIT_MODE_WEIGHTED_BLENDED:
		{
			return (char*)("Sub Group Mode: Weighted Blended ");
		}
		case eOIT_MODE_PER_PIXEL_LINKED_LIST:
		{
			return (char*)("Sub Group Mode: Per Pixel Linked List ");
		}
		case eOIT_MODE_MULTI_LAYERED_ALPHA_BLENDING:
		{
			return (char*)("Sub Group Mode: Multi Layered Alpha Blending ");
		}
		default:
		{
			ERROR_AND_DIE("Invalid OIT subgroup mode")
		}
	}
}


//--------------------------------------------------------------------------------------------------
char* Game::GetTranslucentModeAsText() const
{
	switch (m_quadrantInControl)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			return GetTranslucentModeAsText(m_translucentModeQuadrant1);
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			return GetTranslucentModeAsText(m_translucentModeQuadrant2);
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			return GetTranslucentModeAsText(m_translucentModeQuadrant3);
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			return GetTranslucentModeAsText(m_translucentModeQuadrant4);
		}
		default:
		{
			ERROR_AND_DIE("Please provide a valid quadrant in control");
		}
	}
}


//--------------------------------------------------------------------------------------------------
char* Game::GetTranslucentModeAsText(eTranslucentMode translucentMode) const
{
	switch (translucentMode)
	{
		case eTRANSLUCENT_MODE_WORST_CASE:
		{
			return (char*)("Problem");
		}
		case eTRANSLUCENT_MODE_SORTED_DRAW_CALLS:
		{
			return (char*)("Sorted Draw Calls");
		}
		default:
		{
			ERROR_AND_DIE("Please provide a valid translucent mode");
		}
	}
}


//--------------------------------------------------------------------------------------------------
D3D11_Resource* Game::GetRenderTargetFromQuadrantInfo(eSplitScreenQuadrant quadrant) const
{
	switch (quadrant)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			return m_quadrant1RenderTarget;
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			return m_quadrant2RenderTarget;
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			return m_quadrant3RenderTarget;
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			return m_quadrant4RenderTarget;
		}
		default:
		{
			ERROR_AND_DIE("Invalid Quadrant: Please provide a valid quadrant to fetch render target");
		}
	}
}


//--------------------------------------------------------------------------------------------------
D3D11_Resource* Game::GetDepthResourceFromQuadrantInfo(eSplitScreenQuadrant quadrant) const
{
	switch (quadrant)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			return m_quadrant1DepthTarget;
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			return m_quadrant2DepthTarget;
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			return m_quadrant3DepthTarget;
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			return m_quadrant4DepthTarget;
		}
		default:
		{
			ERROR_AND_DIE("Invalid Quadrant: Please provide a valid quadrant to fetch render target");
		}
	}
}


//--------------------------------------------------------------------------------------------------
eTranslucentScene Game::GetSceneFromName(std::string const& sceneName) const
{
	if (sceneName == "TwoQuads")
	{
		return eTRANSLUCENT_SCENE_TWO_QUADS;
	}

	if (sceneName == "MultipleQuads")
	{
		return eTRANSLUCENT_SCENE_MULTIPLE_QUADS;
	}

	if (sceneName == "Bunnies")
	{
		return eTRANSLUCENT_SCENE_BUNNIES;
	}

	if (sceneName == "Teapots")
	{
		return eTRANSLUCENT_SCENE_TEAPOTS;
	}

	if (sceneName == "Fridge")
	{
		return eTRANSLUCENT_SCENE_FRIDGE;
	}

	if (sceneName == "Car")
	{
		return eTRANSLUCENT_SCENE_CAR;
	}

	if (sceneName == "IntersectingMeshes")
	{
		return eTRANSLUCENT_SCENE_INTERSECTING_MESHES;
	}

	if (sceneName == "Fog")
	{
		return eTRANSLUCENT_SCENE_FOG;
	}

	ERROR_AND_DIE("Invalid Scene Name");
}


//--------------------------------------------------------------------------------------------------
eMeshType Game::GetMeshTypeFromText(std::string const& meshTypeAsText) const
{
	if (meshTypeAsText == "Quad")
	{
		return eMESH_TYPE_QUAD;
	}

	if (meshTypeAsText == "OBB")
	{
		return eMESH_TYPE_UNI_TEXTURED_OBB;
	}

	if (meshTypeAsText == "Sphere")
	{
		return eMESH_TYPE_SPHERE;
	}

	if (meshTypeAsText == "Teapot")
	{
		return eMESH_TYPE_TEPOT;
	}

	if (meshTypeAsText == "Bunny")
	{
		return eMESH_TYPE_BUNNY;
	}

	if (meshTypeAsText == "FridgeDoor")
	{
		return eMESH_TYPE_FRIDGE_DOOR;
	}

	if (meshTypeAsText == "FridgeBody")
	{
		return eMESH_TYPE_FRIDGE_BODY;
	}

	if (meshTypeAsText == "BillboardedQuad")
	{
		return eMESH_TYPE_BILLBOARDED_QUAD;
	}

	if (meshTypeAsText == "CarBody")
	{
		return eMESH_TYPE_CAR_MAIN_BODY;
	}

	if (meshTypeAsText == "CarGlass1")
	{
		return eMESH_TYPE_CAR_GLASS_1;
	}

	if (meshTypeAsText == "CarGlass2")
	{
		return eMESH_TYPE_CAR_GLASS_2;
	}

	ERROR_AND_DIE("Invalid Mesh Type");
}


//--------------------------------------------------------------------------------------------------
eTranslucentScene Game::GetSceneFromQuadrant(eSplitScreenQuadrant quadrantToRender) const
{
	switch (quadrantToRender)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			return eTranslucentScene(m_quadrant1SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			return eTranslucentScene(m_quadrant2SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			return eTranslucentScene(m_quadrant3SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			return eTranslucentScene(m_quadrant4SubgroupMode);
		}
		default:
		{
			ERROR_AND_DIE("Please provide a valid quadrant to render");
		}
	}
}


//--------------------------------------------------------------------------------------------------
bool Game::IsNodeBasedOITMode(eSplitScreenQuadrant quadrant /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	switch (quadrant)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			return IsNodeBasedOITMode(m_quadrant1SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			return IsNodeBasedOITMode(m_quadrant2SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			return IsNodeBasedOITMode(m_quadrant3SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			return IsNodeBasedOITMode(m_quadrant4SubgroupMode);
		}
		default:
		{
			ERROR_AND_DIE("Invalid Quadrant: Please provide a valid quadrant to fetch if the OIT mode is node based");
		}
	}
}


//--------------------------------------------------------------------------------------------------
bool Game::IsNodeBasedOITMode(unsigned char subGroupMode /*= 0*/) const
{
	switch (eOITMode(subGroupMode))
	{
		case eOIT_MODE_WORST_CASE:
		{
			return false;
		}
		case eOIT_MODE_CPU_SORTED:
		{
			return false;
		}
		case eOIT_MODE_DEPTH_PEELING:
		{
			return false;
		}
		case eOIT_MODE_VIRTUAL_PIXEL_MAPS:
		{
			return false;
		}
		case eOIT_MODE_WEIGHTED_BLENDED:
		{
			return false;
		}
		case eOIT_MODE_PER_PIXEL_LINKED_LIST:
		{
			return true;
		}
		case eOIT_MODE_MULTI_LAYERED_ALPHA_BLENDING:
		{
			return true;
		}
		default:
		{
			ERROR_AND_DIE("Invalid OIT subgroup mode")
		}
	}
}


//--------------------------------------------------------------------------------------------------
eNodes Game::GetNodesFromQuadrant(eSplitScreenQuadrant quadrant) const
{
	switch (quadrant)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			return m_quadrant1Nodes;
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			return m_quadrant2Nodes;
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			return m_quadrant3Nodes;
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			return m_quadrant4Nodes;
		}
		default:
		{
			ERROR_AND_DIE("Invalid Quadrant");
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::AddSceneObject(SceneObject* sceneObjectToAdd)
{
	(void)sceneObjectToAdd;
}


//--------------------------------------------------------------------------------------------------
void Game::UpdateNodeCount(eNodes& nodesToUpdate)
{
	unsigned char currentNumOfNodes = nodesToUpdate;
	currentNumOfNodes++;
	if (currentNumOfNodes >= eNODES_COUNT)
	{
		currentNumOfNodes = 0;
	}
	nodesToUpdate = (eNodes)currentNumOfNodes;
}


//--------------------------------------------------------------------------------------------------
char* Game::GetTranslucentSceneAsText() const
{
	switch (m_sceneIndex)
	{
		case eTRANSLUCENT_SCENE_TWO_QUADS:
		{
			return (char*)("Quads");
		}
		case eTRANSLUCENT_SCENE_MULTIPLE_QUADS:
		{
			return (char*)("MultipleQuads");
		}
		case eTRANSLUCENT_SCENE_BUNNIES:
		{
			return (char*)("Bunnies");
		}
		case eTRANSLUCENT_SCENE_TEAPOTS:
		{
			return (char*)("Teapots");
		}
		case eTRANSLUCENT_SCENE_FRIDGE:
		{
			return (char*)("Fridge");
		}
		case eTRANSLUCENT_SCENE_CAR:
		{
			return (char*)("Car");
		}
		case eTRANSLUCENT_SCENE_INTERSECTING_MESHES:
		{
			return (char*)("IntersectingMeshes");
		}
		case eTRANSLUCENT_SCENE_FOG:
		{
			return (char*)("Fog");
		}
		default:
		{
			ERROR_AND_DIE("Please provide a valid translucent scene");
		}
	}
}


//--------------------------------------------------------------------------------------------------
char* Game::GetControlledQuadrantAsText() const
{
	switch (m_quadrantInControl)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			return (char*)("Quadrant1");
			break;
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			return (char*)("Quadrant2");
			break;
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			return (char*)("Quadrant3");
			break;
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			return (char*)("Quadrant4");
			break;
		}
		default:
		{
			ERROR_AND_DIE("Please find a valid quadrant to conrtol");
		}
			break;
	}
}


//--------------------------------------------------------------------------------------------------
std::string Game::GetGameModeAndGroupModeAsText(eSplitScreenQuadrant quadrant /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	std::string gameModeAndGroupModeAsText = "";

	switch (m_gameMode)
	{
		case eGAME_MODE_DEPTH_TEST:
		{
			gameModeAndGroupModeAsText += "Game Mode: Depth Test, ";
			gameModeAndGroupModeAsText += GetDepthTestSubGroupModeAsText(quadrant);
			break;
		}
		case eGAME_MODE_ALPHA_BLENDING:
		{
			gameModeAndGroupModeAsText += "Game Mode: Alpha Blending, ";
			gameModeAndGroupModeAsText += GetAlphaBlendingSubGroupModeAsText(quadrant);
			break;
		}
		case eGAME_MODE_OIT:
		{
			gameModeAndGroupModeAsText += "Game Mode: OIT, ";
			gameModeAndGroupModeAsText += GetOITSubGroupModeAsText(quadrant);
			break;
		}
		case eGAME_MODE_UAV_WRITES:
		{
			gameModeAndGroupModeAsText += "Game Mode: UAV Writes, ";
			gameModeAndGroupModeAsText += GetUAVWritesSubGroupModeAsText(quadrant);
			break;
		}
		default:
		{
			ERROR_AND_DIE("Please provide a valid game mode");
		}
	}

	return gameModeAndGroupModeAsText;
}


//--------------------------------------------------------------------------------------------------
char* Game::GetDepthTestSubGroupModeAsText(eSplitScreenQuadrant quadrant /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	switch (quadrant)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			return GetDepthTestSubGroupModeAsText(m_quadrant1SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			return GetDepthTestSubGroupModeAsText(m_quadrant2SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			return GetDepthTestSubGroupModeAsText(m_quadrant3SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			return GetDepthTestSubGroupModeAsText(m_quadrant4SubgroupMode);
		}
		default:
		{
			ERROR_AND_DIE("Invalid Quadrant: Please provide a valid quadrant to fetch depth test subgroup game mode as text");
		}
	}
}


//--------------------------------------------------------------------------------------------------
char* Game::GetDepthTestSubGroupModeAsText(unsigned char subGroupMode /*= 0*/) const
{
	switch (eDepthTestMode(subGroupMode))
	{
		case eDEPTH_TEST_MODE_LESS:
		{
			return (char*)("Sub Group Mode: Depth Comparison Less Than Equal ");
		}
		case eDEPTH_TEST_MODE_GREATER:
		{
			return (char*)("Sub Group Mode: Depth Comparison Greater Than Equal ");
		}
		case eDEPTH_TEST_MODE_DISABLED:
		{
			return (char*)("Sub Group Mode: Depth Test Disabled");
		}
		case eDEPTH_TEST_MODE_EARLY_DEPTH_TEST:
		{
			return (char*)("Sub Group Mode: Early Depth Test");
		}
		default:
		{
			ERROR_AND_DIE("Invalid Depth test subgroup mode")
		}
	}
}


//--------------------------------------------------------------------------------------------------
char* Game::GetAlphaBlendingSubGroupModeAsText(eSplitScreenQuadrant quadrant /*= eSPLIT_SCREEN_QUADRANT_1*/) const
{
	switch (quadrant)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			return GetAlphaBlendingSubGroupModeAsText(m_quadrant1SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			return GetAlphaBlendingSubGroupModeAsText(m_quadrant2SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			return GetAlphaBlendingSubGroupModeAsText(m_quadrant3SubgroupMode);
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			return GetAlphaBlendingSubGroupModeAsText(m_quadrant4SubgroupMode);
		}
		default:
		{
			ERROR_AND_DIE("Invalid Quadrant: Please provide a valid quadrant to fetch alpha blending subgroup game mode as text");
		}
	}
}


//--------------------------------------------------------------------------------------------------
char* Game::GetAlphaBlendingSubGroupModeAsText(unsigned char subGroupMode /*= 0*/) const
{
	switch (eAlphaBlendingMode(subGroupMode))
	{
		case eALPHA_BLENDING_MODE_OVER:
		{
			return (char*)("Sub Group Mode: Over Operator ");
		}
		case eALPHA_BLENDING_MODE_UNDER:
		{
			return (char*)("Sub Group Mode: Under Operator ");
		}
		case eALPHA_BLENDING_MODE_PREMULTIPLIED_OVER:
		{
			return (char*)("Sub Group Mode: Premultiplied Over ");
		}
		case eALPHA_BLENDING_MODE_PREMULTIPLIED_UNDER:
		{
			return (char*)("Sub Group Mode: Premultiplied Under ");
		}
		default:
		{
			ERROR_AND_DIE("Invalid Alpha Blending subgroup mode")
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::GoToPreviousValidMode(eSplitScreenQuadrant quadrantInControl, unsigned char subgroupMaxElement)
{
	switch (quadrantInControl)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			GoToPreviousValidMode(m_quadrant1SubgroupMode, subgroupMaxElement);
			break;
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			GoToPreviousValidMode(m_quadrant2SubgroupMode, subgroupMaxElement);
			break;
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			GoToPreviousValidMode(m_quadrant3SubgroupMode, subgroupMaxElement);
			break;
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			GoToPreviousValidMode(m_quadrant4SubgroupMode, subgroupMaxElement);
			break;
		}
		default:
		{
			ERROR_AND_DIE("Invalid Quadrant: Ensure quadrant is between 1-4");
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::GoToNextValidMode(eSplitScreenQuadrant quadrantInControl, unsigned char subgroupMaxElement)
{
	switch (quadrantInControl)
	{
		case eSPLIT_SCREEN_QUADRANT_1:
		{
			GoToNextValidMode(m_quadrant1SubgroupMode, subgroupMaxElement);
			break;
		}
		case eSPLIT_SCREEN_QUADRANT_2:
		{
			GoToNextValidMode(m_quadrant2SubgroupMode, subgroupMaxElement);
			break;
		}
		case eSPLIT_SCREEN_QUADRANT_3:
		{
			GoToNextValidMode(m_quadrant3SubgroupMode, subgroupMaxElement);
			break;
		}
		case eSPLIT_SCREEN_QUADRANT_4:
		{
			GoToNextValidMode(m_quadrant4SubgroupMode, subgroupMaxElement);
			break;
		}
		default:
		{
			ERROR_AND_DIE("Invalid Quadrant: Ensure quadrant is between 1-4");
		}
	}
}


//--------------------------------------------------------------------------------------------------
void Game::GoToPreviousValidMode(unsigned char& quadrantToUpdate, unsigned char subgroupMaxElement)
{
	quadrantToUpdate = quadrantToUpdate - 1;
	if (quadrantToUpdate == unsigned char(-1))
	{
		quadrantToUpdate = subgroupMaxElement - 1;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::GoToNextValidMode(unsigned char& quadrantToUpdate, unsigned char subgroupMaxElement)
{
	quadrantToUpdate = quadrantToUpdate + 1;
	if (quadrantToUpdate >= subgroupMaxElement)
	{
		quadrantToUpdate = 0;
	}
}


//--------------------------------------------------------------------------------------------------
void Game::GoToPreviousTranslucentMode(eTranslucentMode& translucentMode)
{
	translucentMode = eTranslucentMode(translucentMode - 1);
	if (translucentMode == unsigned char(-1))
	{
		translucentMode = eTranslucentMode(eTRANSLUCENT_MODE_COUNT - 1);
	}
}


//--------------------------------------------------------------------------------------------------
void Game::GoToNextTranslucentMode(eTranslucentMode& translucentMode)
{
	translucentMode = eTranslucentMode(translucentMode + 1);
	if (translucentMode == eTRANSLUCENT_MODE_COUNT)
	{
		translucentMode = eTranslucentMode(0);
	}
}


//--------------------------------------------------------------------------------------------------
void Game::CreateMeshOfType(SceneObject*& out_createdMesh, eMeshType meshTypeToCreate)
{
	// Populate vertex and index array
	std::vector<Vertex_PCU>		tempVerts;
	std::vector<unsigned int>	tempIndexes;
	switch (meshTypeToCreate)
	{
		case eMESH_TYPE_BILLBOARDED_QUAD:
		case eMESH_TYPE_QUAD:
		{
			tempVerts.reserve(4);
			tempIndexes.reserve(6);
			//AddVertsForQuad3D(tempVerts, tempIndexes, Vec3(0.f, 0.5f, -0.5f), Vec3(0.f, -0.5f, -0.5f), Vec3(0.f, -0.5f, 0.5f), Vec3(0.f, 0.5f, 0.5f));
			AddVertsForQuad3D(tempVerts, tempIndexes, Vec3(0.5f, -0.5f, 0.f), Vec3(-0.5f, -0.5f, 0.f), Vec3(-0.5f, 0.5f, 0.f), Vec3(0.5f, 0.5f, 0.f));
			break;
		}
		case eMESH_TYPE_UNI_TEXTURED_OBB:
		{
			tempVerts.reserve(24);
			tempIndexes.reserve(36);
			AddVertsForOBB3D(tempVerts, tempIndexes, OBB3(Vec3::ZERO, Vec3::X_AXIS, Vec3::Y_AXIS, Vec3::Z_AXIS, Vec3(0.5f, 0.5f, 0.5f)));
			break;
		}
		case eMESH_TYPE_MULTI_TEXTURED_OBB:
		{

			break;
		}
		case eMESH_TYPE_SPHERE:
		{
			tempVerts.reserve(242);
			tempIndexes.reserve(1440);
			AddVertsForSphere3D(tempVerts, tempIndexes, Vec3::ZERO, 1.f, 16.f, 16.f);
			break;
		}
		case eMESH_TYPE_TEPOT:
		{
			Mat44 transformFixupMat;
			transformFixupMat.SetIJKT3D(Vec3::X_AXIS, Vec3::Z_AXIS, -Vec3::Y_AXIS, Vec3::ZERO);
			OBJLoader::LoadOBJFileByName("Data/Models/Teapot.obj", tempVerts, tempIndexes, transformFixupMat);
			break;
		}
		case eMESH_TYPE_BUNNY:
		{
			Mat44 transformFixupMat;
			transformFixupMat.SetIJKT3D(-Vec3::X_AXIS, Vec3::Z_AXIS, Vec3::Y_AXIS, Vec3::ZERO);
			OBJLoader::LoadOBJFileByName("Data/Models/StanfordBunny.obj", tempVerts, tempIndexes, transformFixupMat);
			break;
		}
		case eMESH_TYPE_FRIDGE_DOOR:
		{
			Mat44 transformFixupMat;
			transformFixupMat.SetIJKT3D(Vec3::Y_AXIS, Vec3::Z_AXIS, Vec3::X_AXIS, Vec3::ZERO);
			transformFixupMat.SetTranslation3D(Vec3(8.f, 0.f, 0.f));
			OBJLoader::LoadOBJFileByName("Data/Models/FridgeDoor.obj", tempVerts, tempIndexes, transformFixupMat);
			break;
		}
		case eMESH_TYPE_FRIDGE_BODY:
		{
			Mat44 transformFixupMat;
			transformFixupMat.SetIJKT3D(Vec3::Y_AXIS, Vec3::Z_AXIS, Vec3::X_AXIS, Vec3::ZERO);
			transformFixupMat.SetTranslation3D(Vec3(8.f, 0.f, 0.f));
			OBJLoader::LoadOBJFileByName("Data/Models/FridgeBody.obj", tempVerts, tempIndexes, transformFixupMat);
			break;
		}
		case eMESH_TYPE_CAR_MAIN_BODY:
		{
			// Mat44 transformFixupMat;
			// transformFixupMat.SetIJKT3D(Vec3::X_AXIS, Vec3::Y_AXIS, Vec3::Z_AXIS, Vec3::ZERO);
			// transformFixupMat.AppendScaleUniform3D(meshHalfDims.x);
			// OBJLoader::LoadOBJFileByName("Data/Models/CarBody.obj", tempVerts, tempIndexes, transformFixupMat);
			// currentMesh.m_debugName = L"Car Body Render";
			break;
		}
		case eMESH_TYPE_CAR_GLASS_1:
		{
			// Mat44 transformFixupMat;
			// transformFixupMat.SetIJKT3D(Vec3::X_AXIS, Vec3::Y_AXIS, Vec3::Z_AXIS, Vec3::ZERO);
			// transformFixupMat.AppendScaleUniform3D(meshHalfDims.x);
			// OBJLoader::LoadOBJFileByName("Data/Models/CarGlass1.obj", tempVerts, tempIndexes, transformFixupMat);
			// currentMesh.m_debugName = L"Car Glass 1 Render";
			break;
		}
		case eMESH_TYPE_CAR_GLASS_2:
		{
			// Mat44 transformFixupMat;
			// transformFixupMat.SetIJKT3D(Vec3::X_AXIS, Vec3::Y_AXIS, Vec3::Z_AXIS, Vec3::ZERO);
			// transformFixupMat.AppendScaleUniform3D(meshHalfDims.x);
			// OBJLoader::LoadOBJFileByName("Data/Models/CarGlass2.obj", tempVerts, tempIndexes, transformFixupMat);
			// currentMesh.m_debugName = L"Car Glass 2 Render";
			break;
		}
		default:
		{
			ERROR_AND_DIE("Mesh creation error: Invalid mesh type");
		}
	}

	// Create/initialize vertex and index buffers using the vertex and index array
	if (tempVerts.size() == 0 || tempIndexes.size() == 0)
	{
		return;
	}

	out_createdMesh = new SceneObject();
	out_createdMesh->m_numOfIndexes		=	(unsigned int)tempIndexes.size();
	out_createdMesh->m_vertexBuffer		=	g_theRenderer->CreateVertexBuffer((unsigned int)tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
	out_createdMesh->m_indexBuffer		=	g_theRenderer->CreateIndexBuffer(out_createdMesh->m_numOfIndexes, ResourceUsage::GPU_READ, tempIndexes.data());
}


//--------------------------------------------------------------------------------------------------
bool Game::Command_SetDepthPeelCount(EventArgs& args)
{
	g_theGame->m_numOfDepthPeelingPasses = (unsigned int)args.GetValue("Count", 2);
	return true;
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeSceneFromElement(XmlElement const& sceneDef)
{
	std::string sceneName = ParseXmlAttribute(sceneDef, "name", "INVALID");
	GUARANTEE_OR_DIE(sceneName != "INVALID", "Invalid scene Name");
	eTranslucentScene currentSceneAsEnum	=	GetSceneFromName(sceneName);
	XmlElement const* meshInfoElement		=	sceneDef.FirstChildElement("MeshInfo");
	// unsigned int	  numOfMeshesInScene	=	ParseXmlAttribute(sceneDef, "numOfMeshes", 8);
	Scene*&			  currentScene			=	m_scenes[currentSceneAsEnum];
	if (!currentScene)
	{
		currentScene = new Scene;
	}

	while (meshInfoElement)
	{
		std::vector<Vertex_PCU> tempVerts;
		std::vector<unsigned int> tempIndexes;
		SceneObject currentMesh;
		std::string meshTypeAsString		=	ParseXmlAttribute(*meshInfoElement, "type",				  "Quad");
		currentMesh.m_color					=	ParseXmlAttribute(*meshInfoElement,	"color",			  Rgba8::WHITE);
		Vec3		meshCenter				=	ParseXmlAttribute(*meshInfoElement, "center",			  Vec3::ZERO);
		Vec3		meshDimension			=	ParseXmlAttribute(*meshInfoElement, "dimension",		  Vec3(1.f, 1.f, 1.f));
		EulerAngles meshOrientation			=	ParseXmlAttribute(*meshInfoElement, "orientation",		  EulerAngles());
		currentMesh.m_numOfTexturesBound	=	ParseXmlAttribute(*meshInfoElement, "numOfTexturesBound", 1);
		eMeshType	meshType				=	GetMeshTypeFromText(meshTypeAsString);
		currentMesh.m_type					=	meshType;
		currentMesh.m_center				=	meshCenter;
		bool		isOpaque				=	currentMesh.m_color.a == 255;
		Vec3 meshiForward;
		Vec3 meshjLeft;
		Vec3 meshkUp;
		meshOrientation.GetAsVectors_XFwd_YLeft_ZUp(meshiForward, meshjLeft, meshkUp);
		Vec3 meshHalfDims = meshDimension * 0.5f;
		switch (meshType)
		{
			case eMESH_TYPE_BILLBOARDED_QUAD:
			{
				currentMesh.m_billboardedQuadHalfDims = Vec2(meshHalfDims.x, meshHalfDims.y);
				tempVerts.reserve(4);
				tempIndexes.reserve(6);

				Vec3 westSouthBottom = Vec3::ZERO + (Vec3(0.f, 1.f, 0.f)) - (Vec3(0.f, 0.f, 1.f));
				Vec3 eastSouthBottom = Vec3::ZERO - (Vec3(0.f, 1.f, 0.f)) - (Vec3(0.f, 0.f, 1.f));
				Vec3 eastNorthBottom = Vec3::ZERO - (Vec3(0.f, 1.f, 0.f)) + (Vec3(0.f, 0.f, 1.f));
				Vec3 westNorthBottom = Vec3::ZERO + (Vec3(0.f, 1.f, 0.f)) + (Vec3(0.f, 0.f, 1.f));
				AddVertsForQuad3D(tempVerts, tempIndexes, westSouthBottom, eastSouthBottom, eastNorthBottom, westNorthBottom);
				currentMesh.m_debugName = L"Billboarded Quad Render";
				break;
			}
			case eMESH_TYPE_QUAD:
			{
				tempVerts.reserve(4);
				tempIndexes.reserve(6);
				
				Vec3 westSouthBottom = meshCenter + (meshHalfDims.y * meshjLeft) - (meshHalfDims.x * meshiForward);
				Vec3 eastSouthBottom = meshCenter - (meshHalfDims.y * meshjLeft) - (meshHalfDims.x * meshiForward);
				Vec3 eastNorthBottom = meshCenter - (meshHalfDims.y * meshjLeft) + (meshHalfDims.x * meshiForward);
				Vec3 westNorthBottom = meshCenter + (meshHalfDims.y * meshjLeft) + (meshHalfDims.x * meshiForward);
				AddVertsForQuad3D(tempVerts, tempIndexes, westSouthBottom, eastSouthBottom, eastNorthBottom, westNorthBottom);
				currentMesh.m_debugName = L"Quad Render";
				break;
			}
			case eMESH_TYPE_UNI_TEXTURED_OBB:
			{
				tempVerts.reserve(24);
				tempIndexes.reserve(36);

				OBB3 obbMesh(meshCenter, meshiForward, meshjLeft, meshkUp, meshHalfDims);
				AddVertsForOBB3D(tempVerts, tempIndexes, obbMesh);
				currentMesh.m_debugName = L"OBB Render";
				break;
			}
			case eMESH_TYPE_MULTI_TEXTURED_OBB:
			{
				ERROR_AND_DIE("Not supported yet");
				break;
			}
			case eMESH_TYPE_SPHERE:
			{
				tempVerts.reserve(242);
				tempIndexes.reserve(1440);
				AddVertsForSphere3D(tempVerts, tempIndexes, meshCenter, meshDimension.x, 64.f, 32.f);
				currentMesh.m_debugName = L"Sphere Render";
				break;
			}
			case eMESH_TYPE_TEPOT:
			{
				Mat44 transformFixupMat;
				transformFixupMat.SetIJKT3D(meshiForward, meshkUp, -meshjLeft, meshCenter);
				transformFixupMat.AppendScaleUniform3D(meshHalfDims.x);
				OBJLoader::LoadOBJFileByName("Data/Models/Teapot.obj", tempVerts, tempIndexes, transformFixupMat);
				currentMesh.m_debugName = L"Teapot Render";
				break;
			}
			case eMESH_TYPE_BUNNY:
			{
				Mat44 transformFixupMat;
				transformFixupMat.SetIJKT3D(-meshiForward, meshkUp, meshjLeft, meshCenter);
				transformFixupMat.AppendScaleUniform3D(meshHalfDims.x);
				OBJLoader::LoadOBJFileByName("Data/Models/StanfordBunny.obj", tempVerts, tempIndexes, transformFixupMat);
				currentMesh.m_debugName = L"Bunny Render";
				break;
			}
			case eMESH_TYPE_FRIDGE_DOOR:
			{
				Mat44 transformFixupMat;
				transformFixupMat.SetIJKT3D(meshjLeft, meshkUp, meshiForward, meshCenter);
				transformFixupMat.AppendScaleUniform3D(meshHalfDims.x);
				OBJLoader::LoadOBJFileByName("Data/Models/FridgeDoor.obj", tempVerts, tempIndexes, transformFixupMat);
				currentMesh.m_debugName = L"Fridge Door Render";
				break;
			}
			case eMESH_TYPE_FRIDGE_BODY:
			{
				Mat44 transformFixupMat;
				transformFixupMat.SetIJKT3D(meshjLeft, meshkUp, meshiForward, meshCenter);
				transformFixupMat.AppendScaleUniform3D(meshHalfDims.x);
				OBJLoader::LoadOBJFileByName("Data/Models/FridgeBody.obj", tempVerts, tempIndexes, transformFixupMat);
				currentMesh.m_debugName = L"Fridge Body Render";
				break;
			}
			case eMESH_TYPE_CAR_MAIN_BODY:
			{
				Mat44 transformFixupMat;
				transformFixupMat.SetIJKT3D(meshjLeft, meshkUp, meshiForward, meshCenter);
				transformFixupMat.AppendScaleUniform3D(meshHalfDims.x);
				OBJLoader::LoadOBJFileByName("Data/Models/CarBody.obj", tempVerts, tempIndexes, transformFixupMat);
				currentMesh.m_debugName = L"Car Body Render";
				break;
			}
			case eMESH_TYPE_CAR_GLASS_1:
			{
				Mat44 transformFixupMat;
				transformFixupMat.SetIJKT3D(meshjLeft, meshkUp, meshiForward, meshCenter);
				transformFixupMat.AppendScaleUniform3D(meshHalfDims.x);
				OBJLoader::LoadOBJFileByName("Data/Models/CarGlass1.obj", tempVerts, tempIndexes, transformFixupMat);
				currentMesh.m_debugName = L"Car Glass 1 Render";
				break;
			}
			case eMESH_TYPE_CAR_GLASS_2:
			{
				Mat44 transformFixupMat;
				transformFixupMat.SetIJKT3D(meshjLeft, meshkUp, meshiForward, meshCenter);
				transformFixupMat.AppendScaleUniform3D(meshHalfDims.x);
				OBJLoader::LoadOBJFileByName("Data/Models/CarGlass2.obj", tempVerts, tempIndexes, transformFixupMat);
				currentMesh.m_debugName = L"Car Glass 2 Render";
				break;
			}
			default:
			{
				ERROR_AND_DIE("Please provide a valid mesh type");
			}
		}
	
		XmlElement const* textureResourceElement	=	meshInfoElement->FirstChildElement("TextureInfo");
		if (textureResourceElement)
		{
			std::string textureResourceFilePath = ParseXmlAttribute(*textureResourceElement, "texture", "NULL");
			if (textureResourceFilePath != "NULL")
			{
				std::string textureResourceDebugName		=	ParseXmlAttribute(*textureResourceElement, "debugTextureName", "INVALID");
				GUARANTEE_OR_DIE(textureResourceDebugName	!= "INVALID", "Please provide a debug name for texture resource");
				currentMesh.m_textureResource			=	g_theRenderer->CreateTextureResourceFromFile(textureResourceFilePath.c_str(), textureResourceDebugName.c_str(), (unsigned int)textureResourceDebugName.size());
			}
		}
		
		// Scene*& currentSceneObjects = !isOpaque ? currentScene->m_translucentObjects : currentScene->m_opaqueObjects;
		// if (!currentSceneObjects)
		// {
		// 	currentSceneObjects = new Scene;
		// }

		unsigned int currentSceneIndexeSize = (unsigned int)currentScene->m_sceneVertices.size();
		for (unsigned int currentIndex = 0; currentIndex < tempVerts.size(); ++currentIndex)
		{
			Vertex_PCU currentVertex = tempVerts[currentIndex];
			currentVertex.m_color = currentMesh.m_color;
			currentScene->m_sceneVertices.emplace_back(currentVertex);
		}
		// currentSceneObjects->m_sceneVertices.insert(currentSceneObjects->m_sceneVertices.end(), tempVerts.begin(), tempVerts.end());
		currentMesh.m_vertexBuffer	=	g_theRenderer->CreateVertexBuffer(tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
		std::string debugMeshVBName =	sceneName + "_" + meshTypeAsString + "_VertexBuffer";
		g_theRenderer->SetDebugResourceName(currentMesh.m_vertexBuffer, (unsigned int)debugMeshVBName.size(), debugMeshVBName.c_str());
		
		
		currentMesh.m_numOfIndexes			=	(unsigned int)tempIndexes.size();
		for (unsigned int index = 0; index < currentMesh.m_numOfIndexes; ++index)
		{
			unsigned int currentIndex = currentSceneIndexeSize + tempIndexes[index];
			currentScene->m_sceneIndexes.emplace_back(currentIndex);
		}
		currentMesh.m_indexBuffer	=	g_theRenderer->CreateIndexBuffer(currentMesh.m_numOfIndexes, ResourceUsage::GPU_READ, tempIndexes.data());
		std::string debugMeshIBName =	sceneName + "_" + meshTypeAsString + "_IndexBuffer";
		g_theRenderer->SetDebugResourceName(currentMesh.m_indexBuffer, (unsigned int)debugMeshIBName.size(), debugMeshIBName.c_str());
		
		std::vector<SceneObject>& currentSceneObjects = !isOpaque ? currentScene->m_translucentObjects : currentScene->m_opaqueObjects;
		currentSceneObjects.emplace_back(currentMesh);
		if (!isOpaque)
		{
			currentScene->m_sortedTranslucentObjects.emplace_back(currentMesh);
		}
		meshInfoElement	= meshInfoElement->NextSiblingElement("MeshInfo");
	}
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeDepthTestModeSceneObjects()
{

}


//--------------------------------------------------------------------------------------------------
void Game::InitializeVertexAndIndexBuffers()
{
	AABB2 fullScreenQuad = m_screenCamera.GetCameraAABB();
	std::vector<Vertex_PCU> tempVerts;
	std::vector<unsigned int> tempIndexes;
	AddVertsForAABB2D(tempVerts, tempIndexes, fullScreenQuad, Rgba8::WHITE, AABB2(0.f, 1.f, 1.f, 0.f));
	m_fullScreenQuadVB = g_theRenderer->CreateVertexBuffer(tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
	m_fullScreenQuadIB = g_theRenderer->CreateIndexBuffer(tempIndexes.size(), ResourceUsage::GPU_READ, tempIndexes.data());

	float verticalPadding			=	0.0125f;
	float verticalHalfPadding		=	verticalPadding * 0.5f;
	float horizontalPadding			=	verticalHalfPadding;
	float horizontalHalfPadding		=	horizontalPadding * 0.5f;

	// Quadrant 1
	Vec2 Q1BL(horizontalPadding,				0.5f + verticalHalfPadding);
	Vec2 Q1BR(0.5f - horizontalHalfPadding,		0.5f + verticalHalfPadding);
	Vec2 Q1TR(0.5f - horizontalHalfPadding,		1.f - verticalPadding);
	Vec2 Q1TL(horizontalPadding,				1.f - verticalPadding);

	// Quadrant 2
	Vec2 Q2BL(0.5f + horizontalHalfPadding,		0.5f + verticalHalfPadding);
	Vec2 Q2BR(1.f - horizontalPadding,			0.5f + verticalHalfPadding);
	Vec2 Q2TR(1.f - horizontalPadding,			1.f - verticalPadding);
	Vec2 Q2TL(0.5f + horizontalHalfPadding,		1.f - verticalPadding);

	// Quadrant 3
	Vec2 Q3BL(horizontalPadding,				verticalPadding);
	Vec2 Q3BR(0.5f - horizontalHalfPadding,		verticalPadding);
	Vec2 Q3TR(0.5f - horizontalHalfPadding,		0.5f - verticalHalfPadding);
	Vec2 Q3TL(horizontalPadding,				0.5f - verticalHalfPadding);

	// Quadrant 4
	Vec2 Q4BL(0.5f + horizontalHalfPadding,		verticalPadding);
	Vec2 Q4BR(1.f - horizontalPadding,			verticalPadding);
	Vec2 Q4TR(1.f - horizontalPadding,			0.5f - verticalHalfPadding);
	Vec2 Q4TL(0.5f + horizontalHalfPadding,		0.5f - verticalHalfPadding);

	// Vec2 bottomLeft(leftHorizontalPadding,						bottomVerticalPadding);
	// Vec2 bottomLeftMid(0.5f - (leftHorizontalPadding * 0.5f),	bottomVerticalPadding);
	// Vec2 bottomRightMid(0.5f + (leftHorizontalPadding * 0.5f),	bottomVerticalPadding);
	// Vec2 bottomRight(rightHorizontalPadding,					bottomVerticalPadding);
	// Vec2 midRightBottom(rightHorizontalPadding,					0.5f - (bottomVerticalPadding * 0.5f));
	// Vec2 midRightTop(rightHorizontalPadding,					0.5f + (bottomVerticalPadding * 0.5f));
	// Vec2 topRight(rightHorizontalPadding,						1.f - bottomVerticalPadding);
	// Vec2 topRightMid(0.5f + (leftHorizontalPadding * 0.5f),		1.f - bottomVerticalPadding);
	// Vec2 topLeftMid(0.5f - (leftHorizontalPadding * 0.5f),		1.f - bottomVerticalPadding);
	// Vec2 topLeft(leftHorizontalPadding,							1.f - bottomVerticalPadding);
	// Vec2 midLeft();


	tempVerts.clear();
	tempIndexes.clear();
	// AABB2 quadrant1Quad = fullScreenQuad.GetBoxAtUVs(0.f, 0.5f, 0.5f, 1.f);
	AABB2 quadrant1Quad = fullScreenQuad.GetBoxAtUVs(Q1BL.x, Q1BL.y, Q1TR.x, Q1TR.y);
	AddVertsForAABB2D(tempVerts, tempIndexes, quadrant1Quad, Rgba8::WHITE, AABB2(0.f, 1.f, 1.f, 0.f));
	m_quadrant1QuadVB = g_theRenderer->CreateVertexBuffer(tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
	m_quadrant1QuadIB = g_theRenderer->CreateIndexBuffer(tempIndexes.size(), ResourceUsage::GPU_READ, tempIndexes.data());

	tempVerts.clear();
	tempIndexes.clear();
	// AABB2 quadrant2Quad = fullScreenQuad.GetBoxAtUVs(0.5f, 0.5f, 1.f, 1.f);
	AABB2 quadrant2Quad = fullScreenQuad.GetBoxAtUVs(Q2BL.x, Q2BL.y, Q2TR.x, Q2TR.y);
	AddVertsForAABB2D(tempVerts, tempIndexes, quadrant2Quad, Rgba8::WHITE, AABB2(0.f, 1.f, 1.f, 0.f));
	m_quadrant2QuadVB = g_theRenderer->CreateVertexBuffer(tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
	m_quadrant2QuadIB = g_theRenderer->CreateIndexBuffer(tempIndexes.size(), ResourceUsage::GPU_READ, tempIndexes.data());
	
	tempVerts.clear();
	tempIndexes.clear();
	// AABB2 quadrant3Quad = fullScreenQuad.GetBoxAtUVs(0.f, 0.f, 0.5f, 0.5f);
	AABB2 quadrant3Quad = fullScreenQuad.GetBoxAtUVs(Q3BL.x, Q3BL.y, Q3TR.x, Q3TR.y);
	AddVertsForAABB2D(tempVerts, tempIndexes, quadrant3Quad, Rgba8::WHITE, AABB2(0.f, 1.f, 1.f, 0.f));
	m_quadrant3QuadVB = g_theRenderer->CreateVertexBuffer(tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
	m_quadrant3QuadIB = g_theRenderer->CreateIndexBuffer(tempIndexes.size(), ResourceUsage::GPU_READ, tempIndexes.data());
	
	tempVerts.clear();
	tempIndexes.clear();
	// AABB2 quadrant4Quad = fullScreenQuad.GetBoxAtUVs(0.5f, 0.f, 1.f, 0.5f);
	AABB2 quadrant4Quad = fullScreenQuad.GetBoxAtUVs(Q4BL.x, Q4BL.y, Q4TR.x, Q4TR.y);
	AddVertsForAABB2D(tempVerts, tempIndexes, quadrant4Quad, Rgba8::WHITE, AABB2(0.f, 1.f, 1.f, 0.f));
	m_quadrant4QuadVB = g_theRenderer->CreateVertexBuffer(tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
	m_quadrant4QuadIB = g_theRenderer->CreateIndexBuffer(tempIndexes.size(), ResourceUsage::GPU_READ, tempIndexes.data());

	tempVerts.clear();
	tempIndexes.clear();
	AABB2 quadrant1Split2Quad = fullScreenQuad.GetBoxAtUVs(0.25f, 0.5f, 0.75f, 1.f);
	AddVertsForAABB2D(tempVerts, tempIndexes, quadrant1Split2Quad, Rgba8::WHITE, AABB2(0.f, 1.f, 1.f, 0.f));
	m_quadrant1Split2QuadVB = g_theRenderer->CreateVertexBuffer(tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
	m_quadrant1Split2QuadIB = g_theRenderer->CreateIndexBuffer(tempIndexes.size(), ResourceUsage::GPU_READ, tempIndexes.data());

	tempVerts.clear();
	tempIndexes.clear();
	AABB2 quadrant2Split2Quad = fullScreenQuad.GetBoxAtUVs(0.25f, 0.f, 0.75f, 0.5f);
	AddVertsForAABB2D(tempVerts, tempIndexes, quadrant2Split2Quad, Rgba8::WHITE, AABB2(0.f, 1.f, 1.f, 0.f));
	m_quadrant2Split2QuadVB = g_theRenderer->CreateVertexBuffer(tempVerts.size(), sizeof(Vertex_PCU), ResourceUsage::GPU_READ, tempVerts.data());
	m_quadrant2Split2QuadIB = g_theRenderer->CreateIndexBuffer(tempIndexes.size(), ResourceUsage::GPU_READ, tempIndexes.data());
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeScenesFromDefinition()
{
	XmlDocument sceneDefsXML;
	char const* sceneDefFilePath	=	"Data/Definitions/SceneDefinition.xml";
	XmlError actorResult			=	sceneDefsXML.LoadFile(sceneDefFilePath);
	GUARANTEE_OR_DIE(actorResult == tinyxml2::XML_SUCCESS, Stringf("Failed to open required scene defs file \" % s\"", sceneDefFilePath));

	XmlElement* rootElement = sceneDefsXML.RootElement();
	GUARANTEE_OR_DIE(rootElement, Stringf("Failed to fetch required root element"));

	XmlElement* sceneDefElement = rootElement->FirstChildElement();

	while (sceneDefElement)
	{
		InitializeSceneFromElement(*sceneDefElement);
		sceneDefElement	= sceneDefElement->NextSiblingElement();
	}
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeCustomBlendModes()
{
	BlendMode weightedBlendedBMs[2] = { BlendMode::ACCUMULATION, BlendMode::REVEALAGE };
	g_theRenderer->CreateBlendModes(weightedBlendedBMs, 2);
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeGridBuffer()
{
	std::vector<Vertex_PCU> gridCPUVerts;
	constexpr int VERTS_PER_AABB3		= 36;
	constexpr int NUM_OF_AABB3			= 200;
	constexpr int NUM_OF_CPU_GRID_VERTS = VERTS_PER_AABB3 * NUM_OF_AABB3;
	gridCPUVerts.reserve(NUM_OF_CPU_GRID_VERTS);

	for (int xOffset = -50; xOffset < 50; ++xOffset)
	{
		AABB3 bounds = AABB3(Vec3(-50.f, -0.01f, -0.01f), Vec3(50.f, 0.01f, 0.01f));
		Rgba8 color = Rgba8(100, 100, 100);
		if (xOffset == 0)
		{
			bounds = AABB3(Vec3(-50.f, -0.1f, -0.1f), Vec3(50.f, 0.1f, 0.1f));
			color = Rgba8::RED;
		}
		else if (xOffset % 5 == 0)
		{
			bounds = AABB3(Vec3(-50.f, -0.05f, -0.05f), Vec3(50.f, 0.05f, 0.05f));
			color = Rgba8(150, 20, 20);
		}
		bounds.SetCenter(Vec3(0.f, (float)xOffset));

		AddVertsForAABB3D(gridCPUVerts, bounds, color);
	}

	for (int yOffset = -50; yOffset < 50; ++yOffset)
	{
		AABB3 bounds = AABB3(Vec3(-0.01f, -50.f, -0.01f), Vec3(0.01f, 50.f, 0.01f));
		Rgba8 color = Rgba8(100, 100, 100);
		if (yOffset == 0)
		{
			bounds = AABB3(Vec3(-0.1f, -50.f, -0.1f), Vec3(0.1f, 50.f, 0.1f));
			color = Rgba8::GREEN;

		}
		else if (yOffset % 5 == 0)
		{
			bounds = AABB3(Vec3(-0.05f, -50.f, -0.05f), Vec3(0.05f, 50.f, 0.05f));
			color = Rgba8(20, 150, 20);
		}
		bounds.SetCenter(Vec3((float)yOffset, 0.f));

		AddVertsForAABB3D(gridCPUVerts, bounds, color);
	}

	m_gridVertexBuffer = g_theRenderer->CreateVertexBuffer(gridCPUVerts.size(), sizeof(Vertex_PCU));
	g_theRenderer->CopyCPUToGPU(gridCPUVerts.data(), (size_t)m_gridVertexBuffer->GetStride() * gridCPUVerts.size(), m_gridVertexBuffer);
}


//--------------------------------------------------------------------------------------------------
struct UnoptimizedFragment
{
	Vec4	m_premultipliedColor = Vec4(0.f, 0.f, 0.f, 0.f);
	float	m_transmission = FLT_MAX;
	float	m_depth = FLT_MAX;
};


//--------------------------------------------------------------------------------------------------
static int const fragmentsPerPixel_2 = 2;


//--------------------------------------------------------------------------------------------------
struct UnoptimizedFragmentArray_2
{
	UnoptimizedFragment frags[fragmentsPerPixel_2] = { };
};


//--------------------------------------------------------------------------------------------------
static int const fragmentsPerPixel_32 = 32;


//--------------------------------------------------------------------------------------------------
struct UnoptimizedFragmentArray_32
{
	UnoptimizedFragment frags[fragmentsPerPixel_32] = { };
};


//--------------------------------------------------------------------------------------------------
static int const fragmentsPerPixel_4 = 4;


//--------------------------------------------------------------------------------------------------
struct UnoptimizedFragmentArray_4
{
	UnoptimizedFragment frags[fragmentsPerPixel_4] = { };
};


//--------------------------------------------------------------------------------------------------
void Game::InitializeResources()
{
	g_theRenderer->CreateRenderTargetResource(m_quadrant1RenderTarget,					true,  "Quadrant 1 Render Target",					sizeof("Quadrant 1 Render Target"));
	g_theRenderer->CreateRenderTargetResource(m_quadrant2RenderTarget,					true,  "Quadrant 2 Render Target",					sizeof("Quadrant 2 Render Target"));
	g_theRenderer->CreateRenderTargetResource(m_quadrant3RenderTarget,					true,  "Quadrant 3 Render Target",					sizeof("Quadrant 3 Render Target"));
	g_theRenderer->CreateRenderTargetResource(m_quadrant4RenderTarget,					true,  "Quadrant 4 Render Target",					sizeof("Quadrant 4 Render Target"));
	g_theRenderer->CreateRenderTargetResource(m_intermediateRenderTarget,				true, "Intermediate Render Target",				sizeof("Intermediate Render Target"));
	g_theRenderer->CreateRenderTargetResource(m_intermediateTarget,						true,  "Intermediate Target",						sizeof("Intermediate Target"));
	g_theRenderer->CreateRenderTargetResource(m_opaqueBackgroundQuadrant1RenderTarget,	false, "Opaque BG Quadrant 1 RT",		sizeof("Opaque BG Quadrant 1 RT"));
	g_theRenderer->CreateRenderTargetResource(m_depthPeelingIntermediateRenderTarget,	true,  "Depth Peeling Intermediate Render Target",	sizeof("Depth Peeling Intermediate Render Target"));

	g_theRenderer->CreateDepthResource(m_quadrant1DepthTarget,					"Quadrant 1 Depth Target",					sizeof("Quadrant 1 Depth Target"),					true);
	g_theRenderer->CreateDepthResource(m_quadrant2DepthTarget,					"Quadrant 2 Depth Target",					sizeof("Quadrant 2 Depth Target"),					true);
	g_theRenderer->CreateDepthResource(m_quadrant3DepthTarget,					"Quadrant 3 Depth Target",					sizeof("Quadrant 3 Depth Target"),					true);
	g_theRenderer->CreateDepthResource(m_quadrant4DepthTarget,					"Quadrant 4 Depth Target",					sizeof("Quadrant 4 Depth Target"),					true);
	g_theRenderer->CreateDepthResource(m_depthPeelingIntermediateDepthTarget,	"Depth Peeling Intermediate Depth Target",	sizeof("Depth Peeling Intermediate Depth Target"),	true);

	IntVec2 windowDim	=	g_theWindow->GetClientDimensions();
	{
		// Buffer resources 
		// Structured Buffer
		int numOfNodes		=	32;										// Make sure to change the number of nodes in the hlsl (compute shader, both clear and other one)as well
		int numOfElements	=	windowDim.x * windowDim.y * numOfNodes;
		std::vector<Fragment> defaultFragData;
		defaultFragData.resize((size_t)numOfElements);
		
		D3D11_ResourceConfig structuredBufferConfig			=	{ };
		structuredBufferConfig.m_numOfElements				=	numOfElements;
		structuredBufferConfig.m_elementStride				=	sizeof(Fragment);
		structuredBufferConfig.m_usageFlag					=	ResourceUsage::GPU_READ_GPU_WRITE;
		structuredBufferConfig.m_defaultInitializationData	=	defaultFragData.data();
		structuredBufferConfig.m_format						=	ResourceViewFormat::DXGI_FORMAT_UNKNOWN;
		structuredBufferConfig.m_isStandard					=	false;
		structuredBufferConfig.m_type						=	ResourceType::STRUCTURED_BUFFER;
		structuredBufferConfig.m_debugName					=	"FragmentsPerPixelStructuredBuffer_32";
		structuredBufferConfig.m_debugNameSize				=	sizeof("FragmentsPerPixelStructuredBuffer_32");
		g_theRenderer->CreateResourceFromConfig(structuredBufferConfig, m_fragmentsPerPixelStructuredBuffer_32);

		// Byte-Address Buffer
		numOfElements = windowDim.x * windowDim.y;
		std::vector<unsigned int> defaultRootLinkedListData;
		defaultRootLinkedListData.resize(numOfElements, 0xFFFFFFFFU);
		// g_theRenderer->CreateRawBuffer(m_byteAddressBufferOfHeadLinkedList, windowDim.x * windowDim.y, ResourceUsage::GPU_READ_GPU_WRITE, defaultRootLinkedListData.data());
		// g_theRenderer->SetDebugResourceName(m_byteAddressBufferOfHeadLinkedList, sizeof("HeadOfFragmentsPerPixelLinkedList"), "HeadOfFragmentsPerPixelLinkedList");

		
		D3D11_ResourceConfig rawBufferConfig			=	{ };
		rawBufferConfig.m_numOfElements					=	numOfElements;
		rawBufferConfig.m_elementStride					=	4;
		rawBufferConfig.m_usageFlag						=	ResourceUsage::GPU_READ_GPU_WRITE;
		rawBufferConfig.m_defaultInitializationData		=	defaultRootLinkedListData.data();
		rawBufferConfig.m_type							=	ResourceType::RAW_BUFFER;
		rawBufferConfig.m_debugName						=	"headNodeByteAddressBuffer_32";
		rawBufferConfig.m_debugNameSize					=	sizeof("headNodeByteAddressBuffer_32");
		g_theRenderer->CreateResourceFromConfig(rawBufferConfig, m_headNodeByteAddressBuffer_32);

		// Constant Buffer
		m_gameConstantBuffer				=	g_theRenderer->CreateConstantBuffer(sizeof(GameConstants));
		GameConstants screenConstants		=	{ };
		screenConstants.m_screenWidth		=	(float)windowDim.x;
		screenConstants.m_uintMax			=	(unsigned int)-1;
		screenConstants.m_cameraFar			=   m_player->m_camera.GetCameraFarClipDist();
		g_theRenderer->CopyCPUToGPU(&screenConstants, sizeof(GameConstants), m_gameConstantBuffer);
		g_theRenderer->BindConstantBuffer(g_gameConstantsSlot, m_gameConstantBuffer, BindingLocation::COMPUTE_SHADER);
	}

	{
		// Buffer resources 
		// Structured Buffer
		int numOfNodes		=	4;										// Make sure to change the number of nodes in the hlsl (compute shader, both clear and other one)as well
		int numOfElements	=	windowDim.x * windowDim.y * numOfNodes;
		std::vector<Fragment> defaultFragData;
		defaultFragData.resize((size_t)numOfElements);
		
		D3D11_ResourceConfig structuredBufferConfig			=	{ };
		structuredBufferConfig.m_numOfElements				=	numOfElements;
		structuredBufferConfig.m_elementStride				=	sizeof(Fragment);
		structuredBufferConfig.m_usageFlag					=	ResourceUsage::GPU_READ_GPU_WRITE;
		structuredBufferConfig.m_defaultInitializationData	=	defaultFragData.data();
		structuredBufferConfig.m_format						=	ResourceViewFormat::DXGI_FORMAT_UNKNOWN;
		structuredBufferConfig.m_isStandard					=	false;
		structuredBufferConfig.m_type						=	ResourceType::STRUCTURED_BUFFER;
		structuredBufferConfig.m_debugName					=	"FragmentsPerPixelStructuredBuffer_4";
		structuredBufferConfig.m_debugNameSize				=	sizeof("FragmentsPerPixelStructuredBuffer_4");
		g_theRenderer->CreateResourceFromConfig(structuredBufferConfig, m_fragmentsPerPixelStructuredBuffer_4);

		// Byte-Address Buffer
		numOfElements = windowDim.x * windowDim.y;
		std::vector<unsigned int> defaultRootLinkedListData;
		defaultRootLinkedListData.resize(numOfElements, 0xFFFFFFFFU);
		// g_theRenderer->CreateRawBuffer(m_byteAddressBufferOfHeadLinkedList, windowDim.x * windowDim.y, ResourceUsage::GPU_READ_GPU_WRITE, defaultRootLinkedListData.data());
		// g_theRenderer->SetDebugResourceName(m_byteAddressBufferOfHeadLinkedList, sizeof("HeadOfFragmentsPerPixelLinkedList"), "HeadOfFragmentsPerPixelLinkedList");

		
		D3D11_ResourceConfig rawBufferConfig			=	{ };
		rawBufferConfig.m_numOfElements					=	numOfElements;
		rawBufferConfig.m_elementStride					=	4;
		rawBufferConfig.m_usageFlag						=	ResourceUsage::GPU_READ_GPU_WRITE;
		rawBufferConfig.m_defaultInitializationData		=	defaultRootLinkedListData.data();
		rawBufferConfig.m_type							=	ResourceType::RAW_BUFFER;
		rawBufferConfig.m_debugName						=	"headNodeByteAddressBuffer_4";
		rawBufferConfig.m_debugNameSize					=	sizeof("headNodeByteAddressBuffer_4");
		g_theRenderer->CreateResourceFromConfig(rawBufferConfig, m_headNodeByteAddressBuffer_4);
	}


	{
		// Buffer resources 
		// Structured Buffer
		int numOfNodes		=	2;										// Make sure to change the number of nodes in the hlsl (compute shader, both clear and other one)as well
		int numOfElements	=	windowDim.x * windowDim.y * numOfNodes;
		std::vector<Fragment> defaultFragData;
		defaultFragData.resize((size_t)numOfElements);
		
		D3D11_ResourceConfig structuredBufferConfig			=	{ };
		structuredBufferConfig.m_numOfElements				=	numOfElements;
		structuredBufferConfig.m_elementStride				=	sizeof(Fragment);
		structuredBufferConfig.m_usageFlag					=	ResourceUsage::GPU_READ_GPU_WRITE;
		structuredBufferConfig.m_defaultInitializationData	=	defaultFragData.data();
		structuredBufferConfig.m_format						=	ResourceViewFormat::DXGI_FORMAT_UNKNOWN;
		structuredBufferConfig.m_isStandard					=	false;
		structuredBufferConfig.m_type						=	ResourceType::STRUCTURED_BUFFER;
		structuredBufferConfig.m_debugName					=	"FragmentsPerPixelStructuredBuffer_2";
		structuredBufferConfig.m_debugNameSize				=	sizeof("FragmentsPerPixelStructuredBuffer_2");
		g_theRenderer->CreateResourceFromConfig(structuredBufferConfig, m_fragmentsPerPixelStructuredBuffer_2);

		// Byte-Address Buffer
		numOfElements = windowDim.x * windowDim.y;
		std::vector<unsigned int> defaultRootLinkedListData;
		defaultRootLinkedListData.resize(numOfElements, 0xFFFFFFFFU);
		// g_theRenderer->CreateRawBuffer(m_byteAddressBufferOfHeadLinkedList, windowDim.x * windowDim.y, ResourceUsage::GPU_READ_GPU_WRITE, defaultRootLinkedListData.data());
		// g_theRenderer->SetDebugResourceName(m_byteAddressBufferOfHeadLinkedList, sizeof("HeadOfFragmentsPerPixelLinkedList"), "HeadOfFragmentsPerPixelLinkedList");

		
		D3D11_ResourceConfig rawBufferConfig			=	{ };
		rawBufferConfig.m_numOfElements					=	numOfElements;
		rawBufferConfig.m_elementStride					=	4;
		rawBufferConfig.m_usageFlag						=	ResourceUsage::GPU_READ_GPU_WRITE;
		rawBufferConfig.m_defaultInitializationData		=	defaultRootLinkedListData.data();
		rawBufferConfig.m_type							=	ResourceType::RAW_BUFFER;
		rawBufferConfig.m_debugName						=	"headNodeByteAddressBuffer_2";
		rawBufferConfig.m_debugNameSize					=	sizeof("headNodeByteAddressBuffer_2");
		g_theRenderer->CreateResourceFromConfig(rawBufferConfig, m_headNodeByteAddressBuffer_2);
	}


	// Weighted Blended OIT textures
	D3D11_ResourceConfig accumulationRenderTargetConfig		=	{ };
	accumulationRenderTargetConfig.m_type					=	ResourceType::TEXTURE2D;
	accumulationRenderTargetConfig.m_usageFlag				=	ResourceUsage::GPU_READ_GPU_WRITE;
	accumulationRenderTargetConfig.m_format					=	ResourceViewFormat::DXGI_FORMAT_R16G16B16A16_FLOAT;
	accumulationRenderTargetConfig.m_bindFlags				=	ResourceBindFlag((unsigned int)ResourceBindFlag::RENDER_TARGET | (unsigned int)ResourceBindFlag::SHADER_RESOURCE);
	accumulationRenderTargetConfig.m_debugName				=	"AccumulationRenderTarget";
	accumulationRenderTargetConfig.m_debugNameSize			=	sizeof("AccumulationRenderTarget");
	accumulationRenderTargetConfig.m_width					=	windowDim.x;
	accumulationRenderTargetConfig.m_height					=	windowDim.y;
	g_theRenderer->CreateResourceFromConfig(accumulationRenderTargetConfig, m_accumulationTargetWeightedBlended);

	D3D11_ResourceConfig revealageRenderTargetConfig		=	{ };
	revealageRenderTargetConfig.m_type				=	ResourceType::TEXTURE2D;
	revealageRenderTargetConfig.m_usageFlag			=	ResourceUsage::GPU_READ_GPU_WRITE;
	revealageRenderTargetConfig.m_format			=	ResourceViewFormat::DXGI_FORMAT_R16_FLOAT;
	revealageRenderTargetConfig.m_bindFlags			=	ResourceBindFlag((unsigned int)ResourceBindFlag::RENDER_TARGET | (unsigned int)ResourceBindFlag::SHADER_RESOURCE);
	revealageRenderTargetConfig.m_debugName			=	"RevealageRenderTarget";
	revealageRenderTargetConfig.m_debugNameSize		=	sizeof("RevealageRenderTarget");
	revealageRenderTargetConfig.m_width				=	windowDim.x;
	revealageRenderTargetConfig.m_height			=	windowDim.y;
	g_theRenderer->CreateResourceFromConfig(revealageRenderTargetConfig, m_revealageTargetWeightedBlended);

	// MLAB Resources
	m_screenConstantBuffer					=	g_theRenderer->CreateConstantBuffer(sizeof(ScreenConstants));
	ScreenConstants screenConstantsMLAB		=	{ };
	screenConstantsMLAB.m_screenWidth		=	(float)windowDim.x;
	g_theRenderer->CopyCPUToGPU(&screenConstantsMLAB, sizeof(ScreenConstants), m_screenConstantBuffer);
	g_theRenderer->BindConstantBuffer(g_screenConstantsSlot, m_screenConstantBuffer, BindingLocation::COMPUTE_SHADER);

	// Unoptimized MLAB Resources
	// // Untouched Fragment Mask
	size_t numOfPixels = size_t(windowDim.x) * size_t(windowDim.y);
	std::vector<unsigned char> unoptimizedMLABClearedMaskDefaultData;
	unoptimizedMLABClearedMaskDefaultData.resize(numOfPixels, (unsigned char)1);

	D3D11_ResourceConfig unoptimizedMLABUntouchedFragmentMaskConfig			=	{ };
	unoptimizedMLABUntouchedFragmentMaskConfig.m_type						=	ResourceType::TEXTURE2D;
	unoptimizedMLABUntouchedFragmentMaskConfig.m_format						=	ResourceViewFormat::DXGI_FORMAT_R8_UINT;
	unoptimizedMLABUntouchedFragmentMaskConfig.m_bindFlags					=	ResourceBindFlag((unsigned int)(ResourceBindFlag::UNORDERED_ACCESS) | (unsigned int)(ResourceBindFlag::SHADER_RESOURCE));		
	unoptimizedMLABUntouchedFragmentMaskConfig.m_usageFlag					=	ResourceUsage::GPU_READ_GPU_WRITE;
	unoptimizedMLABUntouchedFragmentMaskConfig.m_debugName					=	"Unoptimized MLAB Untouched Fragment Mask";
	unoptimizedMLABUntouchedFragmentMaskConfig.m_debugNameSize				=	sizeof("Unoptimized MLAB Untouched Fragment Mask");
	unoptimizedMLABUntouchedFragmentMaskConfig.m_width						=	windowDim.x;
	unoptimizedMLABUntouchedFragmentMaskConfig.m_height						=	windowDim.y;
	unoptimizedMLABUntouchedFragmentMaskConfig.m_defaultInitializationData	=   unoptimizedMLABClearedMaskDefaultData.data();
	unoptimizedMLABUntouchedFragmentMaskConfig.m_sizeOfTexelInBytes			=	1;
	g_theRenderer->CreateResourceFromConfig(unoptimizedMLABUntouchedFragmentMaskConfig, m_unoptimizedMLABUntouchedFragmentMask);

	// // Blending Array Structured Buffer
	{
		UnoptimizedFragmentArray_2* blendingArray = new UnoptimizedFragmentArray_2[numOfPixels]();

		D3D11_ResourceConfig unoptimizedMLABstructuredBufferConfig = { };
		unoptimizedMLABstructuredBufferConfig.m_defaultInitializationData = blendingArray;
		unoptimizedMLABstructuredBufferConfig.m_numOfElements = (unsigned int)numOfPixels;
		unoptimizedMLABstructuredBufferConfig.m_elementStride = sizeof(UnoptimizedFragmentArray_2);
		unoptimizedMLABstructuredBufferConfig.m_isStandard = false;
		unoptimizedMLABstructuredBufferConfig.m_usageFlag = ResourceUsage::GPU_READ_GPU_WRITE;
		unoptimizedMLABstructuredBufferConfig.m_format = ResourceViewFormat::DXGI_FORMAT_UNKNOWN;
		unoptimizedMLABstructuredBufferConfig.m_type = ResourceType::STRUCTURED_BUFFER;
		unoptimizedMLABstructuredBufferConfig.m_debugNameSize = sizeof("Unoptimized MLAB Blending Array");
		unoptimizedMLABstructuredBufferConfig.m_debugName = "Unoptimized MLAB Blending Array";
		g_theRenderer->CreateResourceFromConfig(unoptimizedMLABstructuredBufferConfig, m_unoptimizedMLABStructuredBuffer_2);
	}

	{
		UnoptimizedFragmentArray_4* blendingArray	= new UnoptimizedFragmentArray_4[numOfPixels]();

		D3D11_ResourceConfig unoptimizedMLABstructuredBufferConfig			=	{ };
		unoptimizedMLABstructuredBufferConfig.m_defaultInitializationData	=	blendingArray;
		unoptimizedMLABstructuredBufferConfig.m_numOfElements				=	(unsigned int)numOfPixels;
		unoptimizedMLABstructuredBufferConfig.m_elementStride				=	sizeof(UnoptimizedFragmentArray_4);
		unoptimizedMLABstructuredBufferConfig.m_isStandard					=	false;
		unoptimizedMLABstructuredBufferConfig.m_usageFlag					=	ResourceUsage::GPU_READ_GPU_WRITE;
		unoptimizedMLABstructuredBufferConfig.m_format						=	ResourceViewFormat::DXGI_FORMAT_UNKNOWN;
		unoptimizedMLABstructuredBufferConfig.m_type						=	ResourceType::STRUCTURED_BUFFER;
		unoptimizedMLABstructuredBufferConfig.m_debugNameSize				=	sizeof("Unoptimized MLAB Blending Array_4");
		unoptimizedMLABstructuredBufferConfig.m_debugName					=	"Unoptimized MLAB Blending Array_4";
		g_theRenderer->CreateResourceFromConfig(unoptimizedMLABstructuredBufferConfig, m_unoptimizedMLABStructuredBuffer_4);
	}

	{
		UnoptimizedFragmentArray_32* blendingArray = new UnoptimizedFragmentArray_32[numOfPixels]();

		D3D11_ResourceConfig unoptimizedMLABstructuredBufferConfig = { };
		unoptimizedMLABstructuredBufferConfig.m_defaultInitializationData = blendingArray;
		unoptimizedMLABstructuredBufferConfig.m_numOfElements = (unsigned int)numOfPixels;
		unoptimizedMLABstructuredBufferConfig.m_elementStride = sizeof(UnoptimizedFragmentArray_32);
		unoptimizedMLABstructuredBufferConfig.m_isStandard = false;
		unoptimizedMLABstructuredBufferConfig.m_usageFlag = ResourceUsage::GPU_READ_GPU_WRITE;
		unoptimizedMLABstructuredBufferConfig.m_format = ResourceViewFormat::DXGI_FORMAT_UNKNOWN;
		unoptimizedMLABstructuredBufferConfig.m_type = ResourceType::STRUCTURED_BUFFER;
		unoptimizedMLABstructuredBufferConfig.m_debugNameSize = sizeof("Unoptimized MLAB Blending Array_32");
		unoptimizedMLABstructuredBufferConfig.m_debugName = "Unoptimized MLAB Blending Array_32";
		g_theRenderer->CreateResourceFromConfig(unoptimizedMLABstructuredBufferConfig, m_unoptimizedMLABStructuredBuffer_32);
	}
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeShaders()
{
	m_defaultShader									=	g_theRenderer->CreateOrGetShader("Data/Shaders/Default");
	m_vpmPeelingShader								=	g_theRenderer->CreateOrGetShader("Data/Shaders/VPMPeelingShader");
	m_vpmDepthCompositeShader						=	g_theRenderer->CreateComputeShader("Data/Shaders/VPMDepthCompositeShader");
	m_vpmColorCompositeShader						=	g_theRenderer->CreateComputeShader("Data/Shaders/VPMColorCompositeShader");
	m_depthPeelingShader							=	g_theRenderer->CreateOrGetShader("Data/Shaders/DepthPeelingShader");
	m_depthPeelingDepthCompositeShader				=	g_theRenderer->CreateComputeShader("Data/Shaders/DepthPeelingDepthCompositeShader");
	m_depthPeelingColorCompositeShader				=	g_theRenderer->CreateComputeShader("Data/Shaders/DepthPeelingColorCompositeShader");
	m_depthPeelingFinalBackgroundPassShader			=	g_theRenderer->CreateComputeShader("Data/Shaders/DepthPeelingFinalBackgroundPassShader");
	m_underCompositeShader							=	g_theRenderer->CreateComputeShader("Data/Shaders/UnderCompositeShader");
	m_uavWriteWithROVShader							=	g_theRenderer->CreateOrGetShader("Data/Shaders/UAVWriteWithROVShader");
	m_uavWriteWithoutROVShader						=	g_theRenderer->CreateOrGetShader("Data/Shaders/UAVWriteWithoutROVShader");
	m_defaultPremultipliedAlphaShader				=	g_theRenderer->CreateOrGetShader("Data/Shaders/DefaultPremultipliedAlphaShader");
	m_perPixelLinkedListClearBufferShader_32		=	g_theRenderer->CreateComputeShader("Data/Shaders/ClearBuffers");
	m_perPixelLinkedListCompositePassShader_32		=	g_theRenderer->CreateComputeShader("Data/Shaders/PopulateRenderTarget");
	m_perPixelLinkedListClearBufferShader_4			=	g_theRenderer->CreateComputeShader("Data/Shaders/ClearBuffers_4");
	m_perPixelLinkedListCompositePassShader_4		=	g_theRenderer->CreateComputeShader("Data/Shaders/PopulateRenderTarget_4");
	m_perPixelLinkedListClearBufferShader_2			=	g_theRenderer->CreateComputeShader("Data/Shaders/ClearBuffers_2");
	m_perPixelLinkedListCompositePassShader_2		=	g_theRenderer->CreateComputeShader("Data/Shaders/PopulateRenderTarget_2");
	m_populatePerPixelLinkedListShader				=	g_theRenderer->CreateOrGetShader("Data/Shaders/PopulateFragmentLinkedList");
	m_populateAccumRevealRTsShader					=	g_theRenderer->CreateOrGetShader("Data/Shaders/PopulateAccumRevealRTs");				
	m_weightedBlendedCompositeShader				=	g_theRenderer->CreateOrGetShader("Data/Shaders/CompositePass");
	m_unoptimizedMLABPopulateBlendingArrayShader_2	=	g_theRenderer->CreateOrGetShader("Data/Shaders/UnoptimizedMLABPopulateBlendingArrayShader");
	m_unoptimizedMLABPopulateRenderTargetShader_2	=	g_theRenderer->CreateComputeShader("Data/Shaders/UnoptimizedMLABPopulateRTShader");
	m_unoptimizedMLABPopulateBlendingArrayShader_4	=	g_theRenderer->CreateOrGetShader("Data/Shaders/UnoptimizedMLABPopulateBlendingArrayShader_4");
	m_unoptimizedMLABPopulateRenderTargetShader_4	=	g_theRenderer->CreateComputeShader("Data/Shaders/UnoptimizedMLABPopulateRTShader_4");
	m_unoptimizedMLABPopulateBlendingArrayShader_32	=	g_theRenderer->CreateOrGetShader("Data/Shaders/UnoptimizedMLABPopulateBlendingArrayShader_32");
	m_unoptimizedMLABPopulateRenderTargetShader_32	=	g_theRenderer->CreateComputeShader("Data/Shaders/UnoptimizedMLABPopulateRTShader_32");
}


//--------------------------------------------------------------------------------------------------
void Game::InitializeMeshes()
{
	// for (unsigned char meshTypeIndex = 0; meshTypeIndex < eMESH_TYPE_COUNT; ++meshTypeIndex)
	// {
	// 	 CreateMeshOfType(m_meshList[meshTypeIndex], (eMeshType)meshTypeIndex);
	// }
}