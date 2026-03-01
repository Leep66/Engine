#include "StructuredBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"

extern Renderer* g_theRenderer;

void StructuredBuffer::BindAsSRV(int slot) const
{
	if (m_srv && g_theRenderer && g_theRenderer->GetDeviceContext())
	{
		ID3D11DeviceContext* context = g_theRenderer->GetDeviceContext();
		context->VSSetShaderResources(slot, 1, &m_srv);
		context->PSSetShaderResources(slot, 1, &m_srv); 
		context->CSSetShaderResources(slot, 1, &m_srv);
	}
}




void StructuredBuffer::BindAsUAV(int slot) const
{
	if (m_uav && g_theRenderer && g_theRenderer->GetDeviceContext())
	{
		ID3D11DeviceContext* context = g_theRenderer->GetDeviceContext();
		UINT initialCount = 0;
		context->CSSetUnorderedAccessViews(slot, 1, &m_uav, &initialCount);
	}
}

void StructuredBuffer::UnbindAsSRV(int slot) const
{
	if (g_theRenderer && g_theRenderer->GetDeviceContext())
	{
		ID3D11DeviceContext* context = g_theRenderer->GetDeviceContext();
		ID3D11ShaderResourceView* nullSRV = nullptr;

		context->VSSetShaderResources(slot, 1, &nullSRV);
		context->PSSetShaderResources(slot, 1, &nullSRV);
		context->CSSetShaderResources(slot, 1, &nullSRV);
	}
}


void StructuredBuffer::UnbindAsUAV(int slot) const
{
	if (g_theRenderer && g_theRenderer->GetDeviceContext())
	{
		ID3D11DeviceContext* context = g_theRenderer->GetDeviceContext();
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		context->CSSetUnorderedAccessViews(slot, 1, &nullUAV, nullptr);
	}
}

void StructuredBuffer::Update(const void* data, uint32_t count)
{
	if (!m_buffer || !g_theRenderer || !g_theRenderer->GetDeviceContext())
		return;

	ID3D11DeviceContext* context = g_theRenderer->GetDeviceContext();

	uint32_t copySize = Min(count, m_count) * m_stride;

	D3D11_BUFFER_DESC desc;
	m_buffer->GetDesc(&desc);

	if (desc.Usage == D3D11_USAGE_DYNAMIC && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE))
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		HRESULT hr = context->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped.pData, data, copySize);
			context->Unmap(m_buffer, 0);
		}
	}
	else
	{
		context->UpdateSubresource(m_buffer, 0, nullptr, data, m_stride, m_stride * count);
	}
}
