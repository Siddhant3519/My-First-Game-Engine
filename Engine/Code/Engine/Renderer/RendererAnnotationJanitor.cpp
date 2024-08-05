#include "Engine/Renderer/RendererAnnotationJanitor.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"


//--------------------------------------------------------------------------------------------------
extern Renderer* g_theRenderer;


//--------------------------------------------------------------------------------------------------
RendererAnnotationJanitor::RendererAnnotationJanitor(wchar_t const* annotationText)
{
	GUARANTEE_OR_DIE(g_theRenderer != nullptr, "Please create a renderer before calling the renderer annotation janitor");
	g_theRenderer->BeginAnnotationEvent(annotationText);
}


//--------------------------------------------------------------------------------------------------
RendererAnnotationJanitor::~RendererAnnotationJanitor()
{
	GUARANTEE_OR_DIE(g_theRenderer != nullptr, "Please create a renderer before calling the renderer annotation janitor");
	g_theRenderer->EndAnnotationEvent();
}
