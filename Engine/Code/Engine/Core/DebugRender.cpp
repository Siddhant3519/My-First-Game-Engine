#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/DebugRender.hpp"


//--------------------------------------------------------------------------------------------------
#include <mutex>


//--------------------------------------------------------------------------------------------------
DebugRenderConfig g_theConfig;
bool g_isVisible = false;
std::mutex g_debugRenderMutex;


//--------------------------------------------------------------------------------------------------
struct DebugRenderAlways
{
	std::vector<Vertex_PCU> m_alwaysVertexes;
	Stopwatch* m_alwaysStopwatch = nullptr;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
};

struct DebugRenderDepth
{
	std::vector<Vertex_PCU> m_depthVertexes;
	Stopwatch* m_depthStopwatch = nullptr;
	float m_duration = -2.f;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
};

struct DebugRenderXRay
{
	std::vector<Vertex_PCU> m_xRayVertexes;
	Stopwatch* m_xRayStopwatch = nullptr;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
};

struct DebugRenderText
{
	std::vector<Vertex_PCU> m_textVertexes;
	Stopwatch* m_textStopwatch = nullptr;
	float m_duration = 0.f;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	Vec3 origin;
};

struct DebugRenderWorldText
{
	std::vector<Vertex_PCU> m_textVertexes;
	Stopwatch* m_textStopwatch = nullptr;
	float m_duration = 0.f;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	Vec3 origin;
};

struct DebugRenderScreenText
{
	std::vector<Vertex_PCU> m_screenTextVertexes;
	float m_duration = -2.f;
	Stopwatch* m_textStopwatch = nullptr;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
};

std::vector<DebugRenderDepth>		g_theDepthRenderVertexes;
std::vector<DebugRenderDepth>		g_theDepthRenderWireframeVertexes;
std::vector<DebugRenderAlways>		g_theAlwaysRenderVertexes;
std::vector<DebugRenderAlways>		g_theAlwaysRenderWireframeVertexes;
std::vector<DebugRenderXRay>		g_theXRayRenderVertexes;
std::vector<DebugRenderXRay>		g_theXRayRenderWireframeVertexes;
std::vector<DebugRenderText>		g_theTextRenderVertexes;
std::vector<DebugRenderWorldText>	g_theWorldText;
std::vector<DebugRenderScreenText>	g_theScreenTextRenderVertexes;

BitmapFont* g_Font = nullptr;

void PopulateDepthVertexes(std::vector<Vertex_PCU>& verts)
{
	for (int depthObjIndex = 0; depthObjIndex < (int)g_theDepthRenderVertexes.size(); ++depthObjIndex)
	{
		DebugRenderDepth& depthObj = g_theDepthRenderVertexes[depthObjIndex];

		float interpolatorFactor = depthObj.m_depthStopwatch->GetElapsedFraction();
		Rgba8 interpolatedColor = Interpolate(depthObj.m_startColor, depthObj.m_endColor, interpolatorFactor);
		for (int depthVertexIndex = 0; depthVertexIndex < depthObj.m_depthVertexes.size(); ++depthVertexIndex)
		{
			depthObj.m_depthVertexes[depthVertexIndex].m_color = interpolatedColor;
			verts.push_back(depthObj.m_depthVertexes[depthVertexIndex]);
		}
	}
}

void PopulateAlwaysVertexes(std::vector<Vertex_PCU>& verts, std::vector<DebugRenderAlways>& alwaysVerts)
{
	for (int alwaysObjIndex = 0; alwaysObjIndex < (int)g_theAlwaysRenderVertexes.size(); ++alwaysObjIndex)
	{
		DebugRenderAlways& alwaysObj = alwaysVerts[alwaysObjIndex];

		float interpolatorFactor = alwaysObj.m_alwaysStopwatch->GetElapsedFraction();
		Rgba8 interpolatedColor = Interpolate(alwaysObj.m_startColor, alwaysObj.m_endColor, interpolatorFactor);
		for (int AlwaysVertexIndex = 0; AlwaysVertexIndex < alwaysObj.m_alwaysVertexes.size(); ++AlwaysVertexIndex)
		{
			alwaysObj.m_alwaysVertexes[AlwaysVertexIndex].m_color = interpolatedColor;
			verts.push_back(alwaysObj.m_alwaysVertexes[AlwaysVertexIndex]);
		}
	}
}

