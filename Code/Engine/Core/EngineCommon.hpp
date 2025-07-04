#pragma once
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include "Engine/Input/NamedStrings.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
#define UNUSED(x) (void)(x);

class NamedStrings;
extern NamedStrings g_gameConfigBlackboard;
typedef std::vector<Vertex_PCU> Vertexes;
constexpr int MAX_LIGHTS = 8;