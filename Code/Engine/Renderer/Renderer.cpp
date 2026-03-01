#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "ThirdParty/stb/stb_image.h"
#include "Game/EngineBuildPreferences.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"



#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#if defined(ENGINE_DEBUG_RENDER)
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif



ID3D11Device* m_device = nullptr;
ID3D11DeviceContext* m_deviceContext = nullptr;
IDXGISwapChain* m_swapChain = nullptr;
ID3D11RenderTargetView* m_renderTargetView = nullptr;
ID3D11RasterizerState* m_rasterizerState = nullptr;

std::vector<uint8_t> m_vertexShaderByteCode;
std::vector<uint8_t> m_pixelShaderByteCode;

#if defined(ENGINE_DEBUG_RENDER)
void* m_dxgiDebug = nullptr;
void* m_dxgiDebugModule = nullptr;
#endif

std::string shaderSource = DefaultShader::SHADER_SOURCE;

//HDC g_displayDeviceContext = nullptr;
//HGLRC g_openGLRenderingContext = nullptr;



struct LightConstants
{
	Vec4 SunColor = Vec4(1.f, 1.f, 1.f, 1.f);
	Vec3 SunDirection = Vec3::ZERO;
	int NumLights = 0;
	Light LightsArray[MAX_LIGHTS];
};

static const int k_lightConstantsSlot = 4;

struct PerFrameConstants
{
	float Time = 0.f;
	int DebugInt = 0;
	float DebugFloat = 0.f;
	float padding = 0.f;
};

static const int k_perframeConstantsSlot = 1;

struct CameraConstants
{
	Mat44 WorldToCameraTransform;
	Mat44 CameraToRenderTransform;
	Mat44 RenderToClipTransform;
	Vec3  CameraPosition;
	float EMPTY_PADDING = 0.f;
	
};
static const int k_cameraConstantsSlot = 2;

struct ModelConstants
{
	Mat44 ModelToWorldTransform;
	float ModelColor[4] = { 0.f };
};
static const int k_modelConstantsSlot = 3;

struct SpecialEffectConstants
{
	int SpecialEffect = 0;
	int EMPTY_PADDING[3] = { };
};
static const int k_specialSlot = 8;


static const char* HrName(HRESULT hr) {
	switch (hr) {
	case DXGI_ERROR_DEVICE_REMOVED:         return "DXGI_ERROR_DEVICE_REMOVED";
	case DXGI_ERROR_DEVICE_RESET:           return "DXGI_ERROR_DEVICE_RESET";
	case DXGI_ERROR_DEVICE_HUNG:            return "DXGI_ERROR_DEVICE_HUNG";
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR:  return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
	default:                                return "UNKNOWN_HRESULT";
	}
}

Renderer::Renderer(RendererConfig const& config)
	: m_config(config)
{
}

Renderer::~Renderer()
{
}

static int ComputeMipLevels(int width, int height, int requestedLevels)
{
	int maxDim = (width > height) ? width : height;

	int fullLevels = 1; 
	while (maxDim > 1) {
		maxDim >>= 1;
		++fullLevels;
	}

	if (requestedLevels <= 0 || requestedLevels > fullLevels) {
		return fullLevels;
	}

	return requestedLevels;
}


void Renderer::Startup()
{
	// CreateRenderingContext(); 

	unsigned int deviceFlags = 0;
#if defined(ENGINE_DEBUG_RENDER)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

#if defined(ENGINE_DEBUG_RENDER)
	m_dxgiDebugModule = (void*)::LoadLibraryA("dxgidebug.dll");
	if (m_dxgiDebugModule == nullptr)
	{
		ERROR_AND_DIE("Could not load dxgidebug.dll.");
	}

	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	((GetDebugModuleCB)::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))
		(__uuidof(IDXGIDebug), &m_dxgiDebug);

	if (m_dxgiDebug == nullptr)
	{
		ERROR_AND_DIE("Could not load debug module.");
	}
#endif


	DXGI_SWAP_CHAIN_DESC swapChainDesc = { };
	swapChainDesc.BufferDesc.Width = m_config.m_window->GetClientDimensions().x;
	swapChainDesc.BufferDesc.Height = m_config.m_window->GetClientDimensions().y;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = (HWND)m_config.m_window->GetHWND();
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags,
		nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc,
		&m_swapChain, &m_device, nullptr, &m_deviceContext);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create D3D 11 device and swap chain.");
	}

	ID3D11Texture2D* backBuffer;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not get swap chain buffer.");
	}

	hr = m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could create render target view for swap chain buffer.");
	}

	DX_SAFE_RELEASE(backBuffer);

	UINT initialVertexBufferPCUSize = sizeof(Vertex_PCU);
	m_immediatePCUVBO = CreateVertexBuffer(initialVertexBufferPCUSize, sizeof(Vertex_PCU));

	UINT initialVertexBufferPCUTBNSize = sizeof(Vertex_PCUTBN);
	m_immediatePCUTBNVBO = CreateVertexBuffer(initialVertexBufferPCUTBNSize, sizeof(Vertex_PCUTBN));

	UINT initialIndexBufferSize = sizeof(unsigned int);
	m_immediateIBO = CreateIndexBuffer(initialIndexBufferSize, sizeof(unsigned int));

#if defined (LIGHT_CONSTANTS)
	CreateLightCBO();
#endif

	m_cameraCBO = CreateConstantBuffer(sizeof(CameraConstants));
	m_modelCBO = CreateConstantBuffer(sizeof(ModelConstants));



#if defined (PERFRAME_CONSTANTS)
	m_perframeCBO = CreateConstantBuffer(sizeof(PerFrameConstants));
#endif

#if defined(SPECIAL_EFFECT_CONSTANTS)
	m_specialCBO = CreateConstantBuffer(sizeof(SpecialEffectConstants));
#endif

	m_defaultShader = CreateShader("Data/Shaders/Default");
	m_currentShader = m_defaultShader;
	BindShader(m_currentShader);

	CreateBlendStates();
	CreateSamplerStates();

	float width = (float)m_config.m_window->GetClientDimensions().x;
	float height = (float)m_config.m_window->GetClientDimensions().y;

	ViewportData data;
	data.TopLeftX = 0.f;
	data.TopLeftY = 0.f;
	data.Width = width;
	data.Height = height;

	SetViewport(data);


	/*m_deviceContext->RSSetViewports(1, viewports);*/

	CreateRasterizerMode();

	CreateDepthMode();
	
	Image defaultImg(IntVec2(2, 2), Rgba8::WHITE);
	Texture* defaultTexture = CreateTextureFromImage(defaultImg);
	m_defaultTexture = defaultTexture;
	BindTexture(m_defaultTexture);

	CreateBackbufferRTV();                         
	CreateSceneDepth(IntVec2((int)width, (int)height), 1);
}

void Renderer::BeginFrame()
{
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilDSV);
	m_deviceContext->ClearDepthStencilView(m_depthStencilDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
}

void Renderer::EndFrame()
{
	HRESULT hr = m_swapChain->Present(1, 0);
	if (FAILED(hr)) {
		HRESULT reason = m_device->GetDeviceRemovedReason();
			DebugAddMessage(Stringf("Present failed: %s (0x%08X), removed reason: %s (0x%08X)",
			HrName(hr), (unsigned)hr, HrName(reason), (unsigned)reason), 1.f, Rgba8::RED);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) 
		{
			ERROR_AND_DIE("Device has been lost, application will now terminate.");
		}
	}

}