void PopulateDepthVertexes(std::vector<Vertex_PCU>& verts, std::vector<DebugRenderDepth>& depthverts)
{
	for (int depthObjIndex = 0; depthObjIndex < (int)depthverts.size(); ++depthObjIndex)
	{
		DebugRenderDepth& depthObj = depthverts[depthObjIndex];

		float interpolatorFactor = depthObj.m_depthStopwatch->GetElapsedFraction();
		Rgba8 interpolatedColor = Interpolate(depthObj.m_startColor, depthObj.m_endColor, interpolatorFactor);
		for (int depthVertexIndex = 0; depthVertexIndex < depthObj.m_depthVertexes.size(); ++depthVertexIndex)
		{
			depthObj.m_depthVertexes[depthVertexIndex].m_color = interpolatedColor;
			verts.push_back(depthObj.m_depthVertexes[depthVertexIndex]);
		}
	}
}

void PopulateXRayAlphaVertexes(std::vector<Vertex_PCU>& verts, std::vector<DebugRenderXRay>& alphaVerts)
{
	for (int xRayObjIndex = 0; xRayObjIndex < (int)g_theXRayRenderVertexes.size(); ++xRayObjIndex)
	{
		DebugRenderXRay& xRayObj = alphaVerts[xRayObjIndex];

		float interpolatorFactor = xRayObj.m_xRayStopwatch->GetElapsedFraction();

		xRayObj.m_startColor.a = 127;// char(xRayObj.m_startColor.a * 0.5f);
		xRayObj.m_endColor.a = 127;//  char(xRayObj.m_endColor.a * 0.5f);

		Rgba8 interpolatedColor = Interpolate(xRayObj.m_startColor, xRayObj.m_endColor, interpolatorFactor);
		for (int AlwaysVertexIndex = 0; AlwaysVertexIndex < xRayObj.m_xRayVertexes.size(); ++AlwaysVertexIndex)
		{
			xRayObj.m_xRayVertexes[AlwaysVertexIndex].m_color = interpolatedColor;
			verts.push_back(xRayObj.m_xRayVertexes[AlwaysVertexIndex]);
		}
	}
}

void PopulateXRaySolidVertexes(std::vector<Vertex_PCU>& verts, std::vector<DebugRenderXRay>& solidVerts)
{
	for (int xRayObjIndex = 0; xRayObjIndex < (int)g_theXRayRenderVertexes.size(); ++xRayObjIndex)
	{
		DebugRenderXRay& xRayObj = solidVerts[xRayObjIndex];

		float interpolatorFactor = xRayObj.m_xRayStopwatch->GetElapsedFraction();
		Rgba8 interpolatedColor = Interpolate(xRayObj.m_startColor, xRayObj.m_endColor, interpolatorFactor);
		for (int AlwaysVertexIndex = 0; AlwaysVertexIndex < xRayObj.m_xRayVertexes.size(); ++AlwaysVertexIndex)
		{
			xRayObj.m_xRayVertexes[AlwaysVertexIndex].m_color = interpolatedColor;
			verts.push_back(xRayObj.m_xRayVertexes[AlwaysVertexIndex]);
		}
	}
}

void PopulateScreenTextVertexes(std::vector<Vertex_PCU>& verts, std::vector<DebugRenderScreenText>& textVerts)
{
	for (int textIndex = 0; textIndex < (int)textVerts.size(); ++textIndex)
	{
		DebugRenderScreenText& textObj = textVerts[textIndex];
		if (textObj.m_duration == 0)
		{
			for (int textVertexIndex = 0; textVertexIndex < textObj.m_screenTextVertexes.size(); ++textVertexIndex)
			{
				verts.push_back(textObj.m_screenTextVertexes[textVertexIndex]);
			}
			continue;
		}
		float interpolatorFactor = textObj.m_textStopwatch->GetElapsedFraction();
		Rgba8 interpolatedColor = Interpolate(textObj.m_startColor, textObj.m_endColor, interpolatorFactor);
		for (int textVertexIndex = 0; textVertexIndex < textObj.m_screenTextVertexes.size(); ++textVertexIndex)
		{
			textObj.m_screenTextVertexes[textVertexIndex].m_color = interpolatedColor;
			verts.push_back(textObj.m_screenTextVertexes[textVertexIndex]);
		}
	}
}

