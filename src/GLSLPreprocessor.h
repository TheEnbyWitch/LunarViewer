#pragma once
#include "Common.h"
#include <vector>
#include <map>
#include <iostream>

enum class GLSLStage
{
	NOT_SET = 0,
	VERTEX = 1,
	FRAGMENT = 2
};

struct GLSLCompilerSettings
{
	std::map<std::string, std::string> Defines;

	GLSLStage Stage = GLSLStage::NOT_SET;
};

namespace GLSLPreprocessor
{
	void Init();
	void Shutdown();
	bool ProcessShader(const char* shaderCode, const GLSLCompilerSettings& settings, std::string &result);


};

