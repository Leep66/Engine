#pragma once
#include <string>

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11ComputeShader;

struct ShaderConfig
{
	std::string m_name;

	std::string m_vertexEntryPoint = "VertexMain";
	std::string m_pixelEntryPoint = "PixelMain";

	std::string m_computeEntryPoint = "CSMain";
};

class Shader
{
	friend class Renderer;

public:
	explicit Shader(const ShaderConfig& config);
	Shader(const Shader& copy) = delete;
	~Shader();

	const std::string& GetName() const { return m_config.m_name; }

	bool IsCompute() const { return m_computeShader != nullptr; }
	bool IsGraphics() const { return (m_vertexShader != nullptr) || (m_pixelShader != nullptr); }

public:
	ShaderConfig m_config;

	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;

	ID3D11ComputeShader* m_computeShader = nullptr;
};