void PopulateTextVertexes(Camera const& camera, std::vector<DebugRenderText>& textVerts)
{
	for (int textIndex = 0; textIndex < (int)textVerts.size(); ++textIndex)
	{
		DebugRenderText& textObj = textVerts[textIndex];

		float interpolatorFactor = textObj.m_textStopwatch->GetElapsedFraction();
		Rgba8 interpolatedColor = Interpolate(textObj.m_startColor, textObj.m_endColor, interpolatorFactor);
		for (int textVertexIndex = 0; textVertexIndex < textObj.m_textVertexes.size(); ++textVertexIndex)
		{
			textObj.m_textVertexes[textVertexIndex].m_color = interpolatedColor;
		}

		Mat44 camMat = camera.GetCameraOrientation().GetAsMatrix_XFwd_YLeft_ZUp();
		camMat.SetTranslation3D(camera.GetCameraPosition());
		// Mat44 billboardMat = GetBillboardMatrix(BillboardType::FULL_CAMERA_OPPOSING, camMat, textObj.origin);
		Mat44 billboardMat = GetBillboardMatrix(BillboardType::WORLD_UP_CAMERA_FACING, camMat, textObj.origin);
		g_theConfig.m_renderer->SetModelConstants(billboardMat);
		g_theConfig.m_renderer->SetBlendMode(BlendMode::ALPHA);
		g_theConfig.m_renderer->SetDepthMode(DepthMode::ENABLED);
		g_theConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		g_theConfig.m_renderer->BindTexture(&g_Font->GetTexture());
		g_theConfig.m_renderer->DrawVertexArray((int)textObj.m_textVertexes.size(), textObj.m_textVertexes.data());
	}
}

void PopulateWorldText(std::vector<Vertex_PCU>& verts, std::vector<DebugRenderWorldText>& textVerts)
{
	for (int textIndex = 0; textIndex < (int)textVerts.size(); ++textIndex)
	{
		DebugRenderWorldText& textObj = textVerts[textIndex];

		float interpolatorFactor = textObj.m_textStopwatch->GetElapsedFraction();
		Rgba8 interpolatedColor = Interpolate(textObj.m_startColor, textObj.m_endColor, interpolatorFactor);
		for (int textVertexIndex = 0; textVertexIndex < textObj.m_textVertexes.size(); ++textVertexIndex)
		{
			textObj.m_textVertexes[textVertexIndex].m_color = interpolatedColor;
			verts.push_back(textObj.m_textVertexes[textVertexIndex]);
		}
	}
}

void DebugRenderSystemStartup(DebugRenderConfig const& config)
{
	g_theConfig.m_renderer = config.m_renderer;
	g_theConfig.m_startHidden = config.m_startHidden;
	g_isVisible = config.m_startHidden;

	g_Font = config.m_renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
}

void DebugRenderSystemShutdown()
{
}

void DebugRenderSetVisible()
{
	g_isVisible = true;
}

void DebugRenderSetHidden()
{
	g_isVisible = false;
}

void DebugRenderClear()
{
	g_theDepthRenderVertexes.clear();
	g_theDepthRenderWireframeVertexes.clear();
	g_theAlwaysRenderVertexes.clear();
	g_theAlwaysRenderWireframeVertexes.clear();
	g_theXRayRenderVertexes.clear();
	g_theXRayRenderWireframeVertexes.clear();
	g_theTextRenderVertexes.clear();
	g_theWorldText.clear();
}

void DebugRenderToggle()
{
	if (g_isVisible)
	{
		DebugRenderSetHidden();
	}
	else
	{
		DebugRenderSetVisible();
	}
}

