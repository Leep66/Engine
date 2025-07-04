#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <vector>

#define DX_SAFE_RELEASE(dxObject) { if(dxObject) { (dxObject)->Release(); (dxObject) = nullptr; } }
#define SAFE_RELEASE(obj) { if (obj) { delete obj; obj = nullptr; } }


class Window;
struct IntVec2;
class VertexBuffer;
class IndexBuffer;
class BitmapFont;
class Texture;
class Shader;
class ConstantBuffer;
class Camera;
class ConstBuffer;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilState;
class Image;
struct AABB2;
struct D3D11_VIEWPORT;

enum class VertexType
{
	Vertex_PCU,
	Vertex_PCUTBN
};

enum class BlendMode
{
	Blend_OPAQUE,
	ALPHA,
	ADDITIVE,
	COUNT 
};

enum class SamplerMode
{
	POINT_CLAMP,
	BILINEAR_WRAP,
	COUNT
};

enum class RasterizerMode
{
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	COUNT
};

enum class DepthMode
{
	DISABLED,
	READ_ONLY_ALWAYS,
	READ_ONLY_LESS_EQUAL,
	READ_WRITE_LESS_EQUAL,
	COUNT
};

struct RendererConfig
{
	Window* m_window = nullptr;
};

struct Light
{
	Vec4 m_color = Vec4();
	Vec3 m_position = Vec3();
	float EMPTY_PADDING = 0.f;
	Vec3 m_direction = Vec3();
	float m_ambience = 0.f;

	float m_innerRadius = 0.f;
	float m_outerRadius = 0.f;
	float m_innerDotThreshold = -1.f;
	float m_outerDotThreshold = -1.f;
};



struct Lights
{
	Vec4 m_sunColor = Vec4(1.f, 1.f, 1.f, 1.f);
	Vec3 m_sunDirection = Vec3(2.0f, 1.0f, -1.0f);;
	int m_numLights = 0;
	Light m_lightsArray[MAX_LIGHTS];
};



struct PerFrameDebug
{
	float m_time = 0.f;
	int m_debugInt = 0;
	float m_debugFloat = 0.f;
};

struct ViewportData
{
	float TopLeftX = 0.f;
	float TopLeftY = 0.f;
	float Width = 0.f;
	float Height = 0.f;
};

class Renderer
{
public:
	Renderer(RendererConfig const& config);
	~Renderer();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void ClearScreen(const Rgba8& clearColor) const;
	void BeginCamera(const Camera& camera) const;
	void EndCamera(const Camera& camera) const;
	void DrawVertexArray(std::vector<Vertex_PCU> const& verts);
	void DrawVertexArray(std::vector<Vertex_PCUTBN> const& verts);
	void DrawVertexArray(std::vector<Vertex_PCUTBN> const& verts, std::vector<unsigned int> indexes);
	void DrawVertexArray(int numVertexes, Vertex_PCU const* verts);

	D3D11_VIEWPORT GetViewport(ViewportData viewportData) const;
	void SetViewport(ViewportData viewBoxs);

