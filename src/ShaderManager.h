#pragma once
#include "Common.h"
#include "raylib.h"
#include <unordered_set>
#include <iostream>

enum LunarShaderConfigFlags
{

};

struct LunarShader
{
	Shader RaylibShader = { 0 };
};

namespace ShaderManager
{
	void Init();
	void Shutdown();

	LunarShader* CompileShader(const char* shaderPath, /*LunarRenderMode*/ uint32_t renderMode, bool UseAnimInterpolation);

	const char* GetVertexShaderPath(const char* shader, /*LunarRenderMode*/ uint32_t renderMode, bool UseAnimInterpolation);
	const char* GetFragmentShaderPath(const char* shader, /*LunarRenderMode*/ uint32_t renderMode, bool UseAnimInterpolation);
}