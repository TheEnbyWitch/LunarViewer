workspace "LunarViewer"
	configurations { "Debug","Debug.DLL", "Release", "Release.DLL" }
	platforms { "x64"}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		
	filter "configurations:Debug.DLL"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"	
		
	filter "configurations:Release.DLL"
		defines { "NDEBUG" }
		optimize "On"	
		
	filter { "platforms:x64" }
		architecture "x86_64"
		
	targetdir "bin/%{cfg.buildcfg}/"
	
	defines{"PLATFORM_DESKTOP", "GRAPHICS_API_OPENGL_33"}
		
project "raylib"
		filter "configurations:Debug.DLL OR Release.DLL"
			kind "SharedLib"
			defines {"BUILD_LIBTYPE_SHARED"}
			
		filter "configurations:Debug OR Release"
			kind "StaticLib"
			
		filter "action:vs*"
			defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS", "_WIN32"}
			links {"winmm"}
			
		filter "action:gmake*"
			links {"pthread", "GL", "m", "dl", "rt", "X11"}
			
		filter{}
		
		location "build"
		language "C++"
		targetdir "bin/%{cfg.buildcfg}"
		cppdialect "C++17"
		
		includedirs { "raylib/src", "raylib/src/external/glfw/include"}
		vpaths 
		{
			["Header Files"] = { "raylib/src/**.h"},
			["Source Files/*"] = {"raylib/src/**.c"},
		}
		files {"raylib/src/*.h", "raylib/src/*.c"}
		
project "LunarViewer"
	kind "ConsoleApp"
	location "src"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	cppdialect "C++17"
	
	includedirs {"src"}
	vpaths 
	{
		["Header Files"] = { "**.h"},
		["Source Files"] = {"**.c", "**.cpp"},
	}
	files {"src/**.c", "src/**.cpp", "src/**.h"}

	links {"raylib", "physfs"}
	libdirs {"glslang/lib"}
	
	includedirs { "src", "raylib/src", "physfs", "glslang/include" }
	defines{"PLATFORM_DESKTOP", "GRAPHICS_API_OPENGL_33"}
	
	filter "action:vs*"
		defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS", "_WIN32"}
		dependson {"raylib", "physfs"}
		links {"winmm", "raylib.lib", "kernel32"}
		libdirs {"bin/%{cfg.buildcfg}"}
		
	filter "action:gmake*"
		links {"pthread", "GL", "m", "dl", "rt", "X11"}

	filter "configurations:Debug.DLL OR Debug"
		links {"glslangd", "MachineIndependentd", "GenericCodeGend", "OSDependentd", "OGLCompilerd"}
			
	filter "configurations:Release.DLL OR Release"
		links {"glslang", "MachineIndependent", "GenericCodeGen", "OSDependent", "OGLCompiler"}

project "physfs"
	kind "StaticLib"
		
	location "build"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	cppdialect "C++17"

	vpaths 
	{
		["Header Files"] = { "physfs/**.h"},
		["Source Files/*"] = {"physfs/**.c", "physfs/**.cpp"},
	}
	files {"physfs/*.h", "physfs/*.c", "physfs/*.cpp"}