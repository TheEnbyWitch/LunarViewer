#include <iostream>
#include <cstdlib>
#include <stdarg.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <signal.h>
#endif 

#include "Common.h"
#include "external/glad.h"

char* va(const char* fmt, ...)
{
	static int index = 0;
	static char string[4][1024];	// in case called by nested functions
	char* result;
	result = string[index];
	index = (index + 1) & 3;
	va_list args;
	va_start(args, fmt);
	vsprintf(result, fmt, args);
	va_end(args);
	return result;
}

void Com_Printf(const char* fmt, ...)
{
	static int index = 0;
	static char string[4][1024];	// in case called by nested functions
	char* result;
	result = string[index];
	index = (index + 1) & 3;
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void GL_PushGroupMarker(uint32_t length, const char* group)
{
#ifdef DEBUG
	if(glPushGroupMarkerEXT)
		glPushGroupMarkerEXT(length, group);
#endif
}

void GL_PopGroupMarker()
{
#ifdef DEBUG
	if(glPopGroupMarkerEXT)
		glPopGroupMarkerEXT();
#endif
}

void Com_ErrorEx(int level, char* source, char* msg)
{
	printf("ERROR: %s\n%s\n", source, msg);

	if (level > ERR_NONE)
	{
#ifdef _WIN32
		MessageBoxA(nullptr, va("%s\n%s%s", source, msg, level == ERR_FATAL ? "\n\nThe app will now close" : ""), "Error", MB_ICONERROR | MB_OK);
#endif
	}

#ifdef _WIN32
	if (IsDebuggerPresent())
#endif
	{
		if (level > ERR_NONE)
		{
			// if we're running with a debugger, break
#ifdef _WIN32
			__debugbreak(); 
#else
			raise(SIGTRAP);
#endif
		}
	}
//#endif

	if (level == ERR_FATAL)
	{
		exit(-1);
	}
}

#if defined(TRACK_MEMORY)
#include <unordered_map>

std::unordered_map<void*, FMemTrack> Memory;
#endif

void AddMemToTracker(void* ptr, FMemTrack mem)
{
#if defined(TRACK_MEMORY)
	if (mem.call_info.Check == 0)
	{
		// LUNA: dont track allocations without call info
		return;
	}
	Com_Printf("[MemTracker] Allocated %u bytes of memory of type %s (%s)\n", mem.size, mem.name.c_str(), mem.raw_name.c_str());
	Memory.emplace(std::make_pair(ptr, mem));
#endif
}

void FreeMemFromTracker(void* ptr)
{
#if defined(TRACK_MEMORY)
	auto search = Memory.find(ptr);
	if (search != Memory.end())
	{
		Com_Printf("[MemTracker] Freed %u bytes of memory of type %s (%s)\n", Memory[ptr].size, Memory[ptr].name.c_str(), Memory[ptr].raw_name.c_str());
		Memory.erase(ptr);
	}
#endif
}

void CheckForMemoryLeaks()
{
#if defined(TRACK_MEMORY)
	bool foundLeak = false;
	for (auto kvp : Memory)
	{
		if (kvp.second.call_info.Check != 0)
		{
			Com_Printf("[MemTracker] Memory leaked! %s:%d -> %s() = %s\n",
				kvp.second.call_info.File.c_str(),
				kvp.second.call_info.Line,
				kvp.second.call_info.Function.c_str(),
				kvp.second.name.c_str());
		}
		else
		{
			Com_Printf("[MemTracker] Memory leaked! There was no Call Info associated with this allocation. %s\n", kvp.second.name.c_str());
		}
		foundLeak = true;
	}

	if (!foundLeak)
	{
		Com_Printf("No memory leaks found! You're good to go :D\n");
	}
#endif
}
