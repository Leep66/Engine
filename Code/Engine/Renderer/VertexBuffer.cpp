#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <d3d11.h>
#include <cassert>
#include <cstdlib>
#include <iostream>

VertexBuffer::VertexBuffer(ID3D11Device* device, unsigned int size, unsigned int stride)
	: m_device(device)
	, m_physicalRadius(size)
	, m_stride(stride)
{

	Create();
}

VertexBuffer::~VertexBuffer() 
{
	DX_SAFE_RELEASE(m_buffer);
	m_device = nullptr;
	
}

void VertexBuffer::Create() 
{
	if (m_buffer)
    {
        DX_SAFE_RELEASE(m_buffer); 
    }

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = m_physicalRadius * m_stride;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_buffer);
	if (!SUCCEEDED(hr))
	{
		std::cerr << "Error creating vertex buffer. HRESULT: " << hr << std::endl;
		ERROR_AND_DIE("Could not create vertex buffer.");
	}
}

void VertexBuffer::Resize(unsigned int size) 
{
	m_physicalRadius = size;

	Create();
}

unsigned int VertexBuffer::GetSize() const
{
	return m_physicalRadius;
}

unsigned int VertexBuffer::GetStride() const
{
	return m_stride;
}
