#include "ShaderManager.h"
//#include "slang.h"
#include "GLSLPreprocessor.h"

//SlangSession* session;

std::vector<LunarShader> ShaderInstances;

void ShaderManager::Init()
{
    //session = spCreateSession(NULL);
    GLSLPreprocessor::Init();
}

void ShaderManager::Shutdown()
{
    //spDestroySession(session);
    GLSLPreprocessor::Shutdown();
}

LunarShader* ShaderManager::CompileShader(const char* shaderName, /*LunarRenderMode*/ uint32_t renderMode, bool UseAnimInterpolation)
{
    const char* shaderCode = LoadFileText(TextFormat("shaders_src/%s.glsl", shaderName));

    GLSLCompilerSettings compileSettingsVert;
    std::string shaderResult = "";
    compileSettingsVert.Defines.emplace("RENDERMODE", TextFormat("%u", renderMode));
    compileSettingsVert.Defines.emplace("VERTEX_SHADER", "1");
    compileSettingsVert.Stage = GLSLStage::VERTEX;
    
    if(UseAnimInterpolation)
        compileSettingsVert.Defines.emplace("ANIM_INTERPOLATION", "1");

    GLSLPreprocessor::ProcessShader(shaderCode, compileSettingsVert, shaderResult);

    SaveFileText(GetVertexShaderPath(shaderName, renderMode, UseAnimInterpolation), (char *)shaderResult.c_str());

    GLSLCompilerSettings compileSettingsFrag;
    compileSettingsFrag.Defines.emplace("RENDERMODE", TextFormat("%u", renderMode));
    compileSettingsFrag.Defines.emplace("FRAGMENT_SHADER", "1");
    compileSettingsFrag.Stage = GLSLStage::FRAGMENT;

    // Skipping anim interpolation for fragment shader
    if (UseAnimInterpolation)
    {
        //compileSettingsFrag.Defines.emplace("ANIM_INTERPOLATION", "1");
        Com_Printf("Skipped fragment shader compile with anim interpolation\n");
        return nullptr;
    }
    else
    {
        GLSLPreprocessor::ProcessShader(shaderCode, compileSettingsFrag, shaderResult);
        SaveFileText(GetFragmentShaderPath(shaderName, renderMode, UseAnimInterpolation), (char*)shaderResult.c_str());
    }

    //RL_FREE((void*)shaderCode);
    Com_Free(shaderCode);

    // Awful, just absolutely awful
#if 0
    SlangCompileRequest* request = spCreateCompileRequest(session);

    spSetCodeGenTarget(request, SLANG_GLSL);
    spAddSearchPath(request, "shaders_src/");

    spAddPreprocessorDefine(request, "RENDERMODE", TextFormat("%u", renderMode));

    spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_HLSL, "");

    spAddTranslationUnitSourceFile(request, 0, TextFormat("shaders_src/%s.hlsl", shaderName));

    SlangProfileID profVert = spFindProfile(session, "glsl_vertex");
    SlangProfileID profFrag = spFindProfile(session, "glsl_fragment");

    int vertEntry = spAddEntryPoint(request, 0, "VSMain", profVert);
    int fragEntry = spAddEntryPoint(request, 0, "PSMain", profFrag);

    spSetCompileFlags(request, SLANG_COMPILE_FLAG_NO_CODEGEN);

    int result = spCompile(request);
    const char* output = spGetDiagnosticOutput(request);

    if (result)
    {
        Com_Error(ERR_DIALOG, "Failed compiling %s for %s\n%s", shaderName, id, output);
        spDestroyCompileRequest(request);
        return nullptr;
    }

    const char* code = spGetEntryPointSource(request, vertEntry);
    SaveFileText(TextFormat("shaders/%s.rm%u.vs", shaderName, renderMode), (char* )code);
    code = spGetEntryPointSource(request, fragEntry);
    SaveFileText(TextFormat("shaders/%s.rm%u.fs", shaderName, renderMode), (char* )code);

    spDestroyCompileRequest(request);

#endif
    return nullptr;
}

const char* ShaderManager::GetVertexShaderPath(const char* shader, /*LunarRenderMode*/ uint32_t renderMode, bool UseAnimInterpolation)
{
    return TextFormat("shaders_cache/%s.rm%u_%s.vs", shader, renderMode, UseAnimInterpolation ? "ai" : "ni");
}

const char* ShaderManager::GetFragmentShaderPath(const char* shader, /*LunarRenderMode*/ uint32_t renderMode, bool UseAnimInterpolation)
{
    // Skipping anim interpolation for fragment shader
    return TextFormat("shaders_cache/%s.rm%u.fs", shader, renderMode);
}