void DebugRenderBeginFrame()
{
	for (int depthObjIndex = 0; depthObjIndex < (int)g_theDepthRenderVertexes.size(); ++depthObjIndex)
	{
		DebugRenderDepth& depthObj = g_theDepthRenderVertexes[depthObjIndex];

		if (depthObj.m_duration == -1.f)
		{
			continue;
		}

		if (depthObj.m_depthStopwatch->HasDurationElapsed())
		{
			DebugRenderDepth lastDepth = g_theDepthRenderVertexes[g_theDepthRenderVertexes.size() - 1];
			g_theDepthRenderVertexes[g_theDepthRenderVertexes.size() - 1] = depthObj;
			depthObj = lastDepth;
			g_theDepthRenderVertexes.pop_back();
		}

		if (depthObj.m_depthStopwatch->IsStopped())
		{
			depthObj.m_depthStopwatch->Start();
		}
	}

	for (int depthObjIndex = 0; depthObjIndex < (int)g_theDepthRenderWireframeVertexes.size(); ++depthObjIndex)
	{
		DebugRenderDepth& depthObj = g_theDepthRenderWireframeVertexes[depthObjIndex];

		if (depthObj.m_depthStopwatch->HasDurationElapsed())
		{
			DebugRenderDepth lastDepth = g_theDepthRenderWireframeVertexes[g_theDepthRenderWireframeVertexes.size() - 1];
			g_theDepthRenderWireframeVertexes[g_theDepthRenderWireframeVertexes.size() - 1] = depthObj;
			depthObj = lastDepth;
			g_theDepthRenderWireframeVertexes.pop_back();
		}

		if (depthObj.m_depthStopwatch->IsStopped())
		{
			depthObj.m_depthStopwatch->Start();
		}
	}

	for (int alwaysObjIndex = 0; alwaysObjIndex < (int)g_theAlwaysRenderVertexes.size(); ++alwaysObjIndex)
	{
		DebugRenderAlways& alwaysObj = g_theAlwaysRenderVertexes[alwaysObjIndex];

		if (alwaysObj.m_alwaysStopwatch->HasDurationElapsed())
		{
			DebugRenderAlways lastAlways = g_theAlwaysRenderVertexes[g_theAlwaysRenderVertexes.size() - 1];
			g_theAlwaysRenderVertexes[g_theAlwaysRenderVertexes.size() - 1] = alwaysObj;
			alwaysObj = lastAlways;
			g_theAlwaysRenderVertexes.pop_back();
		}

		else if (alwaysObj.m_alwaysStopwatch->IsStopped())
		{
			alwaysObj.m_alwaysStopwatch->Start();
		}
	}

	for (int xRayObjIndex = 0; xRayObjIndex < (int)g_theXRayRenderVertexes.size(); ++xRayObjIndex)
	{
		DebugRenderXRay& xRayObj = g_theXRayRenderVertexes[xRayObjIndex];

		if (xRayObj.m_xRayStopwatch->HasDurationElapsed())
		{
			DebugRenderXRay lastXRay = g_theXRayRenderVertexes[g_theXRayRenderVertexes.size() - 1];
			g_theXRayRenderVertexes[g_theXRayRenderVertexes.size() - 1] = xRayObj;
			xRayObj = lastXRay;
			g_theXRayRenderVertexes.pop_back();
		}

		else if (xRayObj.m_xRayStopwatch->IsStopped())
		{
			xRayObj.m_xRayStopwatch->Start();
		}
	}

	for (int textObjIndex = 0; textObjIndex < (int)g_theTextRenderVertexes.size(); ++textObjIndex)
	{
		DebugRenderText& textObj = g_theTextRenderVertexes[textObjIndex];

		if (textObj.m_textStopwatch->HasDurationElapsed())
		{
			DebugRenderText lastText = g_theTextRenderVertexes[g_theTextRenderVertexes.size() - 1];
			g_theTextRenderVertexes[g_theTextRenderVertexes.size() - 1] = textObj;
			textObj = lastText;
			g_theTextRenderVertexes.pop_back();
		}

		else if (textObj.m_textStopwatch->IsStopped())
		{
			textObj.m_textStopwatch->Start();
		}
	}

	for (int textObjIndex = 0; textObjIndex < (int)g_theWorldText.size(); ++textObjIndex)
	{
		DebugRenderWorldText& textObj = g_theWorldText[textObjIndex];
	
		if (textObj.m_duration == -1.f)
		{
			continue;
		}
		if (textObj.m_textStopwatch->HasDurationElapsed())
		{
			DebugRenderWorldText lastText = g_theWorldText[g_theWorldText.size() - 1];
			g_theWorldText[g_theWorldText.size() - 1] = textObj;
			textObj = lastText;
			g_theWorldText.pop_back();
		}

		else if (textObj.m_textStopwatch->IsStopped())
		{
			textObj.m_textStopwatch->Start();
		}
	}
}