void Renderer::Shutdown()
{
	
	for (int i = 0; i < (int)DepthMode::COUNT; ++i)
	{
		DX_SAFE_RELEASE(m_depthStencilStates[i]);
	}

	DX_SAFE_RELEASE(m_depthStencilDSV);
	DX_SAFE_RELEASE(m_depthStencilTexture);

	for (int i = 0; i < (int)RasterizerMode::COUNT; ++i)
	{
		DX_SAFE_RELEASE(m_rasterizerStates[i]);
	}

	for (int i = 0; i < (int)BlendMode::COUNT; ++i)
	{
		DX_SAFE_RELEASE(m_blendStates[i]);
	}

	for (int i = 0; i < (int)SamplerMode::COUNT; ++i)
	{
		DX_SAFE_RELEASE(m_samplerStates[i]);
	}

	SAFE_DELETE(m_immediatePCUVBO);
	SAFE_DELETE(m_immediateIBO);
	SAFE_DELETE(m_immediatePCUTBNVBO);

	SAFE_DELETE(m_cameraCBO);
	SAFE_DELETE(m_modelCBO);
	SAFE_DELETE(m_lightCBO);
	SAFE_DELETE(m_perframeCBO);
	SAFE_DELETE(m_specialCBO); 

	DX_SAFE_RELEASE(m_sceneDepthCopySRV);
	DX_SAFE_RELEASE(m_sceneDepthCopyTex);
	DX_SAFE_RELEASE(m_sceneDepthSRV);
	DX_SAFE_RELEASE(m_sceneDepthDSV);
	DX_SAFE_RELEASE(m_sceneDepthDSV_ReadOnly);
	DX_SAFE_RELEASE(m_sceneDepthTex);

	DX_SAFE_RELEASE(m_backbufferRTV);
	DX_SAFE_RELEASE(m_backbufferTex);

	for (Shader* shader : m_loadedShaders)
	{
		SAFE_DELETE(shader);
	}
	m_loadedShaders.clear();

	m_currentShader = nullptr;

	for (Texture* texture : m_loadedTextures)
	{
		SAFE_DELETE(texture);
	}
	m_loadedTextures.clear();


	for (BitmapFont* font : m_loadedFonts)
	{
		SAFE_DELETE(font);
	}
	m_loadedFonts.clear();

	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_deviceContext);
	DX_SAFE_RELEASE(m_device);

	m_blendState = nullptr;
	m_samplerState = nullptr;
	m_rasterizerState = nullptr;
	m_depthStencilTexture = nullptr;
	m_depthStencilState = nullptr;
	m_currentCamera = nullptr;

