#pragma once
#include <d3d11.h>

class RenderStateSaver 
{
private:
	ID3D11DeviceContext* m_ctx;
	ID3D11VertexShader* m_prevVS;
	ID3D11GeometryShader* m_prevGS;
	ID3D11PixelShader* m_prevPS;
	ID3D11InputLayout* m_prevLayout;
	D3D11_PRIMITIVE_TOPOLOGY m_prevTopology;

public:
	RenderStateSaver(ID3D11DeviceContext* ctx) : m_ctx(ctx) 
	{
		m_ctx->VSGetShader(&m_prevVS, nullptr, nullptr);
		m_ctx->GSGetShader(&m_prevGS, nullptr, nullptr);
		m_ctx->PSGetShader(&m_prevPS, nullptr, nullptr);
		m_ctx->IAGetInputLayout(&m_prevLayout);
		m_ctx->IAGetPrimitiveTopology(&m_prevTopology);
	}

	~RenderStateSaver() 
	{
		m_ctx->VSSetShader(m_prevVS, nullptr, 0);
		m_ctx->GSSetShader(m_prevGS, nullptr, 0);
		m_ctx->PSSetShader(m_prevPS, nullptr, 0);
		m_ctx->IASetInputLayout(m_prevLayout);
		m_ctx->IASetPrimitiveTopology(m_prevTopology);

		if (m_prevVS) m_prevVS->Release();
		if (m_prevGS) m_prevGS->Release();
		if (m_prevPS) m_prevPS->Release();
		if (m_prevLayout) m_prevLayout->Release();
	}
};