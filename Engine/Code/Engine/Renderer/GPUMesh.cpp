#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"


//--------------------------------------------------------------------------------------------------
extern Renderer* g_theRenderer;


//--------------------------------------------------------------------------------------------------
GPUMesh::GPUMesh()
{
	m_gpuVertexBuffer	=	g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCUTBN), (unsigned int)sizeof(Vertex_PCUTBN));
	m_gpuIndexBuffer	=	g_theRenderer->CreateIndexBuffer(sizeof(Vertex_PCUTBN));
}


//--------------------------------------------------------------------------------------------------
GPUMesh::~GPUMesh()
{
	delete m_gpuVertexBuffer;
	m_gpuVertexBuffer = nullptr;

	delete m_gpuIndexBuffer;
	m_gpuIndexBuffer = nullptr;
}

//--------------------------------------------------------------------------------------------------
void GPUMesh::PopulateVertexAndIndexBuffersUsingCPUMesh(CPUMesh const* cpuMesh)
{
	g_theRenderer->CopyCPUToGPU(cpuMesh->m_cpuVerts.data(),		(size_t)m_gpuVertexBuffer->GetStride()	*  cpuMesh->m_cpuVerts.size(),		m_gpuVertexBuffer);
	g_theRenderer->CopyCPUToGPU(cpuMesh->m_cpuIndexes.data(),	(size_t)m_gpuIndexBuffer->GetStride()	*  cpuMesh->m_cpuIndexes.size(),	m_gpuIndexBuffer);
	m_indexCount = (int)cpuMesh->m_cpuIndexes.size();
}


//--------------------------------------------------------------------------------------------------
void GPUMesh::Render() const
{
	g_theRenderer->DrawVertexAndIndexBuffer(m_gpuVertexBuffer, m_gpuIndexBuffer, m_indexCount);
}