#if defined(ENGINE_DEBUG_RENDER)
	if (m_dxgiDebug) {
		((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(
			DXGI_DEBUG_ALL,
			(DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
		);
		((IDXGIDebug*)m_dxgiDebug)->Release();
		m_dxgiDebug = nullptr;
	}

	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif
}


ID3D11Device* Renderer::GetDevice() const
{
	return m_device;
}

ID3D11DeviceContext* Renderer::GetDeviceContext() const
{
	return m_deviceContext;
}

void Renderer::ClearScreen(const Rgba8& clearColor) const
{
	float colorAsFloats[4];
	clearColor.GetAsFloats(colorAsFloats);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, colorAsFloats);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::BeginCamera(const Camera& camera) const
{
	SetCameraConstants(camera);
	SetModelConstants();
	m_cameraStack.push_back(&camera);
	m_currentCamera = &camera;
}

void Renderer::EndCamera(const Camera& camera) const
{
	UNUSED(camera);
	if (!m_cameraStack.empty()) m_cameraStack.pop_back();
	m_currentCamera = m_cameraStack.empty() ? nullptr : m_cameraStack.back();
}

void Renderer::DrawVertexArray(std::vector<Vertex_PCU> const& verts)
{
	DrawVertexArray(static_cast<int>(verts.size()), verts.data());
}

void Renderer::DrawVertexArray(std::vector<Vertex_PCUTBN> const& verts)
{
	int numVertexes = static_cast<int>(verts.size());
	if (numVertexes <= 0) return;

	std::vector<unsigned int> indices(numVertexes);
	for (int i = 0; i < numVertexes; ++i) {
		indices[i] = i;
	}

	int numIndices = static_cast<int>(indices.size());

	CopyCPUToGPU(m_immediatePCUTBNVBO, m_immediateIBO, verts.data(), indices.data(), numVertexes, numIndices);
	DrawIndexedVertexBuffer(m_immediatePCUTBNVBO, m_immediateIBO, numIndices);
}
void Renderer::DrawVertexArray(std::vector<Vertex_PCUTBN> const& verts, std::vector<unsigned int> indexes)
{
	int numVertexes = static_cast<int>(verts.size());
	int numIndices = static_cast<int>(indexes.size());

	if (numVertexes <= 0 || numIndices <= 0) return;

	CopyCPUToGPU(m_immediatePCUTBNVBO, m_immediateIBO, verts.data(), indexes.data(), numVertexes, numIndices);
	DrawIndexedVertexBuffer(m_immediatePCUTBNVBO, m_immediateIBO, numIndices);
}
void Renderer::DrawVertexArray(int numVertexes, Vertex_PCU const* verts)
{
	std::vector<unsigned int> indices(numVertexes);
	for (int i = 0; i < numVertexes; ++i) {
		indices[i] = i;
	}

	CopyCPUToGPU(m_immediatePCUVBO, m_immediateIBO, verts, indices.data(), numVertexes, numVertexes);
	DrawIndexedVertexBuffer(m_immediatePCUVBO, m_immediateIBO, numVertexes);
}


void Renderer::DrawArray(int vertexCount)
{
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_deviceContext->Draw(vertexCount, 0);
}

D3D11_VIEWPORT Renderer::GetViewport(ViewportData viewportData) const
{
	float x = viewportData.TopLeftX;
	float y = viewportData.TopLeftY;
	float width = viewportData.Width;
	float height = viewportData.Height;

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = x;
	viewport.TopLeftY = y;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	return viewport;
}

void Renderer::SetViewport(ViewportData viewportData)
{

	D3D11_VIEWPORT vp = GetViewport(viewportData);
 	m_deviceContext->RSSetViewports(1, &vp);
}


Image* Renderer::CreateImageFromFile(char const* imageFilePath)
{
	if (!imageFilePath)
	{
		return nullptr;
	}

	Image* image = new Image(imageFilePath);

	if (image->GetDimensions().x == 0 || image->GetDimensions().y == 0)
	{
		delete image;
		return nullptr;
	}

	return image;
}


Texture* Renderer::CreateTextureFromImage(const Image& image,
                                          bool generateMipmaps /*= false*/,
                                          int requestedMipLevels /*= 0*/)
{
	if (image.GetDimensions().x == 0 || image.GetDimensions().y == 0) {
		return nullptr;
	}

	for (Texture* existingTexture : m_loadedTextures)
	{
		if (existingTexture->GetImageFilePath() == image.GetImageFilePath())
		{
			return existingTexture;
		}
	}

	const IntVec2 dims = image.GetDimensions();
	const int width  = dims.x;
	const int height = dims.y;

	Texture* newTexture = new Texture();
	newTexture->m_dimensions = dims;

	HRESULT hr = S_OK;

	if (!generateMipmaps) 
	{
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width              = width;
		textureDesc.Height             = height;
		textureDesc.MipLevels          = 1;
		textureDesc.ArraySize          = 1;
		textureDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count   = 1;
		textureDesc.Usage              = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags     = 0;
		textureDesc.MiscFlags          = 0;

		D3D11_SUBRESOURCE_DATA textureData = {};
		textureData.pSysMem     = image.GetRawData();
		textureData.SysMemPitch = 4 * width;
		textureData.SysMemSlicePitch = 0;

		hr = m_device->CreateTexture2D(&textureDesc, &textureData, &newTexture->m_texture);
		if (!SUCCEEDED(hr))
		{
			delete newTexture;
			ERROR_AND_DIE(Stringf("CreateTextureFromImage failed for image file \"%s\".",
				image.GetImageFilePath().c_str()));
			return nullptr;
		}

		hr = m_device->CreateShaderResourceView(newTexture->m_texture, nullptr,
			&newTexture->m_shaderResourceView);
		if (!SUCCEEDED(hr))
		{
			DX_SAFE_RELEASE(newTexture->m_texture);
			delete newTexture;
			ERROR_AND_DIE(Stringf("CreateShaderResourceView failed for image file \"%s\".",
				image.GetImageFilePath().c_str()));
			return nullptr;
		}
	}
	else
	{
		const int mipLevels = ComputeMipLevels(width, height, requestedMipLevels);

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width              = width;
		texDesc.Height             = height;
		texDesc.MipLevels          = mipLevels;
		texDesc.ArraySize          = 1;
		texDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count   = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage              = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags     = 0;
		texDesc.MiscFlags          = 0;

		if (mipLevels > 1) 
		{
			texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			texDesc.MiscFlags  = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		hr = m_device->CreateTexture2D(&texDesc, nullptr, &newTexture->m_texture);
		if (FAILED(hr)) 
		{
			delete newTexture;
			ERROR_AND_DIE(Stringf("CreateTexture2D (mip) failed for image file \"%s\".",
				image.GetImageFilePath().c_str()));
			return nullptr;
		}

		m_deviceContext->UpdateSubresource(newTexture->m_texture,
			0, nullptr, image.GetRawData(), width * 4, 0);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format                    = texDesc.Format;
		srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels       = mipLevels;

		hr = m_device->CreateShaderResourceView(newTexture->m_texture,
			&srvDesc, &newTexture->m_shaderResourceView);
		if (FAILED(hr)) 
		{
			DX_SAFE_RELEASE(newTexture->m_texture);
			delete newTexture;
			ERROR_AND_DIE(Stringf("CreateShaderResourceView (mip) failed for image file \"%s\".",
				image.GetImageFilePath().c_str()));
			return nullptr;
		}

		if (mipLevels > 1) 
		{
			m_deviceContext->GenerateMips(newTexture->m_shaderResourceView);
		}
	}

	m_loadedTextures.push_back(newTexture);
	return newTexture;
}




Texture* Renderer::GetTextureForFile(char const* imageFilePath)
{
	for (Texture* texture : m_loadedTextures)
	{
		if (texture->GetImageFilePath() == imageFilePath)
		{
			return texture;
		}
	}
	return nullptr;
}

Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath, bool generateMipmaps,int requestedMipLevels)
{
	Texture* existingTexture = GetTextureForFile(imageFilePath);
	if (existingTexture)
	{
		return existingTexture;
	}

	Texture* newTexture = CreateTextureFromFile(imageFilePath,
		generateMipmaps,
		requestedMipLevels);
	newTexture->m_name = imageFilePath;
	newTexture->m_dimensions = newTexture->GetDimensions();
	return newTexture;
}

Texture* Renderer::CreateTextureFromFile(char const* imageFilePath,
	bool generateMipmaps /*= true*/,
	int requestedMipLevels /*= 0*/)
{
	Image* newImage = CreateImageFromFile(imageFilePath);
	if (!newImage) return nullptr;

	const IntVec2 dims = newImage->GetDimensions();
	const int w = dims.x;
	const int h = dims.y;

	if (!generateMipmaps) 
	{
		Texture* newTexture = CreateTextureFromImage(*newImage, false, 0);
		newTexture->m_name = imageFilePath;
		delete newImage;
		return newTexture;
	}

	int mipLevelsRequest = requestedMipLevels;
	if (mipLevelsRequest <= 0)
	{
		if (w == 32 && h == 32) 
		{
			mipLevelsRequest = 5;
		}
		else if (w == 64 && h == 64) 
		{
			mipLevelsRequest = 6;
		}
		else 
		{
			mipLevelsRequest = 0; 
		}
	}

	Texture* newTexture = CreateTextureFromImage(*newImage,
		true,
		mipLevelsRequest); 

	newTexture->m_name = imageFilePath;
	delete newImage;
	return newTexture;
}



Texture* Renderer::CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData)
{
	GUARANTEE_OR_DIE(texelData, Stringf("CreateTextureFromData failed for \"%s\" - texelData was null!", name));
	GUARANTEE_OR_DIE(bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf("CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel));
	GUARANTEE_OR_DIE(dimensions.x > 0 && dimensions.y > 0, Stringf("CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y));

	Texture* newTexture = new Texture();
	newTexture->m_name = name;
	newTexture->m_dimensions = dimensions;

	return newTexture;
}

BitmapFont* Renderer::CreateOrGetBitmapFont(char const* bitmapFontFilePath)
{
	for (BitmapFont* font : m_loadedFonts)
	{
		if (font != nullptr && !strcmp(font->m_fontFilePathNameWithNoExtension.c_str(), bitmapFontFilePath))
		{
			return font;
		}
	}

	BitmapFont* newFont = CreateBitmapFontFromFile(bitmapFontFilePath);
	if (newFont != nullptr)
	{
		m_loadedFonts.push_back(newFont);
	}

	return newFont;
}

BitmapFont* Renderer::CreateBitmapFontFromFile(char const* bitmapFontFilePath)
{
	std::string fontTexturePath = std::string(bitmapFontFilePath) + ".png";
	Texture* fontTexture = CreateOrGetTextureFromFile(fontTexturePath.c_str());

	if (fontTexture == nullptr)
	{
		return nullptr;
	}

	BitmapFont* newFont = new BitmapFont(bitmapFontFilePath, *fontTexture);

	return newFont;
}

//-----------------------------------------------------------------------------------------------
void Renderer::BindTexture(const Texture* texture, int slot /*= 0 */)
{
	if (texture == nullptr)
	{
		m_deviceContext->PSSetShaderResources(slot, 1, &m_defaultTexture->m_shaderResourceView);
	}
	else
	{
		m_deviceContext->PSSetShaderResources(slot, 1, &texture->m_shaderResourceView);
	}
	m_deviceContext->PSSetSamplers(slot, 1, &m_samplerState);
}


void Renderer::CreateRenderingContext()
{
	
}



Shader* Renderer::CreateShader(const char* shaderName, const char* source, VertexType vertexType)
{
	ShaderConfig config;
	config.m_name = shaderName;
	Shader* shader = new Shader(config);

	bool hasVertexShader = false;
	bool hasPixelShader = false;
	bool hasComputeShader = false;

	std::vector<unsigned char> vertexShaderByteCode;
	if (CompileShaderToByteCode(vertexShaderByteCode, shaderName, source, "VertexMain", "vs_5_0"))
	{
		HRESULT hr = m_device->CreateVertexShader(
			vertexShaderByteCode.data(),
			vertexShaderByteCode.size(),
			nullptr, &shader->m_vertexShader);
		if (SUCCEEDED(hr))
		{
			hasVertexShader = true;
		}
		else
		{
			DebuggerPrintf("Warning: Failed to create vertex shader for %s", shaderName);
		}
	}

	std::vector<unsigned char> pixelShaderByteCode;
	if (CompileShaderToByteCode(pixelShaderByteCode, shaderName, source, "PixelMain", "ps_5_0"))
	{
		HRESULT hr = m_device->CreatePixelShader(
			pixelShaderByteCode.data(),
			pixelShaderByteCode.size(),
			nullptr, &shader->m_pixelShader);
		if (SUCCEEDED(hr))
		{
			hasPixelShader = true;
		}
		else
		{
			DebuggerPrintf("Warning: Failed to create pixel shader for %s", shaderName);
		}
	}

	std::vector<unsigned char> computeShaderByteCode;
	if (CompileShaderToByteCode(computeShaderByteCode, shaderName, source, "CSMain", "cs_5_0"))
	{
		HRESULT hr = m_device->CreateComputeShader(
			computeShaderByteCode.data(),
			computeShaderByteCode.size(),
			nullptr, &shader->m_computeShader);
		if (SUCCEEDED(hr))
		{
			hasComputeShader = true;
			DebuggerPrintf("Successfully created compute shader for %s", shaderName);
		}
		else
		{
			DebuggerPrintf("Warning: Failed to create compute shader for %s", shaderName);
		}
	}

	if (!hasVertexShader && !hasPixelShader && !hasComputeShader)
	{
		delete shader;
		ERROR_AND_DIE(Stringf("Failed to create any shader for %s", shaderName));
		return nullptr;
	}

	if (hasVertexShader && vertexShaderByteCode.size() > 0)
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDescPCU[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		D3D11_INPUT_ELEMENT_DESC inputElementDescPCUTBN[] = {
			{"POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR",      0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BITANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		const D3D11_INPUT_ELEMENT_DESC* inputLayoutDesc = nullptr;
		UINT numElements = 0;

		if (vertexType == VertexType::Vertex_PCUTBN)
		{
			inputLayoutDesc = inputElementDescPCUTBN;
			numElements = ARRAYSIZE(inputElementDescPCUTBN);
		}
		else
		{
			inputLayoutDesc = inputElementDescPCU;
			numElements = ARRAYSIZE(inputElementDescPCU);
		}

		HRESULT hr = m_device->CreateInputLayout(
			inputLayoutDesc, numElements,
			vertexShaderByteCode.data(),
			vertexShaderByteCode.size(),
			&shader->m_inputLayout);

		if (!SUCCEEDED(hr))
		{
			DebuggerPrintf("Warning: Failed to create input layout for %s", shaderName);
		}
	}

	m_loadedShaders.push_back(shader);
	return shader;
}


Shader* Renderer::CreateShader(char const* shaderName, VertexType vertexType)
{
	std::string filename = std::string(shaderName) + ".hlsl";


	int result = FileReadToString(shaderSource, filename);
	if (result == -1) {
		ERROR_AND_DIE(Stringf("Failed to read shader file: %s", filename.c_str()));
		return nullptr;
	}

	Shader* shader = CreateShader(shaderName, shaderSource.c_str(), vertexType);
	if (!shader) {
		ERROR_AND_DIE(Stringf("Failed to create shader from file: %s", filename.c_str()));
		return nullptr;
	}

	return shader;
}

Shader* Renderer::CreateShader(char const* shaderName, VertexType vertexType, char const* vertexEntryPoint)
{
	std::string filename = std::string(shaderName) + ".hlsl";
	std::string source;

	int result = FileReadToString(source, filename);
	if (result == -1) {
		ERROR_AND_DIE(Stringf("Failed to read shader file: %s", filename.c_str()));
		return nullptr;
	}

	return CreateShaderWithEntryPoint(shaderName, source.c_str(), vertexType, vertexEntryPoint);
}

Shader* Renderer::CreateShaderWithEntryPoint(const char* shaderName, const char* source, VertexType vertexType, const char* vertexEntryPoint)
{
	ShaderConfig config;
	config.m_name = shaderName;
	Shader* shader = new Shader(config);

	bool hasVertexShader = false;
	bool hasPixelShader = false;
	bool hasComputeShader = false;

	std::vector<unsigned char> vertexShaderByteCode;

	const char* vsEntryPoint = vertexEntryPoint ? vertexEntryPoint : "VertexMain";

	if (CompileShaderToByteCode(vertexShaderByteCode, shaderName, source, vsEntryPoint, "vs_5_0"))
	{
		HRESULT hr = m_device->CreateVertexShader(
			vertexShaderByteCode.data(),
			vertexShaderByteCode.size(),
			nullptr, &shader->m_vertexShader);
		if (SUCCEEDED(hr))
		{
			hasVertexShader = true;
		}
		else
		{
			DebuggerPrintf("Warning: Failed to create vertex shader for %s with entry point %s", shaderName, vsEntryPoint);
		}
	}

	std::vector<unsigned char> pixelShaderByteCode;
	if (CompileShaderToByteCode(pixelShaderByteCode, shaderName, source, "PixelMain", "ps_5_0"))
	{
		HRESULT hr = m_device->CreatePixelShader(
			pixelShaderByteCode.data(),
			pixelShaderByteCode.size(),
			nullptr, &shader->m_pixelShader);
		if (SUCCEEDED(hr))
		{
			hasPixelShader = true;
		}
		else
		{
			DebuggerPrintf("Warning: Failed to create pixel shader for %s", shaderName);
		}
	}

	std::vector<unsigned char> computeShaderByteCode;
	if (CompileShaderToByteCode(computeShaderByteCode, shaderName, source, "CSMain", "cs_5_0"))
	{
		HRESULT hr = m_device->CreateComputeShader(
			computeShaderByteCode.data(),
			computeShaderByteCode.size(),
			nullptr, &shader->m_computeShader);
		if (SUCCEEDED(hr))
		{
			hasComputeShader = true;
			DebuggerPrintf("Successfully created compute shader for %s", shaderName);
		}
		else
		{
			DebuggerPrintf("Warning: Failed to create compute shader for %s", shaderName);
		}
	}

	if (!hasVertexShader && !hasPixelShader && !hasComputeShader)
	{
		delete shader;
		ERROR_AND_DIE(Stringf("Failed to create any shader for %s with entry point %s", shaderName, vsEntryPoint));
		return nullptr;
	}

	if (hasVertexShader && vertexShaderByteCode.size() > 0)
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDescPCU[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		D3D11_INPUT_ELEMENT_DESC inputElementDescPCUTBN[] = {
			{"POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR",      0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BITANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		const D3D11_INPUT_ELEMENT_DESC* inputLayoutDesc = nullptr;
		UINT numElements = 0;

		if (vertexType == VertexType::Vertex_PCUTBN)
		{
			inputLayoutDesc = inputElementDescPCUTBN;
			numElements = ARRAYSIZE(inputElementDescPCUTBN);
		}
		else if (vertexType == VertexType::Vertex_None)
		{
			inputLayoutDesc = nullptr;
			numElements = 0;
		}

		else
		{
			inputLayoutDesc = inputElementDescPCU;
			numElements = ARRAYSIZE(inputElementDescPCU);
		}

		HRESULT hr = m_device->CreateInputLayout(
			inputLayoutDesc, numElements,
			vertexShaderByteCode.data(),
			vertexShaderByteCode.size(),
			&shader->m_inputLayout);

		if (!SUCCEEDED(hr))
		{
			DebuggerPrintf("Warning: Failed to create input layout for %s with entry point %s", shaderName, vsEntryPoint);
		}
	}

	m_loadedShaders.push_back(shader);
	return shader;
}

Shader* Renderer::CreateComputeShader(char const* shaderName,
	char const* source,
	char const* entryPoint /*= "CSMain"*/)
{
	std::vector<unsigned char> byteCode;
	bool ok = CompileShaderToByteCode(byteCode,
		shaderName,
		source,
		entryPoint,
		"cs_5_0");
	if (!ok)
	{
		ERROR_AND_DIE(Stringf("Compute shader compilation failed: %s", shaderName));
		return nullptr;
	}

	ID3D11ComputeShader* cs = nullptr;
	HRESULT hr = m_device->CreateComputeShader(byteCode.data(),
		byteCode.size(),
		nullptr,
		&cs);
	if (FAILED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateComputeShader failed: %s", shaderName));
		return nullptr;
	}

	ShaderConfig cfg;
	cfg.m_name = shaderName;
	cfg.m_computeEntryPoint = entryPoint;

	Shader* shader = new Shader(cfg);
	shader->m_computeShader = cs;

	m_loadedShaders.push_back(shader);
	return shader;
}

Shader* Renderer::CreateComputeShader(char const* shaderPath, char const* entryPoint /*= "CSMain"*/)
{
	std::string(shaderPath) += ".hlsl";
	std::string source;
	int result = FileReadToString(source, shaderPath);
	if (result == -1)
	{
		ERROR_AND_DIE(Stringf("Failed to read compute shader file: %s", shaderPath));
		return nullptr;
	}

	return CreateComputeShader(shaderPath, source.c_str(), entryPoint);
}


Shader* Renderer::GetShader(char const* shaderName)
{
	for (Shader* shader : m_loadedShaders) 
	{
		if (shader->GetName() == shaderName) 
		{ 
			return shader;
		}
	}
	return nullptr;
}

bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode,
	const char* name,
	const char* source,
	const char* entryPoint,
	const char* target)
{
	DWORD shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDER)
	shaderFlags = D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	shaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

	ID3DBlob* shaderBlob = NULL;
	ID3DBlob* errorBlob = NULL;

	HRESULT hr = D3DCompile(
		source, strlen(source),
		name, nullptr, nullptr,
		entryPoint, target, shaderFlags,
		0, &shaderBlob, &errorBlob);

	if (SUCCEEDED(hr))
	{
		outByteCode.resize(shaderBlob->GetBufferSize());
		memcpy(outByteCode.data(),
			shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize());

		DX_SAFE_RELEASE(shaderBlob);
		DX_SAFE_RELEASE(errorBlob);
		return true;
	}
	else
	{
		if (errorBlob != NULL)
		{
#if defined(ENGINE_DEBUG_RENDER)
			DebuggerPrintf((char*)errorBlob->GetBufferPointer());
#endif
			DX_SAFE_RELEASE(errorBlob);
		}

		DX_SAFE_RELEASE(shaderBlob);
		return false;
	}
}

void Renderer::BindShader(Shader* shader)
{
	if (!shader)
	{
		m_deviceContext->VSSetShader(m_defaultShader->m_vertexShader, nullptr, 0);
		m_deviceContext->PSSetShader(m_defaultShader->m_pixelShader, nullptr, 0);
		m_deviceContext->IASetInputLayout(m_defaultShader->m_inputLayout);
	}
	else
	{
		m_deviceContext->VSSetShader(shader->m_vertexShader, nullptr, 0);
		m_deviceContext->PSSetShader(shader->m_pixelShader, nullptr, 0);
		m_deviceContext->IASetInputLayout(shader->m_inputLayout);
		m_currentShader = shader;
	} 
	
}

void Renderer::BindComputeShader(Shader* computeShader)
{
	if (computeShader && computeShader->m_computeShader) 
	{
		m_deviceContext->CSSetShader(computeShader->m_computeShader, nullptr, 0);
		m_deviceContext->VSSetShader(nullptr, nullptr, 0);
		m_deviceContext->PSSetShader(nullptr, nullptr, 0);
		m_deviceContext->GSSetShader(nullptr, nullptr, 0);
	}
	else {
		m_deviceContext->CSSetShader(nullptr, nullptr, 0);
	}
}

VertexBuffer* Renderer::CreateVertexBuffer(const unsigned int size, unsigned int stride)
{
	return new VertexBuffer(m_device, size, stride);
}

IndexBuffer* Renderer::CreateIndexBuffer(const unsigned int size, unsigned int stride)
{
	return new IndexBuffer(m_device, size, stride);
}


StructuredBuffer* Renderer::CreateStructuredBuffer(uint32_t stride, uint32_t count, bool createSRV, bool createUAV, unsigned int uavFlags /*= 0*/, const void* initData /*= nullptr*/)
{
	ID3D11Device* device = GetDevice();
	if (!device || stride == 0 || count == 0) return nullptr;

	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = stride * count;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = stride;

	if (createUAV) {
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = 0;
		if (createSRV) bd.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		bd.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		bd.CPUAccessFlags = 0;
	}
	else {
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = 0;
		if (createSRV) bd.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	D3D11_SUBRESOURCE_DATA srd{};
	srd.pSysMem = initData;

	ID3D11Buffer* buf = nullptr;
	HRESULT hr = device->CreateBuffer(&bd, initData ? &srd : nullptr, &buf);
	if (FAILED(hr)) return nullptr;

	ID3D11ShaderResourceView* srv = nullptr;
	if (createSRV) {
		D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
		sd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		sd.Format = DXGI_FORMAT_UNKNOWN;
		sd.Buffer.FirstElement = 0;
		sd.Buffer.NumElements = count;
		hr = device->CreateShaderResourceView(buf, &sd, &srv);
		if (FAILED(hr)) { buf->Release(); return nullptr; }
	}

	ID3D11UnorderedAccessView* uav = nullptr;
	if (createUAV) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC ud{};
		ud.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		ud.Format = DXGI_FORMAT_UNKNOWN;
		ud.Buffer.FirstElement = 0;
		ud.Buffer.NumElements = count;
		ud.Buffer.Flags = (uavFlags & D3D11_BUFFER_UAV_FLAG_APPEND)
			? D3D11_BUFFER_UAV_FLAG_APPEND : 0;
		hr = device->CreateUnorderedAccessView(buf, &ud, &uav);
		if (FAILED(hr)) { if (srv) srv->Release(); buf->Release(); return nullptr; }
	}

	auto* sb = new StructuredBuffer(buf, srv, uav, stride, count);
	if (uav) uav->Release();
	if (srv) srv->Release();
	buf->Release();
	return sb;
}

void Renderer::UpdateConstantBuffer(ConstantBuffer* cb, const void* data, size_t size)
{
	if (!cb || !data || !m_deviceContext) return;

	ID3D11Buffer* buffer = cb->GetBuffer();
	if (!buffer) return;

	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);

	if (desc.Usage == D3D11_USAGE_DYNAMIC) 
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		HRESULT hr = m_deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

		if (SUCCEEDED(hr))
		{
			memcpy(mapped.pData, data, size);
			m_deviceContext->Unmap(buffer, 0);
		}
		else 
		{
			OutputDebugStringA(("Failed to map constant buffer, HRESULT: " + std::to_string(hr) + "\n").c_str());
		}
	}
	else if (desc.Usage == D3D11_USAGE_DEFAULT) 
	{
		m_deviceContext->UpdateSubresource(buffer, 0, nullptr, data, 0, 0);
		OutputDebugStringA("ConstantBuffer updated successfully using UpdateSubresource\n");
	}
	else
	{
		OutputDebugStringA("ERROR: Unsupported buffer usage type\n");
	}
}

void Renderer::BindConstantBufferCS(int slot, ConstantBuffer* cb)
{
	ID3D11Buffer* buf = cb->m_buffer;
	m_deviceContext->CSSetConstantBuffers(slot, 1, &buf);
}

void Renderer::BindConstantBufferVS(int slot, ConstantBuffer* cb)
{
	ID3D11Buffer* buf = cb->m_buffer;
	m_deviceContext->VSSetConstantBuffers(slot, 1, &buf);
}

void Renderer::BindConstantBufferGS(int slot, ConstantBuffer* cb)
{
	ID3D11Buffer* buf = cb->m_buffer;
	m_deviceContext->GSSetConstantBuffers(slot, 1, &buf);
}

void Renderer::BindConstantBufferPS(int slot, ConstantBuffer* cb)
{
	ID3D11Buffer* buf = cb->m_buffer;
	m_deviceContext->PSSetConstantBuffers(slot, 1, &buf);
}

void Renderer::CopyCPUToGPU(VertexBuffer* vbo, IndexBuffer* ibo, const Vertex_PCU* vertexes, const unsigned int* indices, int numVertexes, int numIndices)
{
	unsigned int requiredVBOSize = numVertexes * vbo->GetStride();
	if (requiredVBOSize > vbo->GetSize()) {
		vbo->Resize(requiredVBOSize);
	}

	unsigned int requiredIBOSize = numIndices * ibo->GetStride();
	if (requiredIBOSize > ibo->GetSize()) {
		ibo->Resize(requiredIBOSize);
	}

	D3D11_MAPPED_SUBRESOURCE vboResource;
	m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vboResource);
	memcpy(vboResource.pData, vertexes, requiredVBOSize);
	m_deviceContext->Unmap(vbo->m_buffer, 0);

	D3D11_MAPPED_SUBRESOURCE iboResource;
	m_deviceContext->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &iboResource);
	memcpy(iboResource.pData, indices, requiredIBOSize);
	m_deviceContext->Unmap(ibo->m_buffer, 0);
}

void Renderer::CopyCPUToGPU(VertexBuffer* vbo, IndexBuffer* ibo, const Vertex_PCUTBN* vertexes, const unsigned int* indices, int numVertexes, int numIndices)
{
	unsigned int requiredVBOSize = numVertexes * vbo->GetStride();
	if (requiredVBOSize > vbo->GetSize()) {
		vbo->Resize(requiredVBOSize);
	}

	unsigned int requiredIBOSize = numIndices * ibo->GetStride();
	if (requiredIBOSize > ibo->GetSize()) {
		ibo->Resize(requiredIBOSize);
	}

	D3D11_MAPPED_SUBRESOURCE vboResource;
	m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vboResource);
	memcpy(vboResource.pData, vertexes, requiredVBOSize);
	m_deviceContext->Unmap(vbo->m_buffer, 0);

	D3D11_MAPPED_SUBRESOURCE iboResource;
	m_deviceContext->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &iboResource);
	memcpy(iboResource.pData, indices, requiredIBOSize);
	m_deviceContext->Unmap(ibo->m_buffer, 0);
}


void Renderer::BindVertexBuffer(VertexBuffer* vbo)
{
	UINT startOffset = 0;
	
	ID3D11Buffer* buffers[] = { vbo->m_buffer };
	m_deviceContext->IASetVertexBuffers(0, 1, buffers, &vbo->m_stride, &startOffset);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::BindIndexBuffer(IndexBuffer* ibo)
{
	m_deviceContext->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, 0);
}

ConstantBuffer* Renderer::CreateConstantBuffer(unsigned int size)
{
	size = (size + 15) & ~15;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = (UINT)size;
	desc.Usage = D3D11_USAGE_DYNAMIC; 
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; 
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	ID3D11Buffer* buffer = nullptr;
	HRESULT hr = m_device->CreateBuffer(&desc, nullptr, &buffer);

	if (FAILED(hr)) {
		OutputDebugStringA(("Failed to create constant buffer, HRESULT: " + std::to_string(hr) + "\n").c_str());
		return nullptr;
	}

	OutputDebugStringA(("Created DYNAMIC constant buffer, size: " + std::to_string(size) + "\n").c_str());
	return new ConstantBuffer(buffer, size);
}


void Renderer::CopyCPUToGPU(const void* data, unsigned int size, ConstantBuffer* cbo) const
{
	D3D11_MAPPED_SUBRESOURCE resource;
	HRESULT hr = m_deviceContext->Map(cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	if (!SUCCEEDED(hr)) 
	{
		ERROR_AND_DIE("Failed to Map Constant Buffer");
		return;
	}
	memcpy(resource.pData, data, size);
	m_deviceContext->Unmap(cbo->m_buffer, 0);
}


void Renderer::BindConstantBuffer(int slot, ConstantBuffer* cbo) const
{
	m_deviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_deviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
}

void Renderer::SetCameraConstants(const Camera& camera) const
{
	CameraConstants cameraConstants;
	cameraConstants.CameraPosition = camera.GetPosition();
	cameraConstants.WorldToCameraTransform = camera.GetWorldToCameraTransform();
	cameraConstants.CameraToRenderTransform = camera.GetCameraToRenderTransform();
	cameraConstants.RenderToClipTransform = camera.GetRenderToClipTransform();

	CopyCPUToGPU(&cameraConstants, sizeof(CameraConstants), m_cameraCBO);
	BindConstantBuffer(k_cameraConstantsSlot, m_cameraCBO);
}

void Renderer::SetModelConstants(const Mat44& modelToWorldTransform, const Rgba8 modelColor) const
{
	ModelConstants modelConstants;
	modelConstants.ModelToWorldTransform = modelToWorldTransform;
	modelColor.GetAsFloats(modelConstants.ModelColor);

	CopyCPUToGPU(&modelConstants, sizeof(ModelConstants), m_modelCBO);
	BindConstantBuffer(k_modelConstantsSlot, m_modelCBO);
}

void Renderer::SetLightConstants(Lights light)
{
	LightConstants lightConstants;
	lightConstants.SunDirection = light.m_sunDirection.GetNormalized();
	lightConstants.SunColor = light.m_sunColor;
	lightConstants.NumLights = light.m_numLights;
	
	for (int i = 0; i < light.m_numLights; ++i)
	{
		lightConstants.LightsArray[i] = light.m_lightsArray[i];
	}

	CopyCPUToGPU(&lightConstants, sizeof(LightConstants), m_lightCBO);
	BindConstantBuffer(k_lightConstantsSlot, m_lightCBO);

}


void Renderer::SetSpecialEffect(int effect)
{
	SpecialEffectConstants seConstants;
	seConstants.SpecialEffect = effect;

	CopyCPUToGPU(&seConstants, sizeof(SpecialEffectConstants), m_specialCBO);
	BindConstantBuffer(k_specialSlot, m_specialCBO);
}

void Renderer::SetPerFrameConstants(PerFrameDebug debugData)
{
	PerFrameConstants perframeConstants;
	perframeConstants.Time = debugData.m_time;
	perframeConstants.DebugInt = debugData.m_debugInt;
	perframeConstants.DebugFloat = debugData.m_debugFloat;

	CopyCPUToGPU(&perframeConstants, sizeof(PerFrameConstants), m_perframeCBO);
	BindConstantBuffer(k_perframeConstantsSlot, m_perframeCBO);
}


void Renderer::SetStatesIfChanged()
{
	if (m_blendState != m_blendStates[(int)m_desiredBlendMode]) {
		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		UINT sampleMask = 0xffffffff;
		m_deviceContext->OMSetBlendState(
			m_blendStates[static_cast<int>(m_desiredBlendMode)],
			blendFactor,
			sampleMask
		);

		m_blendState = m_blendStates[(int)m_desiredBlendMode];
	}

	if (m_samplerState != m_samplerStates[(int)m_desiredSamplerMode])
	{
		m_samplerState = m_samplerStates[(int)m_desiredSamplerMode];
		m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);
	}

	if (m_rasterizerState != m_rasterizerStates[(int)m_desiredRasterizerMode])
	{
		m_rasterizerState = m_rasterizerStates[(int)m_desiredRasterizerMode];
		m_deviceContext->RSSetState(m_rasterizerState);
	}

	if (m_depthStencilState != m_depthStencilStates[(int)m_desiredDepthMode])
	{
		m_depthStencilState = m_depthStencilStates[(int)m_desiredDepthMode];
		m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
	}
}

void Renderer::SetBlendMode(BlendMode blendMode)
{
	m_desiredBlendMode = blendMode;
}

void Renderer::SetSamplerMode(SamplerMode samplerMode)
{
	m_desiredSamplerMode = samplerMode;
}

void Renderer::SetRasterizerMode(RasterizerMode rasterizerMode)
{
	m_desiredRasterizerMode = rasterizerMode;
}

void Renderer::SetDepthMode(DepthMode depthMode)
{
	m_desiredDepthMode = depthMode;
}

void Renderer::CreateBlendStates()
{
	D3D11_BLEND_DESC opaqueDesc = { };
	opaqueDesc.RenderTarget[0].BlendEnable = TRUE;
	opaqueDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	opaqueDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	opaqueDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	opaqueDesc.RenderTarget[0].SrcBlendAlpha = opaqueDesc.RenderTarget[0].SrcBlend;
	opaqueDesc.RenderTarget[0].DestBlendAlpha = opaqueDesc.RenderTarget[0].DestBlend;
	opaqueDesc.RenderTarget[0].BlendOpAlpha = opaqueDesc.RenderTarget[0].BlendOp;
	opaqueDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HRESULT hr = m_device->CreateBlendState(&opaqueDesc, &m_blendStates[(int)(BlendMode::Blend_OPAQUE)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateBlendState for BlendMode::OPAQUE failed.");
	}

	D3D11_BLEND_DESC alphaDesc = { };
	alphaDesc.RenderTarget[0].BlendEnable = TRUE;
	alphaDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	alphaDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	alphaDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	alphaDesc.RenderTarget[0].SrcBlendAlpha = alphaDesc.RenderTarget[0].SrcBlend;
	alphaDesc.RenderTarget[0].DestBlendAlpha = alphaDesc.RenderTarget[0].DestBlend;
	alphaDesc.RenderTarget[0].BlendOpAlpha = alphaDesc.RenderTarget[0].BlendOp;
	alphaDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState(&alphaDesc, &m_blendStates[(int)(BlendMode::ALPHA)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateBlendState for BlendMode::ALPHA failed.");
	}

	D3D11_BLEND_DESC additiveDesc = { };
	additiveDesc.RenderTarget[0].BlendEnable = TRUE;
	additiveDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	additiveDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	additiveDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	additiveDesc.RenderTarget[0].SrcBlendAlpha = additiveDesc.RenderTarget[0].SrcBlend;
	additiveDesc.RenderTarget[0].DestBlendAlpha = additiveDesc.RenderTarget[0].DestBlend;
	additiveDesc.RenderTarget[0].BlendOpAlpha = additiveDesc.RenderTarget[0].BlendOp;
	additiveDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState(&additiveDesc, &m_blendStates[(int)(BlendMode::ADDITIVE)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateBlendState for BlendMode::ADDITIVE failed.");
	}

	D3D11_BLEND_DESC alphaAddDesc = {};
	alphaAddDesc.RenderTarget[0].BlendEnable = TRUE;
	alphaAddDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	alphaAddDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	alphaAddDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	alphaAddDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	alphaAddDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	alphaAddDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	alphaAddDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState(&alphaAddDesc, &m_blendStates[(int)BlendMode::ALPHA_ADDITIVE]);
	if (FAILED(hr)) ERROR_AND_DIE("CreateBlendState ALPHA_ADDITIVE failed.");

	D3D11_BLEND_DESC premulAlphaDesc = { };
	premulAlphaDesc.RenderTarget[0].BlendEnable = TRUE;
	premulAlphaDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	premulAlphaDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	premulAlphaDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	premulAlphaDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	premulAlphaDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	premulAlphaDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	premulAlphaDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = m_device->CreateBlendState(&premulAlphaDesc, &m_blendStates[(int)BlendMode::PREMULTIPLIED_ALPHA]);
	if (FAILED(hr)) ERROR_AND_DIE("CreateBlendState for PREMULTIPLIED_ALPHA failed.");
}

void Renderer::CreateSamplerStates()
{
	D3D11_SAMPLER_DESC pointClampDesc = { };
	pointClampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	pointClampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	pointClampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	pointClampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	pointClampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	pointClampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = m_device->CreateSamplerState(&pointClampDesc,
		&m_samplerStates[(int)SamplerMode::POINT_CLAMP]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateSamplerState for SamplerMode::POINT_CLAMP failed.");
	}

	D3D11_SAMPLER_DESC bilinearWrapDesc = { };
	bilinearWrapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	bilinearWrapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	bilinearWrapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	bilinearWrapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	bilinearWrapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	bilinearWrapDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_device->CreateSamplerState(&bilinearWrapDesc,
		&m_samplerStates[(int)SamplerMode::BILINEAR_WRAP]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateSamplerState for SamplerMode::POINT_CLAMP failed.");
	}

	m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);

}


void Renderer::CreateRasterizerMode()
{
	HRESULT hr;

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FrontCounterClockwise = true;
	// rasterizerDesc.DepthBias = 0;
	// rasterizerDesc.DepthBiasClamp = 0.f;
	// rasterizerDesc.SlopeScaledDepthBias = 0.f;
	rasterizerDesc.DepthClipEnable = true;
	// rasterizerDesc.ScissorEnable = false;
	// rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, 
		&m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateRasterizerState for RasterizerMode::SOLID_CULL_NONE failed.");
	}
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, 
		&m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_BACK]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateRasterizerState for RasterizerMode::SOLID_CULL_BACK failed.");
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, 
		&m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_NONE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateRasterizerState for RasterizerMode::WIREFRAME_CULL_NONE failed.");
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, 
		&m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_BACK]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateRasterizerState for RasterizerMode::WIREFRAME_CULL_BACK failed.");
	}
}

