#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <d3d11.h>

extern Renderer* g_theRenderer;

ConstantBuffer::ConstantBuffer(ID3D11Buffer* buffer, uint32_t size)
	: m_physicalRadius(size)
	, m_buffer(buffer)
{

}

ConstantBuffer::~ConstantBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}

ID3D11Buffer* ConstantBuffer::GetBuffer() const
{
	return m_buffer;
}

void ConstantBuffer::Update(const void* data, uint32_t byteSize)
{
	if (!m_buffer || !g_theRenderer || !g_theRenderer->GetDeviceContext() || !data)
		return;

	ID3D11DeviceContext* context = g_theRenderer->GetDeviceContext();

	const uint32_t copySize = Min(byteSize, m_physicalRadius);

	D3D11_BUFFER_DESC desc = {};
	m_buffer->GetDesc(&desc);

	if (desc.Usage == D3D11_USAGE_DYNAMIC && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE))
	{
		D3D11_MAPPED_SUBRESOURCE mapped = {};
		HRESULT hr = context->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		if (SUCCEEDED(hr))
		{
			std::memcpy(mapped.pData, data, copySize);
			context->Unmap(m_buffer, 0);
		}
	}
	else
	{
		context->UpdateSubresource(m_buffer, 0, nullptr, data, 0, 0);
	}
}

void ConstantBuffer::BindCS(int slot) const
{
	if (!m_buffer || !g_theRenderer || !g_theRenderer->GetDeviceContext())
		return;
	ID3D11DeviceContext* ctx = g_theRenderer->GetDeviceContext();
	ctx->CSSetConstantBuffers(slot, 1, &m_buffer);
}

void ConstantBuffer::BindVS(int slot) const
{
	if (!m_buffer || !g_theRenderer || !g_theRenderer->GetDeviceContext())
		return;
	ID3D11DeviceContext* ctx = g_theRenderer->GetDeviceContext();
	ctx->VSSetConstantBuffers(slot, 1, &m_buffer);
}

void ConstantBuffer::UnbindCS(int slot) const
{
	if (!g_theRenderer || !g_theRenderer->GetDeviceContext())
		return;
	ID3D11DeviceContext* ctx = g_theRenderer->GetDeviceContext();
	ID3D11Buffer* nullCB = nullptr;
	ctx->CSSetConstantBuffers(slot, 1, &nullCB);
}

void ConstantBuffer::UnbindVS(int slot) const
{
	if (!g_theRenderer || !g_theRenderer->GetDeviceContext())
		return;
	ID3D11DeviceContext* ctx = g_theRenderer->GetDeviceContext();
	ID3D11Buffer* nullCB = nullptr;
	ctx->VSSetConstantBuffers(slot, 1, &nullCB);
}