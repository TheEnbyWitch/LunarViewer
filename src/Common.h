#pragma once

#include <iostream>
#include <vector>

#ifdef _DEBUG
// LUNA: This is slow. I did not write this with the intent of it being performant, only to see where I forgot to free memory
#define TRACK_MEMORY 1
#endif

constexpr int Int32Magic(const char* a)
{
	return ((a[3] << 24) + (a[2] << 16) + (a[1] << 8) + a[0]);
}

enum
{
	ERR_NONE,			/* Just print to console */
	ERR_DIALOG,
	ERR_FATAL,			/* Kill the app */
};


char* va(const char* fmt, ...);

void Com_Printf(const char* fmt, ...);

// LUNA: When I first wrote this code in 2017, I did not think that it would still cause issues in 2021
// Thankfully, C++20 supports it, but you need to use an experimental preprocessor in VS
// Also, GNU++17 supports this!
#if __cplusplus >= 202002L || (defined(__GNUC__))

#define Com_Error(level, fmt, ...) \
	Com_ErrorEx(level, va("%s:%d -> %s()", __FILE__, __LINE__, __FUNCTION__), va(fmt __VA_OPT__(,) __VA_ARGS__));

#elif defined(_MSC_VER)

// Works reliably on MSVC, causes compile errors on Linux! Fun!
#define Com_Error(level, fmt, ...) \
	Com_ErrorEx(level, va("%s:%d -> %s()", __FILE__, __LINE__, __FUNCTION__), va(fmt, __VA_ARGS__));
/*
#elif defined(__GNUC__)

// The GCC way
#define Com_Error(level, fmt, args...) \
	Com_ErrorEx(level, va("%s:%d -> %s()", __FILE__, __LINE__, __FUNCTION__), va(fmt, args));
	*/
#else 

#error "No __VA_ARGS__ support for when no variadic arguments are specified!"

#endif

void Com_ErrorEx(int level, char* source, char* msg);

void GL_PushGroupMarker(uint32_t length, const char* group);

void GL_PopGroupMarker();

enum LunarRenderMode : uint32_t
{
	RENDERMODE_DEFAULT = 0,			// Default, includes lighting
	RENDERMODE_FULLBRIGHT,			// Fullbright
	RENDERMODE_NORMALS,				// Shows model's normals
	RENDERMODE_UVS,					// Shows model's UVs
	RENDERMODE_INDEXED_TEXTURE,		// Shows model's texture without the palette


	RENDERMODE_MAX
};

enum class LunarViewerGame : uint32_t
{
	Quake1 = 0,
	Hexen2
};

struct FViewerSettings
{
	LunarViewerGame CurrentGame = LunarViewerGame::Quake1;

	bool UseVirtualResolution = false;
	uint32_t RenderMode = 0;
	bool UseAnimInterpolation = false;
	int AnimBegin = 0;
	int AnimEnd = 0;
	int CurrentAnimIndex = 0;

	bool DrawFloor = true;
	float FloorOffset = 0.0f;
};

extern FViewerSettings GViewerSettings;


struct FMemTrackCallInfo
{
	uint32_t Check = 0;
	int32_t Line = 0;
	std::string File = "";
	std::string Function = "";
};

struct FMemTrack
{
	std::string name;
	std::string raw_name;
	size_t size;

	FMemTrackCallInfo call_info;
};

void AddMemToTracker(void* ptr, FMemTrack mem);
void FreeMemFromTracker(void* ptr);

void CheckForMemoryLeaks();

template<class T>
inline T* Com_MallocEx(size_t Size, FMemTrackCallInfo CallInfo)
{
	void* ptr = malloc((size_t)Size);
#ifdef TRACK_MEMORY
	//Com_Printf("[Malloc] %s (%s), size: %u\n", typeid(T).name(), typeid(T).raw_name(), (size_t)Size);
	if (ptr)
	{
		AddMemToTracker(ptr, FMemTrack{ typeid(T).name(),
#if defined(_MSC_VER)
			typeid(T).raw_name()
#else
			""
#endif
			, Size, CallInfo });
	}
#endif
	return (T*)ptr;
}

