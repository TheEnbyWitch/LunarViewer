// C 2021 Luna
// ShipGame Main

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "Common.h"
#include "ShaderManager.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "external/glad.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include "external/glad.h"

#include "physfs.h"

#include "CMDL.h"
#include "FileIO.h"

#include "imgui/imgui.h"
// Uses
// https://github.com/oskaritimperi/imgui-impl-raylib
#include "imgui/imgui_impl_raylib.h"

FViewerSettings GViewerSettings;

CMDL* CurrentModel = NULL;

enum EFileProtocol
{
    FP_Normal,
    FP_Pak,
};

unsigned char* ShipGame_LoadFileDataCallback(const char* fileName, unsigned int* bytesRead)
{
    FILE* f = fopen(fileName, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // LUNA: This function is used by raylib too
    // as it is right now, i cant get a callstack for every allocation, so dont record the allocation
    unsigned char* buf = Com_Malloc_NoInfo<unsigned char>(size);

    *bytesRead = fread(buf, 1, size, f);

    fclose(f);

    return buf;
}

char* ShipGame_LoadFileTextCallback(const char* fileName)
{
    Com_Printf("Loading %s, working dir is %s.\n", fileName, GetWorkingDirectory());
    FILE* f = fopen(fileName, "rt");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    int size = ftell(f)+1;
    fseek(f, 0, SEEK_SET);

    // LUNA: This function is used by raylib too
    // as it is right now, i cant get a callstack for every allocation, so dont record the allocation
    char* buf = Com_Malloc_NoInfo<char>(size);
    memset(buf, 0, size);

    fread(buf, 1, size-1, f);

    fclose(f);

    return buf;
}

void ViewerLoadModel(std::string Path)
{
    if (CurrentModel)
    {
        Com_Delete(CurrentModel);
    }

    CurrentModel = Com_New(CMDL, Path);
    CurrentModel->Init(Path);
    CurrentModel->RefreshModel();

    SetWindowTitle(TextFormat("LunarViewer - %s", Path.c_str()));
}

Texture2D ColormapTexture = { 0 };
Texture2D PaletteTexture = { 0 };

const char* GetDirectoryForGame(LunarViewerGame Game)
{
    switch (Game)
    {
    case LunarViewerGame::Quake1:
        return "quake1";
    case LunarViewerGame::Hexen2:
        return "hexen2";
    }

    return "quake1";
}

const char* GetDirectoryForCurrentGame()
{
    return GetDirectoryForGame(GViewerSettings.CurrentGame);
}

void LoadColormap()
{
    // Begin with the palette

    unsigned int bytesRead = 0;

    struct rgb_s
    {
        uint8_t R;
        uint8_t G;
        uint8_t B;
    } *PaletteRGB = (rgb_s*)LoadFileData(TextFormat("%s/palette.lmp", GetDirectoryForCurrentGame()), &bytesRead);

    if (bytesRead < (256 * 3))
    {
        Com_Error(ERR_FATAL, "Failed to load palette! Not enough entries!");
    }

    Image PaletteImage = { 0 };

    PaletteImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    PaletteImage.width = 256;
    PaletteImage.height = 1;
    PaletteImage.mipmaps = 1;
    PaletteImage.data = PaletteRGB;

    PaletteTexture = LoadTextureFromImage(PaletteImage);

    SetTextureFilter(PaletteTexture, TEXTURE_FILTER_POINT);
    SetTextureWrap(PaletteTexture, TEXTURE_WRAP_CLAMP);

    // Load the colormap and convert it to RGB

    uint8_t* Colormap = (uint8_t*)LoadFileData(TextFormat("%s/colormap.lmp", GetDirectoryForCurrentGame()), &bytesRead);

    if (bytesRead < (256 * 64))
    {
        Com_Error(ERR_FATAL, "Failed to load colormap! Not enough entries!");
    }

    Image ColormapImage = { 0 };

    ColormapImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    ColormapImage.width = 256;
    ColormapImage.height = 64;

    rgb_s* ColormapRGB = Com_Calloc(rgb_s, 256 * 64);

    for (int i = 0; i < 256 * 64; i++)
    {
        ColormapRGB[i] = PaletteRGB[Colormap[i]];
    }

    ColormapImage.data = ColormapRGB;
    ColormapImage.mipmaps = 1;

    ColormapTexture = LoadTextureFromImage(ColormapImage);

    SetTextureFilter(ColormapTexture, TEXTURE_FILTER_POINT);
    SetTextureWrap(ColormapTexture, TEXTURE_WRAP_CLAMP);

    Com_Free(PaletteRGB);
    Com_Free(ColormapRGB);
}

Shader shader;
RenderTexture2D Viewport = { 0 };
ImVec2 DesiredViewportSize = ImVec2(16, 16);

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(why?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void ImageViewerWithZoom(Texture texture)
{
    ImGuiIO &io = ImGui::GetIO();
    ImTextureID my_tex_id = texture.id;
    float my_tex_w = (float)texture.width;
    float my_tex_h = (float)texture.height;
    {
        //ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
        ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
        ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
        ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
        ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            float region_sz = 32.0f;
            float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
            float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
            float zoom = 4.0f;
            if (region_x < 0.0f) { region_x = 0.0f; }
            else if (region_x > my_tex_w - region_sz) { region_x = my_tex_w - region_sz; }
            if (region_y < 0.0f) { region_y = 0.0f; }
            else if (region_y > my_tex_h - region_sz) { region_y = my_tex_h - region_sz; }
            ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
            ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
            ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
            ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
            ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
            ImGui::EndTooltip();
        }
    }
}

extern Texture2D NormalTexture;

bool Anims_ArrayGetter(void* data, int index, const char** out)
{
    if(!data)
        return false;

    std::vector<FMDLAnimation> animations = *(std::vector<FMDLAnimation> *)data;

    if (index >= animations.size() || index < 0)
        return false;

    *out = new char[16];

    strcpy(((char *)*out), animations[index].Name);

    return true;
}

Image LVIcon;
Texture2D LVTexture;

int main(int argc, char** argv)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    
    PHYSFS_init(NULL);

    PHYSFS_mount("/", "GAME", 0);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(640, 480, "LunarViewer");

    if(!GetWindowHandle())
    {
        Com_Error(ERR_FATAL, "Failed to initialize raylib and window!");
    }

    int screenWidth = GetMonitorWidth(GetCurrentMonitor());
    int screenHeight = GetMonitorHeight(GetCurrentMonitor());

    int scale = 2;//screenHeight / 240;

    //SetWindowPosition(24, 24);
    //SetWindowSize(320 * scale, 240 * scale);

    SetLoadFileDataCallback(ShipGame_LoadFileDataCallback);
    SetLoadFileTextCallback(ShipGame_LoadFileTextCallback);

    //if(!IsWindowFullscreen())
        //ToggleFullscreen();

    int	aa = 0x12345678;

    bool host_bigendian = false;

    if (*(char*)&aa == 0x12)
        host_bigendian = true;
    else if (*(char*)&aa == 0x78)
        host_bigendian = false;

    Com_Printf("Host machine is %s\n", host_bigendian ? "big endian" : "little endian");

    if (host_bigendian)
    {
        Com_Error(ERR_FATAL, "Host machine is big endian, which is not currently supported!");
    }

    LVIcon = LoadImage("assets/lv_icon.png");
    ImageMipmaps(&LVIcon);
    LVTexture = LoadTextureFromImage(LVIcon);

    ImageResize(&LVIcon, 32, 32);

    SetWindowIcon(LVIcon);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;

    ImFont *fUnorm = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-Regular.ttf", 14.f);
    ImFont *fUtitle = io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-Regular.ttf", 32.f);

    ImGui::StyleColorsDark();

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    //colors[ImGuiCol_Border] = ImVec4(0.50f, 0.43f, 0.45f, 0.50f);
    colors[ImGuiCol_Border] = ImVec4(0.50f, 0.33f, 0.38f, 0.23f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.48f, 0.16f, 0.25f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.98f, 0.26f, 0.46f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.98f, 0.26f, 0.46f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.16f, 0.25f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.98f, 0.26f, 0.46f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.88f, 0.24f, 0.42f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.26f, 0.46f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.98f, 0.26f, 0.46f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.26f, 0.46f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.06f, 0.32f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.98f, 0.26f, 0.46f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.26f, 0.46f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.98f, 0.26f, 0.46f, 1.00f);
    //colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.43f, 0.45f, 0.50f);
    colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.27f, 0.34f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.10f, 0.28f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.10f, 0.28f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.26f, 0.46f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.26f, 0.46f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.26f, 0.46f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.58f, 0.18f, 0.29f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.26f, 0.46f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.88f, 0.33f, 0.49f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.07f, 0.09f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.14f, 0.22f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.98f, 0.26f, 0.46f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.980f, 0.537f, 0.259f, 1.000f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.98f, 0.26f, 0.46f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.98f, 0.26f, 0.46f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    ImGui::GetStyle().WindowRounding = 4.f;
    ImGui::GetStyle().ChildRounding = 4.f;
    ImGui::GetStyle().PopupRounding = 4.f;
    ImGui::GetStyle().FrameRounding = 4.f;

    ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);
    ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_Right;
    ImGui::GetStyle().FrameBorderSize = 1.f;

    ImGui_ImplRaylib_Init();
    ImGui_ImplRaylib_LoadDefaultFontAtlas();

    float UnitScale = 1.f;

    RenderTexture Target = LoadRenderTexture(320, 240);
    SetTextureFilter(Target.texture, TEXTURE_FILTER_POINT);
    Camera Cam;
    Cam.projection = CAMERA_PERSPECTIVE;
    Cam.fovy = 70.f;
    Cam.position = Vector3{ 64,64 * UnitScale,64 * UnitScale };
    Cam.target = Vector3{ 0,32,0 };
    Cam.up = Vector3{ 0,1,0 };

    CMDL::Setup();

    LoadColormap();
    
    ShaderManager::Init();
    for (uint32_t i = 0; i < RENDERMODE_MAX; i++)
    {
        Com_Printf("Compiling shaders %u/%u\n", (i*2) + 1, RENDERMODE_MAX * 2);
        ShaderManager::CompileShader("model_quake1_mdl", i, false);

        Com_Printf("Compiling shaders %u/%u\n", (i * 2) + 2, RENDERMODE_MAX * 2);
        ShaderManager::CompileShader("model_quake1_mdl", i, true);
    }
    

    Material DefaultMaterial = LoadMaterialDefault();
    /*
    shader = LoadShader("shaders/surface.vs", "shaders/surface.np.fs");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    */

    Shader blurpostprocess = LoadShader(nullptr, "shaders/blur.fs");

    

    //SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    float GameTime = 0;

    SetCameraMode(Cam, CAMERA_FREE);

    if (argc > 1)
    {
        ::ViewerLoadModel(argv[1]);
    }

    bool Exit = false;

    // Main game loop
    while (!WindowShouldClose() && !Exit)    // Detect window close button or ESC key
    {
        ImGui_ImplRaylib_NewFrame();
        ImGui::NewFrame();
        ImGui_ImplRaylib_ProcessEvent();

#ifdef DEBUG
        ImGui::ShowDemoWindow();
#endif

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            //dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("MainDockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        ImGui::BeginMenuBar();
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Model"))
            {
                std::string InitialPath = GetWorkingDirectory();
                if (CurrentModel)
                {
                    InitialPath = CurrentModel->Path;
                }
                std::string result = FileIO::GetFileOpen(InitialPath, L"Quake/Hexen 2 Model (*.mdl, supports IDPO and RAPO)\0*.mdl\0\0", GetWindowHandle());

                if (result != "")
                {
                    ViewerLoadModel(result);
                }

            }
            if (ImGui::MenuItem("Exit"))
            {
                Exit = true;
            }
            ImGui::EndMenu();
        }
        if(ImGui::MenuItem("About"))
        {
            ImGui::OpenPopup("About");
        }
        bool unused_open = true;
        if (ImGui::BeginPopupModal("About", &unused_open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove))
        {
            // LUNA: Very Hacky™!!
            if (ImGui::BeginTable("aboutaligncenterlogo", 3, ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("emp", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("emp2", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("emp3", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TableNextColumn();
                ImGui::Image(LVTexture.id, ImVec2(LVTexture.width, LVTexture.height));
                ImGui::TableNextColumn();
                ImGui::EndTable();
            }
            if (ImGui::BeginTable("aboutaligncenter", 3, ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("emp", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("emp2", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("emp3", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TableNextColumn();
                ImGui::PushFont(fUtitle);
                ImGui::TextColored(ImVec4{ 250.f / 255.f, 68.f / 255.f, 117.f / 255.f, 1.f }, "LunarViewer");
                ImGui::PopFont();
                ImGui::TableNextColumn();
                ImGui::EndTable();
            }
            if (ImGui::BeginTable("aboutaligncenter2", 3, ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("emp", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("emp2", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("emp3", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TableNextColumn();
                ImGui::Text("Created and maintained by Luna Ryuko");
                ImGui::TableNextColumn();
                ImGui::EndTable();
            }
            
            if (ImGui::BeginTable("aboutaligncenter3", 3, ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("emp", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("emp2", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("emp3", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TableNextColumn();
                if (ImGui::Button("GitHub"))
                    OpenURL("https://github.com/LunaRyuko/LunarViewer");
                ImGui::TableNextColumn();
                ImGui::EndTable();
            }

            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::EndMenuBar();

        ImGuiID MainDockID = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(MainDockID, ImVec2(0,0), dockspace_flags);

        GameTime += GetFrameTime();


        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update variables / Implement example logic at this point
        //----------------------------------------------------------------------------------

        //bool AnyWindowFocused = ImGui::IsAnyItemHovered();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowDockID(MainDockID, ImGuiCond_FirstUseEver);
        ImGui::Begin("Viewport");
        if (DesiredViewportSize.x != ImGui::GetContentRegionAvail().x
            || DesiredViewportSize.y != ImGui::GetContentRegionAvail().y)
        {
            DesiredViewportSize = ImGui::GetContentRegionAvail();
            UnloadRenderTexture(Viewport);
            Viewport = LoadRenderTexture(DesiredViewportSize.x, DesiredViewportSize.y);

            SetTextureFilter(Viewport.texture, TEXTURE_FILTER_POINT);
        }
        ImGui::PushID("ViewportImage");
        ImGui::Image((ImTextureID)Viewport.texture.id, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::PopID();
        bool IsViewportFocused = ImGui::IsWindowHovered();

        ImGui::SetCursorPos(ImVec2(0.f,
            ImGui::GetWindowContentRegionMin().y + 8.0f));

        ImGui::Indent(ImGui::GetWindowContentRegionMin().x + 8.0f);

        /*ImGui::PushFont(fUtitle);
        ImGui::TextColored(ImVec4{ 250.f / 255.f, 68.f / 255.f, 117.f / 255.f, 1.f }, "LunarViewer");
        ImGui::PopFont();
        */
        if (CurrentModel)
        {
            ImGui::Text("%s", CurrentModel->Path.c_str());
            if (CurrentModel->HasRaylibMesh)
            {

            }
        }
    
        ImGui::Unindent(ImGui::GetWindowContentRegionMin().x + 8.0f);

        ImGui::End();
        ImGui::PopStyleVar();

        if (/*AnyWindowFocused && */IsViewportFocused)
        {
            UpdateCamera(&Cam);
        }

        if (IsKeyDown('Z')) Cam.target = Vector3 { 0.0f, 0.0f, 0.0f };
        if (IsKeyPressed('V')) GViewerSettings.UseVirtualResolution = !GViewerSettings.UseVirtualResolution;

        // Draw
        //----------------------------------------------------------------------------------

        ImGui::Begin("Colormap & Palette Debug", nullptr);
        ImGui::Image((ImTextureID)ColormapTexture.id, ImVec2(256, 64));
        ImGui::Image((ImTextureID)PaletteTexture.id, ImVec2(256, 6));
        
        // TODO: Add functionality to load custom colormaps and palettes
        /*
        ImGui::Button("Load Palette"); ImGui::SameLine();
        ImGui::Button("Load Colormap");
        */
        ImGui::End();

        ImGui::Begin("Rendering");
        ImGui::Text("Render Mode");
        ImGui::Indent();
        ImGui::RadioButton("Default", (int*)&GViewerSettings.RenderMode, 0);
        ImGui::RadioButton("Fullbright", (int*)&GViewerSettings.RenderMode, 1);
        ImGui::RadioButton("Show Normals", (int*)&GViewerSettings.RenderMode, 2);
        ImGui::RadioButton("Show UVs", (int*)&GViewerSettings.RenderMode, 3);
        ImGui::RadioButton("Show Indexed Texture", (int*)&GViewerSettings.RenderMode, 4);
        ImGui::Unindent();
        ImGui::Checkbox("Animation Interpolation", &GViewerSettings.UseAnimInterpolation);
        ImGui::Checkbox("Virtual Resolution (320x240) (V)", &GViewerSettings.UseVirtualResolution);
        if (ImGui::Button("Focus camera on world origin (Z)"))
        {
            Cam.target = Vector3{ 0.0f, 0.0f, 0.0f };
        }

        if (ImGui::BeginCombo("Palette", GetDirectoryForCurrentGame()))
        {
            if (ImGui::Selectable("quake1"))
            {
                GViewerSettings.CurrentGame = LunarViewerGame::Quake1;
                LoadColormap();
                if (CurrentModel)
                {
                    CurrentModel->ChangeRenderMode(CurrentModel->CurrentRenderMode, CurrentModel->UseAnimInterpolation, true);
                }
            }
            if (ImGui::Selectable("hexen2"))
            {
                GViewerSettings.CurrentGame = LunarViewerGame::Hexen2;
                LoadColormap();
                if (CurrentModel)
                {
                    CurrentModel->ChangeRenderMode(CurrentModel->CurrentRenderMode, CurrentModel->UseAnimInterpolation, true);
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Checkbox("Show Floor", &GViewerSettings.DrawFloor);
        ImGui::DragFloat("Floor Offset", &GViewerSettings.FloorOffset, 1.0f, 0.0, 0.0f, "%.3f");
        ImGui::End();

        if (CurrentModel)
        {
            CurrentModel->ChangeRenderMode(GViewerSettings.RenderMode, GViewerSettings.UseAnimInterpolation);
        }


        BeginDrawing();

        float screen[2];

        RenderTexture2D* CurrentViewport = nullptr;

        GL_PushGroupMarker(15, "Viewport Scene");
        
        if (GViewerSettings.UseVirtualResolution)
        {
            BeginTextureMode(Target);
            screen[0] = 160.f;
            screen[1] = 120.f;
            CurrentViewport = &Target;
        }
        else
        {
            BeginTextureMode(Viewport);
            screen[0] = (float)GetScreenWidth() / 2.0f;
            screen[1] = (float)GetScreenHeight() / 2.0f;
            CurrentViewport = &Viewport;
        }
        //SetShaderValue(shader, GetShaderLocation(shader, "screenSize"), screen, SHADER_UNIFORM_VEC2);

        //ClearBackground(DARKGRAY);
        ClearBackground(Color{ 0 });

        BeginMode3D(Cam);

        //DrawGrid(24, 1.0f * UnitScale);

        // We have a model loaded!
        if (CurrentModel && CurrentModel->IsValid()/* && CurrentModel->HasRaylibMesh*/)
        {
            //DrawSphereWires(CurrentModel->MDLHeader.EyePosition, 1.f * UnitScale, 7, 12, RED);
            //rlDrawRenderBatchActive();

            //rlEnableWireMode();

            Vector3 planeCenter = {
                0,
                GViewerSettings.FloorOffset,
                0
            };

            if (CurrentModel->HasRaylibMesh)
            {
                bool HasFloor = GViewerSettings.DrawFloor;
                CurrentModel->Frame(GetFrameTime(), GViewerSettings.AnimBegin, GViewerSettings.AnimEnd);
                SetShaderValue(CurrentModel->RShader, GetShaderLocation(CurrentModel->RShader, "screenSize"), screen, SHADER_UNIFORM_VEC2);
                SetShaderValue(CurrentModel->RShader, GetShaderLocation(CurrentModel->RShader, "floorOffset"), &GViewerSettings.FloorOffset, SHADER_UNIFORM_FLOAT);
                float lightOffset = 0.0f;
                SetShaderValue(CurrentModel->RShader, GetShaderLocation(CurrentModel->RShader, "lightOffset"), &lightOffset, SHADER_UNIFORM_FLOAT);
                int isMirrored = 0;
                SetShaderValue(CurrentModel->RShader, GetShaderLocation(CurrentModel->RShader, "isMirrored"), &isMirrored, SHADER_UNIFORM_INT);

                int isAlphaTested = CurrentModel->MDLHeader.Flags & EF_HOLEY ? 1 : 0;
                SetShaderValue(CurrentModel->RShader, GetShaderLocation(CurrentModel->RShader, "isAlphaTested"), &isAlphaTested, SHADER_UNIFORM_INT);
                rlDisableBackfaceCulling();
                CurrentModel->DrawModel();

                Matrix root = MatrixIdentity();
                root = MatrixMultiply(root, MatrixRotate(Vector3{ 0, 1, 0 }, GetTime()));
                //root = MatrixMultiply(root, MatrixTranslate(0.0f, GViewerSettings.FloorOffset, 0.0f));

                //glEnable(GL_ALPHA_TEST);
                if (HasFloor)
                {
                    glEnable(GL_STENCIL_TEST);
                    glEnable(GL_CLIP_DISTANCE0);
                }
                glStencilMask(0xFF);
                glStencilFunc(GL_ALWAYS, 0, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

                DrawMesh(CurrentModel->RMesh, CurrentModel->RMaterial, root);

                if (HasFloor)
                {
                    glDisable(GL_CLIP_DISTANCE0);

                    rlEnableBackfaceCulling();

                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glStencilFunc(GL_ALWAYS, 1, 0xFF);
                    glDepthMask(GL_FALSE);
                    DrawPlane(planeCenter, Vector2{ 128, 128 }, Color{ 32,32,32, 255 });
                    rlDrawRenderBatchActive();
                    glDepthMask(GL_TRUE);
                    glStencilFunc(GL_ALWAYS, 0, 0xFF);
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

                    rlDisableBackfaceCulling();


                    GL_PushGroupMarker(17, "Floor Reflection");

                    // TODO: Make a shader variant specifically for shadows
                    //Matrix shadowMatrix = MatrixMultiply(root, MatrixScale(1.0f, 0.0f, 1.0f));
                    //shadowMatrix = MatrixMultiply(shadowMatrix, MatrixTranslate(0.0f, GViewerSettings.FloorOffset, 0.0f));

                    glEnable(GL_CLIP_DISTANCE1);

                    Matrix reflectionMatrix = MatrixMultiply(root, MatrixScale(1.0f, -1.0f, 1.0f));
                    reflectionMatrix = MatrixMultiply(reflectionMatrix, MatrixTranslate(0.0f, GViewerSettings.FloorOffset * 2, 0.0f));
                    glStencilFunc(GL_EQUAL, 1, 0xFF);
                    lightOffset = -0.0f;
                    SetShaderValue(CurrentModel->RShader, GetShaderLocation(CurrentModel->RShader, "lightOffset"), &lightOffset, SHADER_UNIFORM_FLOAT);
                    isMirrored = 1;
                    SetShaderValue(CurrentModel->RShader, GetShaderLocation(CurrentModel->RShader, "isMirrored"), &isMirrored, SHADER_UNIFORM_INT);
                    DrawMesh(CurrentModel->RMesh, CurrentModel->RMaterial, reflectionMatrix);
                    glStencilFunc(GL_ALWAYS, 0, 0xFF);

                    glDisable(GL_CLIP_DISTANCE1);
                    glDisable(GL_STENCIL_TEST);


                    rlEnableBackfaceCulling();

                    glDepthMask(GL_FALSE);
                    BeginBlendMode(BLEND_ALPHA);
                    //rlSetBlendFactors(GL_SRC_ALPHA, GL_SRC_ALPHA, GL_FUNC_ADD);
                    DrawPlane(planeCenter, Vector2{ 128, 128 }, Color{ 32, 32, 32, 192 });
                    rlDrawRenderBatchActive();
                    EndBlendMode();
                    glDepthMask(GL_TRUE);
                }

                EndMode3D();
                // LUNA: This was a blur postprocess effect for the floor. dunno if its desired
                if (false)
                {
                    GenTextureMipmaps(&((*CurrentViewport).texture));
                    SetTextureFilter((*CurrentViewport).texture, TEXTURE_FILTER_POINT);
                    BeginShaderMode(blurpostprocess);
                    glEnable(GL_STENCIL_TEST);
                    glStencilMask(0xFF);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                    glStencilFunc(GL_EQUAL, 1, 0xFF);
                    BeginBlendMode(BLEND_CUSTOM);
                    rlSetBlendFactors(GL_SRC_COLOR, GL_DST_COLOR, GL_MAX);
                    //glBlendColor(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    DrawTexture((*CurrentViewport).texture, 0, 0, WHITE);
                    //DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
                    rlDrawRenderBatchActive();
                    glBlendColor(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    EndBlendMode();
                    glStencilFunc(GL_ALWAYS, 0, 0xFF);
                    glDisable(GL_STENCIL_TEST);
                    EndShaderMode();
                }

                GL_PopGroupMarker();
            }
            else 
            {
                EndMode3D();
            }
        }
        else 
        {
            DrawGrid(32, 4.f);
            EndMode3D();
        }

        if (ImGui::Begin("Model"))
        {
            if (CurrentModel && CurrentModel->IsValid())
            {
                ImGui::Text("%d %d %d %d", CurrentModel->AnimData.CurrentFrame,
                    CurrentModel->AnimData.TargetFrame,
                    CurrentModel->AnimData.FrameCount,
                    CurrentModel->AnimData.Interpolate);
                if (ImGui::TreeNode("Header"))
                {
                    ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;

                    if (ImGui::BeginTable("HeaderTable", 2, flags))
                    {
                        FMDLHeader* Header = &CurrentModel->MDLHeader;

                        char* magic = (char*)&Header->Info.Magic;

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("Magic"); ImGui::TableNextColumn();
                        ImGui::Text("%c%c%c%c", magic[0], magic[1], magic[2], magic[3]);

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("Version"); ImGui::TableNextColumn();
                        ImGui::Text("%u", Header->Info.Version);

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("Scale"); ImGui::TableNextColumn();
                        ImGui::Text("{%f, %f, %f}", Header->Scale[0], Header->Scale[1], Header->Scale[2]);

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("Translate"); ImGui::TableNextColumn();
                        ImGui::Text("{%f, %f, %f}", Header->Translate[0], Header->Translate[1], Header->Translate[2]);

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("Bounding Radius"); ImGui::TableNextColumn();
                        ImGui::Text("%f", Header->BoundingRadius);

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("EyePosition"); ImGui::TableNextColumn();
                        ImGui::Text("{%f, %f, %f}", Header->EyePosition.x, Header->EyePosition.y, Header->EyePosition.z);

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("Skins"); ImGui::TableNextColumn();
                        ImGui::Text("%u", Header->NumSkins);

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("Texture Resolution"); ImGui::TableNextColumn();
                        ImGui::Text("%ux%u (%u bytes)", Header->SkinWidth, Header->SkinHeight, CurrentModel->GetSkinByteLength());

                        if (CurrentModel->HasRaylibMesh)
                        {
                            ImGui::TableNextRow(); ImGui::TableNextColumn();
                            ImGui::Text("Vertices"); ImGui::TableNextColumn();
                            ImGui::Text("%u (%u rendered)", Header->NumVerts, CurrentModel->RMesh.vertexCount);
                            HelpMarker("Due to the way UVs are calculated, the vertices need to be unique for every triangle. Fortunately, this does not contribute to the vertex limit in the engine.");

                            ImGui::TableNextRow(); ImGui::TableNextColumn();
                            ImGui::Text("Triangles"); ImGui::TableNextColumn();
                            ImGui::Text("%u", Header->NumTris);
                        }
                        else
                        {
                            ImGui::TableNextRow(); ImGui::TableNextColumn();
                            ImGui::Text("Vertices"); ImGui::TableNextColumn();
                            ImGui::Text("%u", Header->NumVerts);

                            ImGui::TableNextRow(); ImGui::TableNextColumn();
                            ImGui::Text("Triangles"); ImGui::TableNextColumn();
                            ImGui::Text("%u", Header->NumTris);
                        }

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("Frames"); ImGui::TableNextColumn();
                        ImGui::Text("%u", Header->NumFrames);

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("Synchronization Type"); ImGui::TableNextColumn();
                        ImGui::Text("%u", Header->SyncType);

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("Flags"); ImGui::TableNextColumn();
                        ImGui::Text("%u", Header->Flags);

                        ImGui::TableNextRow(); ImGui::TableNextColumn();
                        ImGui::Text("Size"); ImGui::TableNextColumn();
                        ImGui::Text("%f", Header->Size);

                        ImGui::EndTable();
                    }

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Flags"))
                {
                    ImGui::PushID("Flags");
                    for (uint32_t i = 0; i < 31; i++)
                    {
                        ImGui::PushID(i);
                        uint32_t flagVal = (1 << i);
                        const char* FlagName = GetDescriptionForFlagHexen2(flagVal);
                        size_t nameLen = strlen(FlagName);

                        // Discard if name is null
                        if (nameLen <= 0)
                        {
                            ImGui::PopID();
                            continue;
                        }
                        const char* FlagDesc = FlagName + nameLen + 1;

                        ImGui::CheckboxFlags(FlagName, &(CurrentModel->MDLHeader.Flags), flagVal);
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();
                            ImGui::Text("%s", FlagDesc);
                            ImGui::EndTooltip();
                        }

                        ImGui::PopID();
                    }
                    ImGui::PopID();
                    ImGui::TreePop();
                }

                uint32_t ImageIndex = 0;
                if (ImGui::TreeNode("Skins"))
                {
                    ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;

                    for (uint32_t i = 0; i < CurrentModel->MDLHeader.NumSkins; i++)
                    {
                        ImGui::PushID(TextFormat("Skin%i", i));
                        ImGui::Text("Skin %u", i);
                        //ImGui::Image((ImTextureID)CurrentModel->_textures[i].id, ImVec2(CurrentModel->MDLHeader.SkinWidth, CurrentModel->MDLHeader.SkinHeight));
                        ImageViewerWithZoom(CurrentModel->_textures[i]);
                        ImageIndex++;
                        ImGui::PopID();
                    }

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Mesh Data"))
                {
                    ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;

                    if (ImGui::TreeNode("Triangles"))
                    {
                        if (ImGui::BeginTable("TriTable", 2, flags))
                        {
                            FMDLHeader* Header = &CurrentModel->MDLHeader;

                            for (uint32_t v = 0; v < CurrentModel->MDLHeader.NumTris; v++)
                            {
                                auto tri = &CurrentModel->Triangles[v];

                                ImGui::TableNextRow(); ImGui::TableNextColumn();
                                ImGui::Text("%u", tri->IsFrontFace); ImGui::TableNextColumn();
                                ImGui::Text("%u %u %u", tri->Vertex[0], tri->Vertex[1], tri->Vertex[2]);
                            }

                            ImGui::EndTable();
                        }

                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Vertices"))
                    {
                        if (ImGui::BeginTable("VertTable", 2, flags))
                        {
                            FMDLHeader* Header = &CurrentModel->MDLHeader;

                            for (uint32_t v = 0; v < CurrentModel->MDLHeader.NumVerts; v++)
                            {
                                auto vert = &CurrentModel->Vertices[v];

                                ImGui::TableNextRow(); ImGui::TableNextColumn();
                                ImGui::Text("%hhu %hhu %hhu", vert->Position[0], vert->Position[1], vert->Position[2]);  ImGui::TableNextColumn();
                                ImGui::Text("%hhu", vert->NormalIndex);
                            }

                            ImGui::EndTable();
                        }

                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Frames"))
                    {
                        ImGui::PushID("Frames");
                        for (uint32_t v = 0; v < CurrentModel->MDLHeader.NumFrames; v++)
                        {
                            ImGui::PushID(v);
                            FMDLFrameBase *frame = CurrentModel->Frames[v];
                            FMDLSimpleFrame* simpframe = reinterpret_cast<FMDLSimpleFrame*>(frame);

                            if (frame->Type == 0)
                            {
                                if (ImGui::TreeNode(TextFormat("%u (simple) - %s", v, simpframe->Name)))
                                {
                                    ImGui::LabelText("Bounding Box Min", "%hhu %hhu %hhu",
                                        simpframe->BBoxMin.Position[0],
                                        simpframe->BBoxMin.Position[1],
                                        simpframe->BBoxMin.Position[2]);

                                    ImGui::LabelText("Bounding Box Max", "%hhu %hhu %hhu",
                                        simpframe->BBoxMax.Position[0],
                                        simpframe->BBoxMax.Position[1],
                                        simpframe->BBoxMax.Position[2]);

                                    ImGui::TreePop();
                                }
                            }
                            else
                            {
                                if (ImGui::TreeNode(TextFormat("%u (group, not supported yet)", v)))
                                {
                                    ImGui::TreePop();
                                }
                            }

                            ImGui::PopID();
                        }
                        ImGui::PopID();
                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }

                ImGui::Text("Simple Frame Vertex Position Data");
                ImGui::Text("%ux%u", CurrentModel->SimpleFrameTexture.width, CurrentModel->SimpleFrameTexture.height);
                ImageViewerWithZoom(CurrentModel->SimpleFrameTexture);
            }
        }
        ImGui::End();

        ImGui::Begin("Animation");

        if (CurrentModel)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
            float ProgPosY = ImGui::GetCursorPos().y;
            float ProgPosX = ImGui::GetCursorPos().x;
            ImGui::ProgressBar(0.f, ImVec2(-FLT_MIN, 0), "");
            float ProgSizeX = ImGui::GetItemRectSize().x;
            float ProgSizeY = ImGui::GetItemRectSize().y;
            float ProgWidth = (float)(((GViewerSettings.AnimEnd + 1) - GViewerSettings.AnimBegin) / (float)(CurrentModel->MDLHeader.NumFrames)) * ProgSizeX;
            float ProgStep = (float)((1.f) / (float)(CurrentModel->MDLHeader.NumFrames)) * ProgSizeX;
            float ProgX = (float)(GViewerSettings.AnimBegin / (float)(CurrentModel->MDLHeader.NumFrames)) * ProgSizeX;
            float GrabberWidth = 4.f;
            ImGui::SetCursorPos(ImVec2(ProgPosX + ProgX, ProgPosY));
            ImGui::ProgressBar((float)(CurrentModel->AnimData.TargetFrame - GViewerSettings.AnimBegin) / (float)(GViewerSettings.AnimEnd - GViewerSettings.AnimBegin),
                ImVec2(ProgWidth, 0), "");

            ImGui::PopStyleVar();

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            /*
            for (int i = 0; i < CurrentModel->MDLHeader.NumFrames; i++)
            {
                draw_list->AddLine(ImVec2(canvas_pos.x + (i * ProgStep), canvas_pos.y - 2), ImVec2(canvas_pos.x + (i * ProgStep), canvas_pos.y + ProgSizeY), IM_COL32(255, 255, 255, 255), 1.f);
            }
            */

            /*
            ImVec2 dragDelta = ImVec2(0, 0);

            ImGui::SetCursorPos(ImVec2(ProgPosX + ProgX, ProgPosY));
            ImGui::Button("", ImVec2(ProgStep, ProgSizeY));
            static int SavedBegin = 0;
            static int SavedEnd = 0;
            static int FrameOffset = 0;
            static bool Saved = false;
            if (ImGui::IsMouseDragging(0, 1.f) && ImGui::IsItemActive())
            {
                if (!Saved)
                {
                    SavedBegin = GViewerSettings.AnimBegin;
                    SavedEnd = GViewerSettings.AnimEnd;
                    Saved = true;
                }
                dragDelta = ImGui::GetMouseDragDelta(0, 1.0f);
                int FrameOffset = dragDelta.x / ProgStep;
                GViewerSettings.AnimBegin = Clamp(SavedBegin + FrameOffset, 0.f, (float)(CurrentModel->MDLHeader.NumFrames - 1));
            }
            else
            {
                Saved = false;
            }
            */
        }

        ImGui::BeginTable("Animation Stuff", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV);
        ImGui::TableNextColumn();
        if (CurrentModel)
        {
            // true if value got changed
            if (ImGui::Combo("Current Anim", &GViewerSettings.CurrentAnimIndex, Anims_ArrayGetter, (void*)(&CurrentModel->AnimationSets), CurrentModel->AnimationSets.size()))
            {
                FMDLAnimation *anim = &CurrentModel->AnimationSets[GViewerSettings.CurrentAnimIndex];

                GViewerSettings.AnimBegin = anim->Begin;
                GViewerSettings.AnimEnd = anim->End;
            }

            ImGui::DragIntRange2("Anim Range", &GViewerSettings.AnimBegin, &GViewerSettings.AnimEnd, 1.0f, 0, CurrentModel->MDLHeader.NumFrames-1, "%u", "%u", ImGuiSliderFlags_AlwaysClamp );

            /*
            ImGui::Text("%d %d %d %d", CurrentModel->AnimData.CurrentFrame,
                CurrentModel->AnimData.TargetFrame,
                CurrentModel->AnimData.FrameCount,
                CurrentModel->AnimData.Interpolate);*/


            ImGui::TableNextColumn();

            ImGui::Text("Frames: %u", CurrentModel->MDLHeader.NumFrames);
        }
        else
        {
            ImGui::Combo("Current Anim", &GViewerSettings.CurrentAnimIndex, "Load a model first");
            ImGui::DragIntRange2("Anim Range", &GViewerSettings.AnimBegin, &GViewerSettings.AnimEnd, 1.0f, 0, 0, "%u", "%u", ImGuiSliderFlags_AlwaysClamp);

            ImGui::TableNextColumn();

            ImGui::Text("No model");
        }

        ImGui::EndTable();

        ImGui::End();

        if (GViewerSettings.UseVirtualResolution)
        {
            GL_PushGroupMarker(22, "Upscale low-res scene");
            BeginTextureMode(Viewport);

            //ClearBackground(DARKGRAY);

            ClearBackground(Color{ 0 });

            screenHeight = GetScreenHeight();

            float aspectRatio = 320.f / 240.f;
            float targetWidth = (float)screenHeight * aspectRatio;

            float wo = GetScreenWidth() - targetWidth;
            float ho = 0;
            BeginBlendMode(BLEND_CUSTOM);

            rlSetBlendFactors(GL_SRC_COLOR, GL_SRC_COLOR, GL_MAX);

            DrawTexturePro(
                Target.texture,
                Rectangle{ 0, 0, (float)Target.texture.width, (float)-Target.texture.height },
                Rectangle{ wo / 2.f, ho / 2.f, targetWidth, (float)GetScreenHeight() },
                Vector2{ 0,0 },
                0,
                WHITE
            );

            EndBlendMode();
            GL_PopGroupMarker();
        }
        else
        {
            /*
            BeginBlendMode(BLEND_CUSTOM);

            rlSetBlendFactors(GL_SRC_ALPHA, GL_SRC_ALPHA, GL_MAX);

            DrawTexture(
                Target.texture,
                0, 0,
                WHITE
            );

            EndBlendMode();
            */
        }

        /*
        DrawRectangle(0, 0, 355, 64, Color{ 0,0,0,64 });
        DrawText("LunarViewer",  4, 2, 40, Color{ 250, 66, 117, 255 });
        char VAPrints[1024];
        sprintf(VAPrints, "Virtual Resolution (%dx%d): %s", 320, 240, GViewerSettings.UseVirtualResolution ? "ON" : "OFF");
        DrawText(VAPrints, 4, 44, 20, WHITE);
        if (CurrentModel)
        {
            sprintf(VAPrints, "%s", CurrentModel->Path.c_str());
            //int width = GetFontDefault().chars[3].advanceX * TextLength(VAPrints);
            //DrawRectangle(0, 64, width + 8, 24, Color{ 0,0,0,64 });
            DrawText(VAPrints, 4 + 2, 44 + 22 +2, 20, BLACK);
            DrawText(VAPrints, 4, 44+22, 20, WHITE);
        }
        */

        EndTextureMode();

        GL_PopGroupMarker();

        ImGui::End();

        GL_PushGroupMarker(13, "ImGui Render");

        ImGui::Render();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        ImGui_ImplRaylib_Render(ImGui::GetDrawData());

        GL_PopGroupMarker();

        // TODO: Draw everything that requires to be drawn at this point:

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // TODO: Unload all loaded resources at this point

    ImGui_ImplRaylib_Shutdown();
    ImGui::DestroyContext();

    UnloadShader(shader);

    Com_Delete(CurrentModel);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    PHYSFS_deinit();

    CheckForMemoryLeaks();

    return 0;
}