void DebugRenderWorld(Camera const& camera)
{
	if (!g_isVisible)
	{
		return;
	}
	g_theConfig.m_renderer->BeginCamera(camera);
	std::vector<Vertex_PCU> alwaysVertexes;
	PopulateAlwaysVertexes(alwaysVertexes, g_theAlwaysRenderVertexes);
	g_theConfig.m_renderer->SetDepthMode(DepthMode::DISABLED);
	g_theConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theConfig.m_renderer->BindTexture(nullptr);
	g_theConfig.m_renderer->DrawVertexArray((int)alwaysVertexes.size(), alwaysVertexes.data());

	std::vector<Vertex_PCU> depthVertexes;
	PopulateDepthVertexes(depthVertexes);
	g_theConfig.m_renderer->SetDepthMode(DepthMode::ENABLED);
	g_theConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theConfig.m_renderer->BindTexture(nullptr);
	g_theConfig.m_renderer->DrawVertexArray((int)depthVertexes.size(), depthVertexes.data());

	std::vector<Vertex_PCU> depthWireframeVerts;
	PopulateDepthVertexes(depthWireframeVerts, g_theDepthRenderWireframeVertexes);
	g_theConfig.m_renderer->SetDepthMode(DepthMode::ENABLED);
	g_theConfig.m_renderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
	g_theConfig.m_renderer->BindTexture(nullptr);
	g_theConfig.m_renderer->DrawVertexArray((int)depthWireframeVerts.size(), depthWireframeVerts.data());
	
	
	std::vector<Vertex_PCU> xRayAlphaVertexes;
	PopulateXRayAlphaVertexes(xRayAlphaVertexes, g_theXRayRenderVertexes);
	g_theConfig.m_renderer->SetBlendMode(BlendMode::ALPHA);
	g_theConfig.m_renderer->SetDepthMode(DepthMode::DISABLED);
	g_theConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theConfig.m_renderer->BindTexture(nullptr);
	g_theConfig.m_renderer->DrawVertexArray((int)xRayAlphaVertexes.size(), xRayAlphaVertexes.data());
	
	std::vector<Vertex_PCU> xRayDepthVertexes;
	PopulateXRaySolidVertexes(xRayDepthVertexes, g_theXRayRenderVertexes);
	g_theConfig.m_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_theConfig.m_renderer->SetDepthMode(DepthMode::ENABLED);
	g_theConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theConfig.m_renderer->BindTexture(nullptr);
	g_theConfig.m_renderer->DrawVertexArray((int)xRayDepthVertexes.size(), xRayDepthVertexes.data());

	std::vector<Vertex_PCU> worldTextVerts;
	PopulateWorldText(worldTextVerts, g_theWorldText);
	g_theConfig.m_renderer->SetBlendMode(BlendMode::ALPHA);
	g_theConfig.m_renderer->SetDepthMode(DepthMode::ENABLED);
	g_theConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theConfig.m_renderer->BindTexture(&g_Font->GetTexture());
	g_theConfig.m_renderer->DrawVertexArray((int)worldTextVerts.size(), worldTextVerts.data());

	PopulateTextVertexes(camera, g_theTextRenderVertexes);

	g_theConfig.m_renderer->EndCamera(camera);
}

void DebugRenderScreen(Camera const& camera)
{
	if (!g_isVisible)
	{
		return;
	}
	g_theConfig.m_renderer->BeginCamera(camera);

	std::vector<Vertex_PCU> screenTextVertexes;
	PopulateScreenTextVertexes(screenTextVertexes, g_theScreenTextRenderVertexes);
	g_theConfig.m_renderer->SetBlendMode(BlendMode::ALPHA);
	g_theConfig.m_renderer->SetDepthMode(DepthMode::DISABLED);
	g_theConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theConfig.m_renderer->BindTexture(&g_Font->GetTexture());
	g_theConfig.m_renderer->DrawVertexArray((int)screenTextVertexes.size(), screenTextVertexes.data());

	g_theConfig.m_renderer->EndCamera(camera);
}

void DebugRenderEndFrame()
{
	g_theScreenTextRenderVertexes.erase(
		remove_if(
			g_theScreenTextRenderVertexes.begin(), 
			g_theScreenTextRenderVertexes.end(),
			[=](auto const& element)
			{
				return element.m_textStopwatch->HasDurationElapsed() || element.m_duration == 0.f;
			}
		),
		g_theScreenTextRenderVertexes.end()
	);

	for (int textObjIndex = 0; textObjIndex < (int)g_theScreenTextRenderVertexes.size(); ++textObjIndex)
	{
		DebugRenderScreenText& textObj = g_theScreenTextRenderVertexes[textObjIndex];
		// if (textObj.m_duration == -1.f)
		// {
		// 	continue;
		// }
		// 
		// if (textObj.m_textStopwatch->HasDurationElapsed() || textObj.m_duration == 0.f)
		// {
		// 	DebugRenderScreenText lastText = g_theScreenTextRenderVertexes[g_theScreenTextRenderVertexes.size() - 1];
		// 	g_theScreenTextRenderVertexes[g_theScreenTextRenderVertexes.size() - 1] = textObj;
		// 	textObj = lastText;
		// 	g_theScreenTextRenderVertexes.pop_back();
		// }

		// else if (textObj.m_textStopwatch->IsStopped())
		if (textObj.m_textStopwatch->IsStopped())
		{
			textObj.m_textStopwatch->Start();
		}
	}

}

