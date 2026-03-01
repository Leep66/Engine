#pragma once
#include <string>
#include <vector>

struct StaticMeshDefinition
{
	std::string m_name = "";
	std::string m_path = "";
	std::string m_shader = "";
	std::string m_diffuseMap = "";
	std::string m_normalMap = "";
	std::string m_specGlossEmitMap = "";
	int m_unitsPerMeter = 0;
	std::string m_xAxis = "forward";
	std::string m_yAxis = "left";
	std::string m_zAxis = "up";

	static void InitializeStaticMeshDefinitions(const char* path);

	static StaticMeshDefinition const& GetDefinition(std::string name);
	static std::vector<StaticMeshDefinition> s_meshDefs;
};