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
	int SpecialEffect;
	int EMPTY_PADDING[3] = { };
};
static const int k_specialSlot = 8;


Renderer::Renderer(RendererConfig const& config)
	: m_config(config)
{
}

Renderer::~Renderer()
{
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
	swapChainDesc.OutputWindow = (HWND)m_config.m_window->GetHwnd();
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
	m_lightCBO = CreateConstantBuffer(sizeof(LightConstants));
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
}

void Renderer::BeginFrame()
{
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilDSV);
	m_deviceContext->ClearDepthStencilView(m_depthStencilDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
}

void Renderer::EndFrame()
{
	HRESULT hr;
	hr = m_swapChain->Present(1, 0);
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		ERROR_AND_DIE("Device has been lost, application will now terminate.");
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

	if (m_immediatePCUVBO) { delete m_immediatePCUVBO; m_immediatePCUVBO = nullptr; }
	if (m_immediateIBO) { delete m_immediateIBO; m_immediateIBO = nullptr; }
	if (m_immediatePCUTBNVBO) { delete m_immediatePCUTBNVBO; m_immediatePCUTBNVBO = nullptr; }
	if (m_cameraCBO) { delete m_cameraCBO; m_cameraCBO = nullptr; }
	if (m_modelCBO) { delete m_modelCBO; m_modelCBO = nullptr; }
	if (m_lightCBO) { delete m_lightCBO; m_lightCBO = nullptr; }
	if (m_perframeCBO) { delete m_perframeCBO; m_perframeCBO = nullptr; }

	for (Shader* shader : m_loadedShaders)
	{
		delete shader;
		shader = nullptr;
	}
	m_currentShader = nullptr;
	m_loadedShaders.clear();

	for (int i = 0; i < (int)BlendMode::COUNT; ++i)
	{
		DX_SAFE_RELEASE(m_blendStates[i]);
	}

	for (int i = 0; i < (int)SamplerMode::COUNT; ++i)
	{
		DX_SAFE_RELEASE(m_samplerStates[i]);
	}


	for (int i = 0; i < (int)m_loadedTextures.size(); i++)
	{
		delete m_loadedTextures[i];
		m_loadedTextures[i] = nullptr;
	}
	m_loadedTextures.clear();

	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_deviceContext);

	m_blendState = nullptr;
	m_samplerState = nullptr;
	m_rasterizerState = nullptr;
	m_depthStencilTexture = nullptr;
	m_depthStencilState = nullptr;
	
	DX_SAFE_RELEASE(m_device);


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
}

void Renderer::EndCamera(const Camera& camera) const
{
	UNUSED(camera);
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


Texture* Renderer::CreateTextureFromImage(const Image& image)
{
	for (Texture* existingTexture : m_loadedTextures)
	{
		if (existingTexture->GetImageFilePath() == image.GetImageFilePath())
		{
			return existingTexture;
		}
	}

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = image.GetDimensions().x;
	textureDesc.Height = image.GetDimensions().y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureData;
	textureData.pSysMem = image.GetRawData();
	textureData.SysMemPitch = 4 * image.GetDimensions().x;

	Texture* newTexture = new Texture();
	HRESULT hr = m_device->CreateTexture2D(&textureDesc, &textureData, &newTexture->m_texture);
	if (!SUCCEEDED(hr))
	{
		delete newTexture;
		ERROR_AND_DIE(Stringf("CreateTextureFromImage failed for image file \"%s\".",
			image.GetImageFilePath().c_str()));
	}

	hr = m_device->CreateShaderResourceView(newTexture->m_texture, NULL,
		&newTexture->m_shaderResourceView);
	if (!SUCCEEDED(hr))
	{
		DX_SAFE_RELEASE(newTexture->m_texture);
		delete newTexture;
		ERROR_AND_DIE(Stringf("CreateShaderResouorceView failed for image file \"%s\".",
			image.GetImageFilePath().c_str()));
	}

	newTexture->m_dimensions = image.GetDimensions();

	m_loadedTextures.push_back(newTexture);

	return newTexture;
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

void Renderer::CreateRenderingContext()
{
	
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

Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath)
{
	Texture* existingTexture = GetTextureForFile(imageFilePath);
	if (existingTexture)
	{
		return existingTexture;
	}

	Texture* newTexture = CreateTextureFromFile(imageFilePath);
	newTexture->m_name = imageFilePath;
	newTexture->m_dimensions = newTexture->GetDimensions();
	return newTexture;
}

Texture* Renderer::CreateTextureFromFile(char const* imageFilePath)
{
	Image* newImage = CreateImageFromFile(imageFilePath);
	Texture* newTexture = CreateTextureFromImage(*newImage);

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


Shader* Renderer::CreateShader(const char* shaderName, const char* source, VertexType vertexType)
{
	ShaderConfig config;
	config.m_name = shaderName;

	Shader* shader = new Shader(config);

	std::vector<unsigned char> vertexShaderByteCode;
	if (!CompileShaderToByteCode(vertexShaderByteCode, shaderName, source, "VertexMain", "vs_5_0"))
	{
		delete shader;
		ERROR_AND_DIE("Failed to compile vertex shader.");
		return nullptr;
	}

	HRESULT hr = m_device->CreateVertexShader(
		vertexShaderByteCode.data(),
		vertexShaderByteCode.size(),
		nullptr, &shader->m_vertexShader);
	if (!SUCCEEDED(hr))
	{
		delete shader;
		ERROR_AND_DIE("Failed to create vertex shader.");
		return nullptr;
	}

	std::vector<unsigned char> pixelShaderByteCode;
	if (!CompileShaderToByteCode(pixelShaderByteCode, shaderName, source, "PixelMain", "ps_5_0"))
	{
		delete shader;
		ERROR_AND_DIE("Failed to compile pixel shader.");
		return nullptr;
	}

	hr = m_device->CreatePixelShader(
		pixelShaderByteCode.data(),
		pixelShaderByteCode.size(),
		nullptr, &shader->m_pixelShader);
	if (!SUCCEEDED(hr))
	{
		delete shader;
		ERROR_AND_DIE("Failed to create pixel shader.");
		return nullptr;
	}

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

	hr = m_device->CreateInputLayout(
		inputLayoutDesc, numElements,
		vertexShaderByteCode.data(),
		vertexShaderByteCode.size(),
		&shader->m_inputLayout);
	if (!SUCCEEDED(hr))
	{
		delete shader;
		ERROR_AND_DIE("Failed to create input layout.");
		return nullptr;
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
	}
	else
	{
		if (errorBlob != NULL)
		{
			DebuggerPrintf((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		ERROR_AND_DIE(Stringf("Could not compile shader '%s'.", name));
	}

	DX_SAFE_RELEASE(shaderBlob);
	DX_SAFE_RELEASE(errorBlob);

	return true;
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

void Renderer::DrawIndexedVertexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, unsigned int indexCount)
{
	BindVertexBuffer(vbo);
	BindIndexBuffer(ibo);
	SetStatesIfChanged();
	m_deviceContext->DrawIndexed(indexCount, 0, 0);
}



VertexBuffer* Renderer::CreateVertexBuffer(const unsigned int size, unsigned int stride)
{
	return new VertexBuffer(m_device, size, stride);
}

IndexBuffer* Renderer::CreateIndexBuffer(const unsigned int size, unsigned int stride)
{
	return new IndexBuffer(m_device, size, stride);
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


ConstantBuffer* Renderer::CreateConstantBuffer(const unsigned int size)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ConstantBuffer* newBuffer = new ConstantBuffer(size);

	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &newBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create constant buffer.");
	}

	return newBuffer;
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


