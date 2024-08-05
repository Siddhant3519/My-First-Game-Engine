#pragma once

struct ID3D11Buffer;

class IndexBuffer
{
	friend class Renderer;

public:
	IndexBuffer(size_t size);
	IndexBuffer(IndexBuffer const& copy) = delete;
	virtual ~IndexBuffer();

	unsigned int GetStride() const;

public:
	ID3D11Buffer* m_buffer = nullptr;
	size_t m_size = 0;
};