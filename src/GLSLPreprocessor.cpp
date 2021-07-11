#include "GLSLPreprocessor.h"
#include "raylib.h"
#include "glslang/Public/ShaderLang.h"
#include "rlgl.h"
#include "external/glad.h"

const TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
    /* .maxDualSourceDrawBuffersEXT = */ 1,

    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    } };

EShLanguage GetStage(GLSLStage stage)
{
	switch (stage)
	{
	case GLSLStage::VERTEX:
		return EShLanguage::EShLangVertex;
	case GLSLStage::FRAGMENT:
		return EShLanguage::EShLangFragment;
	default:
		return (EShLanguage)0;
	}
}

const char* GetEntryPointForStage(GLSLStage stage)
{
	switch (stage)
	{
	case GLSLStage::VERTEX:
		return "VSMain";
	case GLSLStage::FRAGMENT:
		return "FSMain";
	default:
		return "InvalidStage";
	}
}

void GLSLPreprocessor::Init()
{
	glslang::InitializeProcess();
	
}

void GLSLPreprocessor::Shutdown()
{
	glslang::FinalizeProcess();
}

bool GLSLPreprocessor::ProcessShader(const char* shaderCode, const GLSLCompilerSettings& settings, std::string &result)
{
	if (settings.Stage == GLSLStage::NOT_SET)
	{
		Com_Error(ERR_DIALOG, "Failed to process shader. Invalid stage!");
	}

	result = "";// "#version 330\n";
    std::string shaderCodeToPass = "#version 330\n";
    std::string preamble = "// Autogenerated by LunarViewer\n";
	int defines = 1;
	for each(auto def in settings.Defines)
	{
        shaderCodeToPass += TextFormat("#define %s %s\n", def.first.c_str(), def.second.c_str());
        preamble += TextFormat("// %s = %s\n", def.first.c_str(), def.second.c_str());
		defines++;
	}

    shaderCodeToPass += TextFormat("#line 1\n");
    shaderCodeToPass += shaderCode;

	//return false;

	glslang::TProgram& program = *new glslang::TProgram;
	glslang::TShader* shader = new glslang::TShader(GetStage(settings.Stage));

	TBuiltInResource Resources;
	Resources = DefaultTBuiltInResource;
	//glslang::TShader::DirStackFileIncluder includer;
	glslang::TShader::ForbidIncluder includer;

    const char* shaderstr = shaderCodeToPass.c_str();

	shader->setStrings((const char* const*)(&(shaderstr)), 1);
	shader->setPreamble(preamble.c_str());
	//shader->setSourceEntryPoint(GetEntryPointForStage(settings.Stage));
    shader->setUniformLocationBase(0);
    shader->setNoStorageFormat(true);

	shader->setEnvInput(glslang::EShSourceGlsl, GetStage(settings.Stage), glslang::EShClientNone, 0);
	shader->setEnvClient(glslang::EShClientNone, (glslang::EShTargetClientVersion)0);
	shader->setEnvTarget(glslang::EShTargetLanguage::EShTargetNone, (glslang::EShTargetLanguageVersion)0);

	int defaultVersion = 330;

	std::string resultShaderCode;
	bool processresult = shader->preprocess(&Resources, defaultVersion, ENoProfile, false, false, EShMessages::EShMsgDefault, &resultShaderCode, includer);

	if (!processresult)
	{
		Com_Error(ERR_DIALOG, "Failed to preprocess shader!\n\n%s\n%s", shader->getInfoLog(), shader->getInfoDebugLog());

		return false;
	}
	else
	{
        result += preamble;
		result += resultShaderCode;
	}

    shaderstr = result.c_str();

    unsigned int tempshader = 0;
    tempshader = glCreateShader(settings.Stage == GLSLStage::VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
    glShaderSource(tempshader, 1, &shaderstr, NULL);

    GLint success = 0;
    glCompileShader(tempshader);
    glGetShaderiv(tempshader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        int maxLength = 0;
        glGetShaderiv(tempshader, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char* log = Com_Calloc(char, maxLength);
            glGetShaderInfoLog(tempshader, maxLength, &length, log);

            Com_Error(ERR_DIALOG, "Failed compiling shader!\n\n%s", log);
            Com_Free(log);
        }
    }

    glDeleteShader(tempshader);

    /*
    processresult = shader->parse(&Resources, defaultVersion, false, EShMessages::EShMsgDefault);
    if (!processresult)
    {
        Com_Error(ERR_DIALOG, "Failed parsing shader!\n\n%s\n%s", shader->getInfoLog(), shader->getInfoDebugLog());

        return false;
    }*/
	
	return false;
}