	Image* CreateImageFromFile(char const* imageFilePath);
	Texture* CreateTextureFromImage(const Image& image);
	Texture* GetTextureForFile(char const* imageFilePath);
	Texture* CreateOrGetTextureFromFile(char const* imageFilePath);
	Texture* CreateTextureFromFile(char const* imageFilePath);
	Texture* CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData);

	BitmapFont* CreateOrGetBitmapFont(char const* bitmapFontFilePath);
	BitmapFont* CreateBitmapFontFromFile(char const* bitmapFontFilePath);

	void BindTexture(const Texture* texture, int slot = 0);
	void CreateRenderingContext();

	Shader* CreateShader(char const* shaderName, char const* shaderSource, VertexType vertexType = VertexType::Vertex_PCU);
	Shader* CreateShader(char const* shaderPath, VertexType vertexType = VertexType::Vertex_PCU);
	Shader* GetShader(char const* shaderName);
	bool CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name,
		char const* source, char const* entryPoint, char const* target);
	void BindShader(Shader* shader);

	VertexBuffer* CreateVertexBuffer(const unsigned int size, unsigned int stride);
	IndexBuffer* CreateIndexBuffer(const unsigned int size, unsigned int stride);
	void CopyCPUToGPU(VertexBuffer* vbo, IndexBuffer* ibo, const Vertex_PCU* vertexes, const unsigned int* indices, int numVertexes, int numIndices);
	void CopyCPUToGPU(VertexBuffer* vbo, IndexBuffer* ibo, const Vertex_PCUTBN* vertexes, const unsigned int* indices, int numVertexes, int numIndices);
	void BindVertexBuffer(VertexBuffer* vbo);
	void BindIndexBuffer(IndexBuffer* ibo);

	ConstantBuffer* CreateConstantBuffer(const unsigned int size);
	void CopyCPUToGPU(const void* data, unsigned int size, ConstantBuffer* cbo) const;
	void BindConstantBuffer(int slot, ConstantBuffer* cbo) const;

	void SetCameraConstants(const Camera& camera) const;
	void SetModelConstants(const Mat44& modelToWorldTransform = Mat44(), const Rgba8 modelColor = Rgba8::WHITE) const;
	void SetLightConstants(Lights light);
	void SetSpecialEffect(int effect);
	void SetPerFrameConstants(PerFrameDebug debugData);

	void SetStatesIfChanged();

	void SetBlendMode(BlendMode blendMode);
	void SetSamplerMode(SamplerMode samplerMode);
	void SetRasterizerMode(RasterizerMode rasterizerMode);
	void SetDepthMode(DepthMode depthMode);
	void CreateBlendStates();
	void CreateSamplerStates();
	void CreateRasterizerMode();

	void CreateDepthMode();
	
	
	void DrawIndexedVertexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, unsigned int indexCount);

	Shader* GetCurrentShader() const { return m_currentShader; }

private:
	//
	RendererConfig m_config;
	const Texture* m_defaultTexture = nullptr;
	std::vector<Texture*> m_loadedTextures;
	std::vector<BitmapFont*> m_loadedFonts;
	std::vector<Shader*> m_loadedShaders;
	Shader* m_currentShader = nullptr;
	Shader* m_defaultShader = nullptr;
	VertexBuffer* m_immediatePCUVBO = nullptr;
	VertexBuffer* m_immediatePCUTBNVBO = nullptr;
	IndexBuffer* m_immediateIBO = nullptr;

	Camera* m_currentCamera = nullptr;

	ConstantBuffer* m_cameraCBO = nullptr;
	ConstantBuffer* m_modelCBO = nullptr;
	ConstantBuffer* m_lightCBO = nullptr;
	ConstantBuffer* m_perframeCBO = nullptr;
	ConstantBuffer* m_specialCBO = nullptr;

	ID3D11BlendState* m_blendState = nullptr;
	ID3D11BlendState* m_blendStates[(int)(BlendMode::COUNT)] = {};
	
	ID3D11SamplerState* m_samplerState = nullptr;
	ID3D11SamplerState* m_samplerStates[(int)(SamplerMode::COUNT)] = {};

	ID3D11RasterizerState* m_rasterizerState = nullptr;
	ID3D11RasterizerState* m_rasterizerStates[(int)(RasterizerMode::COUNT)] = {};

	ID3D11Texture2D* m_depthStencilTexture = nullptr;
	ID3D11DepthStencilView* m_depthStencilDSV = nullptr;

	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11DepthStencilState* m_depthStencilStates[(int)(DepthMode::COUNT)] = {};

	BlendMode m_desiredBlendMode = BlendMode::ALPHA;
	SamplerMode m_desiredSamplerMode = SamplerMode::POINT_CLAMP;
	RasterizerMode m_desiredRasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	DepthMode m_desiredDepthMode = DepthMode::READ_WRITE_LESS_EQUAL;
	//
};

