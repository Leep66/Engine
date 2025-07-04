#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <d3d11.h>
#include <cassert>
#include <cstdlib>

IndexBuffer::IndexBuffer(ID3D11Device* device, unsigned int size, unsigned int stride)
	: m_device(device)
	, m_physicalRadius(size)
	, m_stride(stride)
{
	if (m_stride == 0)
	{
		m_stride = sizeof(unsigned int);
	}

	Create();
	
}

IndexBuffer::~IndexBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
	m_device = nullptr;
}

void IndexBuffer::Create()
{
	if (m_buffer)
	{
		DX_SAFE_RELEASE(m_buffer);
	}

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = m_physicalRadius * m_stride;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;


	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create index buffer.");
	}
}

void IndexBuffer::Resize(unsigned int size)
{
	m_physicalRadius = size;

	Create();
}

unsigned int IndexBuffer::GetSize()
{
	return m_physicalRadius;
}

unsigned int IndexBuffer::GetStride()
{
	return m_stride;
}

unsigned int IndexBuffer::GetCount()
{
	return m_physicalRadius / m_stride;
}