void Renderer::CreateDepthMode()
{
	D3D11_TEXTURE2D_DESC depthTextureDesc = {};
	depthTextureDesc.Width = m_config.m_window->GetClientDimensions().x;
	depthTextureDesc.Height = m_config.m_window->GetClientDimensions().y;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.SampleDesc.Count = 1;

	HRESULT hr;
	hr = m_device->CreateTexture2D(&depthTextureDesc, nullptr, &m_depthStencilTexture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create texture for depth stencil.");
	}

	hr = m_device->CreateDepthStencilView(m_depthStencilTexture, nullptr, &m_depthStencilDSV);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create depth stencil view.");
	}

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::DISABLED]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateDepthStencilState for DepthMode::DISABLED failed.");
	}

	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::READ_ONLY_ALWAYS]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateDepthStencilState for DepthMode::READ_ONLY_ALWAYS failed.");
	}

	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::READ_ONLY_LESS_EQUAL]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateDepthStencilState for DepthMode::READ_ONLY_LESS_EQUAL failed.");
	}

	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)DepthMode::READ_WRITE_LESS_EQUAL]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateDepthStencilState for DepthMode::READ_WRITE_LESS_EQUAL failed.");
	}


}


void Renderer::DrawIndexedVertexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, unsigned int indexCount)
{
	BindVertexBuffer(vbo);
	BindIndexBuffer(ibo);
	SetStatesIfChanged();
	m_deviceContext->DrawIndexed(indexCount, 0, 0);
}

