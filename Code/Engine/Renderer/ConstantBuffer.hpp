#pragma once
#include <cstdint>

struct ID3D11Buffer;

class ConstantBuffer
{
	friend class Renderer;

public:
	ConstantBuffer(ID3D11Buffer* buffer, uint32_t size);
	ConstantBuffer(const ConstantBuffer& copy) = delete;
	virtual ~ConstantBuffer();
	ID3D11Buffer* GetBuffer() const;
	void Update(const void* data, uint32_t byteSize);

	void BindCS(int slot) const;
	void BindVS(int slot) const;
	void UnbindCS(int slot) const;
	void UnbindVS(int slot) const;

private:
	ID3D11Buffer* m_buffer = nullptr;
	uint32_t m_physicalRadius = 0;
};