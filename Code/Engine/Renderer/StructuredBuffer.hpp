
#pragma once
#include <d3d11.h>
#include "Engine/Renderer/Renderer.hpp"
#include <cstdint>

class StructuredBuffer {
public:

	StructuredBuffer(ID3D11Buffer* buffer,
		ID3D11ShaderResourceView* srv,
		ID3D11UnorderedAccessView* uav,
		unsigned int stride,
		unsigned int elementCount)
		: m_buffer(buffer)
		, m_srv(srv)
		, m_uav(uav)
		, m_stride(stride)
		, m_count(elementCount)
	{
		if (m_buffer) m_buffer->AddRef();
		if (m_srv)    m_srv->AddRef();
		if (m_uav)    m_uav->AddRef();
	}

	~StructuredBuffer() 
	{
		DX_SAFE_RELEASE(m_uav);
		DX_SAFE_RELEASE(m_srv);
		DX_SAFE_RELEASE(m_buffer);
	}

	ID3D11Buffer* GetBuffer() const { return m_buffer; }
	ID3D11ShaderResourceView* GetSRV()    const { return m_srv; }
	ID3D11UnorderedAccessView* GetUAV()    const { return m_uav; }

	unsigned int GetStride() const { return m_stride; }
	unsigned int GetCount()  const { return m_count; }

	void BindAsSRV(int slot) const;
	void BindAsUAV(int slot) const;
	void UnbindAsSRV(int slot) const;
	void UnbindAsUAV(int slot) const;
	void Update(const void* data, uint32_t count);

private:
	ID3D11Buffer* m_buffer = nullptr;
	ID3D11ShaderResourceView* m_srv = nullptr;
	ID3D11UnorderedAccessView* m_uav = nullptr;
	unsigned int m_stride = 0;
	unsigned int m_count = 0;
};