bool Renderer::IsDeviceLost()
{
	if (!m_device) return true;

	HRESULT hr = m_device->GetDeviceRemovedReason();
	return hr != S_OK;
}

bool Renderer::CreateSceneDepth(IntVec2 size, unsigned int sampleCount /*= 1*/)
{
	DestroySceneDepth();

	D3D11_TEXTURE2D_DESC td = {};
	td.Width = (UINT)size.x;
	td.Height = (UINT)size.y;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R24G8_TYPELESS;
	td.SampleDesc.Count = sampleCount;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &m_sceneDepthTex);
	if (FAILED(hr)) return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};
	dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvd.ViewDimension = (sampleCount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS
		: D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;
	hr = m_device->CreateDepthStencilView(m_sceneDepthTex, &dsvd, &m_sceneDepthDSV);
	if (FAILED(hr)) { DestroySceneDepth(); return false; }

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvReadOnlyDesc = {};
	dsvReadOnlyDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvReadOnlyDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvReadOnlyDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
	hr = m_device->CreateDepthStencilView(m_sceneDepthTex, &dsvReadOnlyDesc, &m_sceneDepthDSV_ReadOnly);
	if (FAILED(hr)) { DestroySceneDepth(); return false; }


	if (sampleCount == 1) 
	{
		D3D11_TEXTURE2D_DESC copyd = td;
		copyd.SampleDesc.Count = 1;
		HRESULT hr2 = m_device->CreateTexture2D(&copyd, nullptr, &m_sceneDepthCopyTex);
		if (FAILED(hr2)) { DestroySceneDepth(); return false; }

		D3D11_SHADER_RESOURCE_VIEW_DESC srvd2 = {};
		srvd2.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvd2.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvd2.Texture2D.MipLevels = 1;
		hr2 = m_device->CreateShaderResourceView(m_sceneDepthCopyTex, &srvd2, &m_sceneDepthCopySRV);
		if (FAILED(hr2)) { DestroySceneDepth(); return false; }
	}
	else {
		D3D11_TEXTURE2D_DESC copyd = td;
		copyd.SampleDesc.Count = 1;
		hr = m_device->CreateTexture2D(&copyd, nullptr, &m_sceneDepthCopyTex);
		if (FAILED(hr)) { DestroySceneDepth(); return false; }

		D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
		srvd.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvd.Texture2D.MipLevels = 1;
		hr = m_device->CreateShaderResourceView(m_sceneDepthCopyTex, &srvd, &m_sceneDepthCopySRV);
		if (FAILED(hr)) { DestroySceneDepth(); return false; }
	}

	return true;
}