template<class T>
inline T* Com_CallocEx(size_t ElementCount, FMemTrackCallInfo CallInfo)
{
	void* ptr = calloc((size_t)ElementCount, sizeof(T));
#ifdef TRACK_MEMORY
	//Com_Printf("[Calloc] %s (%s), size: %u, count: %u\n", typeid(T).name(), typeid(T).raw_name(), sizeof(T), ElementCount);
	if (ptr)
	{
		AddMemToTracker(ptr, FMemTrack{ typeid(T).name(),
#if defined(_MSC_VER)
			typeid(T).raw_name()
#else
			""
#endif
			, ElementCount * sizeof(T), CallInfo });
	}
#endif
	return (T*)ptr;

}

template<class T>
inline T* Com_ReallocEx(T* Element, size_t ElementSize, FMemTrackCallInfo CallInfo)
{
	void* ptr = realloc((void*)Element, (size_t)ElementSize);
#ifdef TRACK_MEMORY
	if (ptr)
	{
		AddMemToTracker(ptr, FMemTrack{ typeid(T).name(),
#if defined(_MSC_VER)
			typeid(T).raw_name()
#else
			""
#endif
			, ElementSize, CallInfo });
		FreeMemFromTracker(Element);
	}
#endif 
	return (T*)ptr;
}
template<class T>
inline void Com_FreeEx(T* Element, FMemTrackCallInfo CallInfo)
{
	free((void*)Element);
#ifdef TRACK_MEMORY
	FreeMemFromTracker((void*)Element);
#endif
}

template<class T>
inline T* Com_Malloc_NoInfo(size_t Size)
{
	return Com_MallocEx<T>(Size, FMemTrackCallInfo{ 0 });
}

template<class T>
inline T* Com_Calloc_NoInfo(size_t ElementCount)
{
	return Com_CallocEx<T>(ElementCount, FMemTrackCallInfo{ 0 });
}

template<class T>
inline T* Com_Realloc_NoInfo(T* Element, size_t ElementSize)
{
	return Com_ReallocEx<T>(Element, ElementSize, FMemTrackCallInfo{ 0 });
}

template<class T>
inline void Com_Free_NoInfo(T* Element)
{
	Com_FreeEx<T>(Element, FMemTrackCallInfo{ 0 });
}



#define Com_Malloc(T, Size) \
	Com_MallocEx<T>((size_t)Size, FMemTrackCallInfo{ (uint32_t)1, __LINE__, __FILE__, __FUNCTION__})

#define Com_Calloc(T, Count) \
	Com_CallocEx<T>((size_t)Count, FMemTrackCallInfo{ (uint32_t)1, __LINE__, __FILE__, __FUNCTION__})

#define Com_Realloc(T, Element, Size) \
	Com_ReallocEx<T>(Element, (size_t)Size, FMemTrackCallInfo{ (uint32_t)1, __LINE__, __FILE__, __FUNCTION__})

#define Com_Free(Element) \
	Com_FreeEx(Element, FMemTrackCallInfo{ (uint32_t)1, __LINE__, __FILE__, __FUNCTION__})

#ifdef TRACK_MEMORY

#define Com_New(T, ...) \
	([__VA_ARGS__]() -> T* { T* ptr = new T(__VA_ARGS__); \
	if(ptr) { AddMemToTracker(ptr, FMemTrack{ typeid(T).name(), "", sizeof(T), FMemTrackCallInfo{ (uint32_t)1, __LINE__, __FILE__, __FUNCTION__} }); } \
	return ptr; })()

#define Com_Delete(Element) \
	{ if(Element) { FreeMemFromTracker(Element); } delete Element; }

#else

#define Com_New(T, ...) \
	new T(__VA_ARGS__)

#define Com_Delete(Element) \
	delete Element


#endif

// LUNA: use Com functions pls
#define malloc(x) Use_Com_Malloc //Com_Malloc(void*, x)
#define calloc(count,size) Use_Com_Calloc //Com_Malloc(void*, count * size)
#define realloc Use_Com_Realloc
#define free Use_Com_Free //Com_Free(x)
