#ifndef IMGUI_IMPL_RAYLIB
#define IMGUI_IMPL_RAYLIB

/*	NOTE: I've added a few macros to deal with compatibility.
	Most are turned off by default to keep it as close as possible to the original code,
	but ImGui has its quirks when it comes to GL2. I'll be integrating part of WEREMSOFT's rendering code.

	(https://github.com/WEREMSOFT/c99-raylib-cimgui-template/).
*/

//	The compiler whines about IMGUI_IMPL_API not being defined, so I'm leaving this here.
#include "imgui.h"

//	Config macros
#include "imgui_impl_raylib_config.h"

#if defined(__cplusplus)
extern "C" {
#endif

	IMGUI_IMPL_API bool     ImGui_ImplRaylib_Init();
	IMGUI_IMPL_API void     ImGui_ImplRaylib_Shutdown();
	IMGUI_IMPL_API void     ImGui_ImplRaylib_NewFrame();
	IMGUI_IMPL_API bool     ImGui_ImplRaylib_ProcessEvent();

#ifdef COMPATIBILITY_MODE
	IMGUI_IMPL_API void     ImGui_ImplRaylib_LoadDefaultFontAtlas();
	IMGUI_IMPL_API void     ImGui_ImplRaylib_Render(ImDrawData* draw_data);
#endif

#if defined(__cplusplus)
}
#endif

#endif