void Renderer::DestroySceneDepth()
{
	DX_SAFE_RELEASE(m_sceneDepthCopySRV);
	DX_SAFE_RELEASE(m_sceneDepthCopyTex);
	DX_SAFE_RELEASE(m_sceneDepthSRV);
	DX_SAFE_RELEASE(m_sceneDepthDSV);
	DX_SAFE_RELEASE(m_sceneDepthTex);
	DX_SAFE_RELEASE(m_sceneDepthDSV_ReadOnly);
}

void Renderer::CreateBackbufferRTV()
{
	DX_SAFE_RELEASE(m_backbufferRTV);
	DX_SAFE_RELEASE(m_backbufferTex);

	HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_backbufferTex);
	if (FAILED(hr)) { return; }

	hr = m_device->CreateRenderTargetView(m_backbufferTex, nullptr, &m_backbufferRTV);
	if (FAILED(hr)) { return; }
}

void Renderer::OnResizeBackbuffer(unsigned int width, unsigned int height)
{
	if (m_backbufferTex) {
		D3D11_TEXTURE2D_DESC desc{};
		m_backbufferTex->GetDesc(&desc);
		if (desc.Width == width && desc.Height == height) {
			return;
		}
	}

	DX_SAFE_RELEASE(m_backbufferRTV);
	DX_SAFE_RELEASE(m_backbufferTex);

	DestroySceneDepth();

	m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

	CreateBackbufferRTV();
	CreateSceneDepth(IntVec2(width, height), 1);
}

