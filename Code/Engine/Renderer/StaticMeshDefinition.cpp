#include "Engine/Renderer/StaticMeshDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"


std::vector<StaticMeshDefinition> StaticMeshDefinition::s_meshDefs;

void StaticMeshDefinition::InitializeStaticMeshDefinitions(const char* path)
{
 	XmlDocument doc;
	XmlResult result = doc.LoadFile(path);
	if (result != tinyxml2::XML_SUCCESS)
	{
		ERROR_AND_DIE("Failed to load " + std::string(path));
	}

	XmlElement* root = doc.RootElement();
	if (root == nullptr)
	{
		ERROR_AND_DIE(std::string(path) + " is missing a root element");
	}

	for (XmlElement* elem = root->FirstChildElement("StaticModelInfo"); elem != nullptr; elem = elem->NextSiblingElement("StaticModelInfo"))
	{
		StaticMeshDefinition def;

		def.m_name = ParseXmlAttribute(*elem, "name", "");
		def.m_path = ParseXmlAttribute(*elem, "path", "");
		def.m_shader = ParseXmlAttribute(*elem, "shader", "");
		def.m_diffuseMap = ParseXmlAttribute(*elem, "diffuseMap", "");
		def.m_normalMap = ParseXmlAttribute(*elem, "normalMap", "");
		def.m_specGlossEmitMap = ParseXmlAttribute(*elem, "specGlossEmitMap", "");
		def.m_unitsPerMeter = ParseXmlAttribute(*elem, "unitsPerMeter", 1);

		def.m_xAxis = ParseXmlAttribute(*elem, "x", "left");
		def.m_yAxis = ParseXmlAttribute(*elem, "y", "up");
		def.m_zAxis = ParseXmlAttribute(*elem, "z", "forward");

		s_meshDefs.push_back(def);
	}

}

StaticMeshDefinition const& StaticMeshDefinition::GetDefinition(std::string name)
{
	for (size_t i = 0; i < (int)s_meshDefs.size(); ++i)
	{
		if (s_meshDefs[i].m_name == name)
		{
			return s_meshDefs[i];
		}
	}

	static StaticMeshDefinition defaultDef; 
	return defaultDef;
}