void DebugAddWorldPoint(Vec3 const& pos, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode)
{
	float numSlices = 8.f;
	float numStacks = 8.f;


	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways debugRenderAlways;
		debugRenderAlways.m_startColor = startColor;
		debugRenderAlways.m_endColor = endColor;
		debugRenderAlways.m_alwaysStopwatch = new Stopwatch(duration);

		AddVertsForUVSphereZ3D(debugRenderAlways.m_alwaysVertexes, pos, radius, numSlices, numStacks, startColor);
		g_debugRenderMutex.lock();
		g_theAlwaysRenderVertexes.push_back(debugRenderAlways);
		g_debugRenderMutex.unlock();
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepth debugRenderDepth;
		debugRenderDepth.m_startColor = startColor;
		debugRenderDepth.m_endColor = endColor;
		debugRenderDepth.m_depthStopwatch = new Stopwatch(duration);

		AddVertsForUVSphereZ3D(debugRenderDepth.m_depthVertexes, pos, radius, numSlices, numStacks, startColor);
		g_debugRenderMutex.lock();
		g_theDepthRenderVertexes.push_back(debugRenderDepth);
		g_debugRenderMutex.unlock();
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay debugRenderXRay;
		debugRenderXRay.m_startColor = startColor;
		debugRenderXRay.m_endColor = endColor;
		debugRenderXRay.m_xRayStopwatch = new Stopwatch(duration);

		AddVertsForUVSphereZ3D(debugRenderXRay.m_xRayVertexes, pos, radius, numSlices, numStacks, startColor);
		g_debugRenderMutex.lock();
		g_theXRayRenderVertexes.push_back(debugRenderXRay);
		g_debugRenderMutex.unlock();
		break;
	}
	default:
		break;
	}
}

void DebugAddWorldLine(Vec3 const& start, Vec3 const& end, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode)
{
	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways debugRenderAlways;
		debugRenderAlways.m_startColor = startColor;
		debugRenderAlways.m_endColor = endColor;
		debugRenderAlways.m_alwaysStopwatch = new Stopwatch(duration);

		AddVertsForCylinder3D(debugRenderAlways.m_alwaysVertexes, start, end, radius, startColor);
		g_debugRenderMutex.lock();
		g_theAlwaysRenderVertexes.push_back(debugRenderAlways);
		g_debugRenderMutex.unlock();
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepth debugRenderDepth;
		debugRenderDepth.m_startColor = startColor;
		debugRenderDepth.m_endColor = endColor;
		debugRenderDepth.m_depthStopwatch = new Stopwatch(duration);
		
		AddVertsForCylinder3D(debugRenderDepth.m_depthVertexes, start, end, radius, startColor);
		g_debugRenderMutex.lock();
		g_theDepthRenderVertexes.push_back(debugRenderDepth);
		g_debugRenderMutex.unlock();
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay debugRenderXRay;
		debugRenderXRay.m_startColor = startColor;
		debugRenderXRay.m_endColor = endColor;
		debugRenderXRay.m_xRayStopwatch = new Stopwatch(duration);

		AddVertsForCylinder3D(debugRenderXRay.m_xRayVertexes, start, end, radius, startColor);
		g_debugRenderMutex.lock();
		g_theXRayRenderVertexes.push_back(debugRenderXRay);
		g_debugRenderMutex.unlock();
		break;
	}
	default:
		break;
	}
}

void DebugAddWorldWireCylinder(Vec3 const& base, Vec3 const& top, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode)
{
	int numSlices = 8;

	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways debugRenderAlways;
		debugRenderAlways.m_startColor = startColor;
		debugRenderAlways.m_endColor = endColor;
		debugRenderAlways.m_alwaysStopwatch = new Stopwatch(duration);

		AddVertsForCylinder3D(debugRenderAlways.m_alwaysVertexes, base, top, radius, startColor, numSlices);
		g_debugRenderMutex.lock();
		g_theAlwaysRenderWireframeVertexes.push_back(debugRenderAlways);
		g_debugRenderMutex.unlock();
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepth debugRenderDepth;
		debugRenderDepth.m_startColor = startColor;
		debugRenderDepth.m_endColor = endColor;
		debugRenderDepth.m_depthStopwatch = new Stopwatch(duration);

		AddVertsForCylinder3D(debugRenderDepth.m_depthVertexes, base, top, radius, startColor, numSlices);
		g_debugRenderMutex.lock();
		g_theDepthRenderWireframeVertexes.push_back(debugRenderDepth);
		g_debugRenderMutex.unlock();
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay debugRenderXRay;
		debugRenderXRay.m_startColor = startColor;
		debugRenderXRay.m_endColor = endColor;
		debugRenderXRay.m_xRayStopwatch = new Stopwatch(duration);

		AddVertsForCylinder3D(debugRenderXRay.m_xRayVertexes, base, top, radius, startColor, numSlices);
		g_debugRenderMutex.lock();
		g_theXRayRenderWireframeVertexes.push_back(debugRenderXRay);
		g_debugRenderMutex.unlock();
		break;
	}
	default:
		break;
	}
}