void Renderer::BeginOpaquePass()
{
	ID3D11RenderTargetView* rt = m_backbufferRTV;
	ID3D11DepthStencilView* ds = m_sceneDepthDSV;

	m_deviceContext->OMSetRenderTargets(1, &rt, ds);

	m_deviceContext->ClearDepthStencilView(ds, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::EndOpaquePass()
{
	if (m_sceneDepthTex && m_sceneDepthCopyTex) 
	{
		m_deviceContext->CopyResource(m_sceneDepthCopyTex, m_sceneDepthTex);
	}
}

ID3D11ShaderResourceView* Renderer::GetSceneDepthSRV() const
{
	if (m_sceneDepthCopySRV) return m_sceneDepthCopySRV; 
	if (m_sceneDepthSRV)     return m_sceneDepthSRV;
	return nullptr;
}

void Renderer::ClearScene()
{
	if (m_deviceContext)
	{
		m_deviceContext->ClearState();
		m_deviceContext->Flush();
	}
}

ID3D11DepthStencilView* Renderer::GetMainDSV_ReadOnly() const
{
	return m_sceneDepthDSV_ReadOnly;
}

ID3D11RenderTargetView* Renderer::GetMainRTV() const
{
	return m_backbufferRTV;
}

ID3D11DepthStencilView* Renderer::GetMainDSV() const
{
	return m_sceneDepthDSV;
}

void Renderer::CreateLightCBO()
{
	m_lightCBO = CreateConstantBuffer(sizeof(Lights));
}