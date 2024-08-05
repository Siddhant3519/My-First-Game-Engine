#pragma once


//--------------------------------------------------------------------------------------------------
class VertexBuffer;
class IndexBuffer;
class CPUMesh;

//--------------------------------------------------------------------------------------------------
class GPUMesh
{
public:
	GPUMesh();
	~GPUMesh();

	void PopulateVertexAndIndexBuffersUsingCPUMesh(CPUMesh const* cpuMesh);
	void Render() const;

private:
	int m_indexCount = 0;

public:
	VertexBuffer*	m_gpuVertexBuffer	= nullptr;
	IndexBuffer*	m_gpuIndexBuffer	= nullptr;
};