void DebugAddWorldWireSphere(Vec3 const& center, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode)
{
	float numSlices = 16;
	float numStacks = 16;

	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways debugRenderAlways;
		debugRenderAlways.m_startColor = startColor;
		debugRenderAlways.m_endColor = endColor;
		debugRenderAlways.m_alwaysStopwatch = new Stopwatch(duration);

		AddVertsForUVSphereZ3D(debugRenderAlways.m_alwaysVertexes, center, radius, numSlices, numStacks, startColor);
		g_debugRenderMutex.lock();
		g_theAlwaysRenderVertexes.push_back(debugRenderAlways);
		g_debugRenderMutex.unlock();
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepth debugRenderDepth;
		debugRenderDepth.m_startColor = startColor;
		debugRenderDepth.m_endColor = endColor;
		debugRenderDepth.m_depthStopwatch = new Stopwatch(duration);

		AddVertsForUVSphereZ3D(debugRenderDepth.m_depthVertexes, center, radius, numSlices, numStacks, startColor);
		g_debugRenderMutex.lock();
		g_theDepthRenderWireframeVertexes.push_back(debugRenderDepth);
		g_debugRenderMutex.unlock();
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay debugRenderXRay;
		debugRenderXRay.m_startColor = startColor;
		debugRenderXRay.m_endColor = endColor;
		debugRenderXRay.m_xRayStopwatch = new Stopwatch(duration);

		AddVertsForUVSphereZ3D(debugRenderXRay.m_xRayVertexes, center, radius, numSlices, numStacks, startColor);
		g_debugRenderMutex.lock();
		g_theXRayRenderVertexes.push_back(debugRenderXRay);
		g_debugRenderMutex.unlock();
		break;
	}
	default:
		break;
	}
}

void DebugAddWorldArrow(Vec3 const& start, Vec3 const& end, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode)
{
	Vec3 iForward = (end - start);
	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways debugRenderAlways;
		debugRenderAlways.m_startColor = startColor;
		debugRenderAlways.m_endColor = endColor;
		debugRenderAlways.m_alwaysStopwatch = new Stopwatch(duration);

		AddVertsForCylinder3D(debugRenderAlways.m_alwaysVertexes, start, start + (iForward * 0.8f), radius * 0.75f, startColor, 8);
		AddVertsForCone3D(debugRenderAlways.m_alwaysVertexes, start + (iForward * 0.8f), end, radius * 1.5f, startColor);
		g_debugRenderMutex.lock();
		g_theAlwaysRenderVertexes.push_back(debugRenderAlways);
		g_debugRenderMutex.unlock();
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepth debugRenderDepth;
		debugRenderDepth.m_startColor = startColor;
		debugRenderDepth.m_endColor = endColor;
		debugRenderDepth.m_duration = duration;
		debugRenderDepth.m_depthStopwatch = new Stopwatch(duration);

		AddVertsForCylinder3D(debugRenderDepth.m_depthVertexes, start, start + (iForward * 0.8f), radius * 0.75f, startColor, 8);
		AddVertsForCone3D(debugRenderDepth.m_depthVertexes, start + (iForward * 0.8f), end, radius * 1.5f, startColor, 8);
		g_theDepthRenderVertexes.push_back(debugRenderDepth);
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay debugRenderXRay;
		debugRenderXRay.m_startColor = startColor;
		debugRenderXRay.m_endColor = endColor;
		debugRenderXRay.m_xRayStopwatch = new Stopwatch(duration);

		AddVertsForCylinder3D(debugRenderXRay.m_xRayVertexes, start, start + (iForward * 0.8f), radius * 0.65f, startColor, 8);
		AddVertsForCone3D(debugRenderXRay.m_xRayVertexes, start + (iForward * 0.8f), end, radius * 1.5f, startColor);
		g_theXRayRenderVertexes.push_back(debugRenderXRay);
		break;
	}
	default:
		break;
	}
}

void DebugAddWorldText(std::string const& text, Mat44 const& transform, float textHeight, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode)
{
	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		//DebugRenderAlways debugRenderAlways;
		//debugRenderAlways.m_startColor = startColor;
		//debugRenderAlways.m_endColor = endColor;
		//debugRenderAlways.m_alwaysStopwatch = new Stopwatch(duration);

		//g_theAlwaysRenderVertexes.push_back(debugRenderAlways);
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderWorldText debugRenderText;
		debugRenderText.m_startColor = startColor;
		debugRenderText.m_endColor = endColor;
		debugRenderText.m_duration = duration;
		debugRenderText.m_textStopwatch = new Stopwatch(duration);

		g_Font->AddVertsForText3D(debugRenderText.m_textVertexes, Vec2(0.f, 0.f), textHeight, text, startColor, 1.f, alignment);
		TransformVertexArray3D(debugRenderText.m_textVertexes, transform);
		g_debugRenderMutex.lock();
		g_theWorldText.push_back(debugRenderText);
		g_debugRenderMutex.unlock();
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		//DebugRenderXRay debugRenderXRay;
		//debugRenderXRay.m_startColor = startColor;
		//debugRenderXRay.m_endColor = endColor;
		//debugRenderXRay.m_xRayStopwatch = new Stopwatch(duration);

		//g_theXRayRenderVertexes.push_back(debugRenderXRay);
		break;
	}
	default:
		break;
	}

}

void DebugAddWorldBillboardText(std::string const& text, Vec3 const& origin, float textHeight, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode)
{
	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		//DebugRenderAlways debugRenderAlways;
		//debugRenderAlways.m_startColor = startColor;
		//debugRenderAlways.m_endColor = endColor;
		//debugRenderAlways.m_alwaysStopwatch = new Stopwatch(duration);

		//g_theAlwaysRenderVertexes.push_back(debugRenderAlways);
		//break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderText debugRenderText;
		debugRenderText.m_startColor = startColor;
		debugRenderText.m_endColor = endColor;
		debugRenderText.m_textStopwatch = new Stopwatch(duration);
		debugRenderText.origin = origin;

		g_Font->AddVertsForText3D(debugRenderText.m_textVertexes, Vec2(0.f, 0.f), textHeight, text, startColor, 1.f, alignment);

		g_debugRenderMutex.lock();
		g_theTextRenderVertexes.push_back(debugRenderText);
		g_debugRenderMutex.unlock();
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		//DebugRenderXRay debugRenderXRay;
		//debugRenderXRay.m_startColor = startColor;
		//debugRenderXRay.m_endColor = endColor;
		//debugRenderXRay.m_xRayStopwatch = new Stopwatch(duration);

		//g_theXRayRenderVertexes.push_back(debugRenderXRay);
		break;
	}
	default:
		break;
	}
}

void DebugAddScreenText(std::string const& text, Vec2 const& position, float size, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor)
{
	(void)position;
	DebugRenderScreenText debugRenderScreenText;
	debugRenderScreenText.m_startColor = startColor;
	debugRenderScreenText.m_endColor = endColor;
	debugRenderScreenText.m_duration = duration;
	debugRenderScreenText.m_textStopwatch = new Stopwatch(duration);
	
	std::vector<Vertex_PCU>& verts = debugRenderScreenText.m_screenTextVertexes;
	// g_theFont->AddVertsForText2D(verts, Vec2(), size, text, startColor, 1.f);
	g_Font->AddVertsForTextInBox2D(verts, AABB2(0.f, 0.f, 1600.f, 800.f), size, text, startColor, 1.f, alignment, TextDrawMode::OVERRUN);
	// AABB2 textBounds = GetVertexBounds2D(verts);
	// Vec2 textDim = textBounds.GetDimensions();
	// textDim.x *= alignment.x;
	// textDim.y *= alignment.y;
	// 
	// Mat44 translation;
	// translation.SetTranslation3D(Vec3(position.x - textDim.x, position.y - textDim.y));
	// 
	// TransformVertexArray3D(verts, translation);
	g_debugRenderMutex.lock();
	g_theScreenTextRenderVertexes.push_back(debugRenderScreenText);
	g_debugRenderMutex.unlock();
}
