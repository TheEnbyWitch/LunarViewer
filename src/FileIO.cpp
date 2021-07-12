#include <string>
#include <codecvt>
#include "Common.h"

#include "FileIO.h"
#if defined(_WIN32)
#include <Windows.h>
#endif

// LUNA: i dont know if this is deprecated and for now i dont care
#if defined(_WIN32)
std::wstring s2ws(const std::string& s)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(s);
}

std::string ws2s(const std::wstring& s)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(s);
}
#endif

std::string FileIO::GetFileOpen(std::string InitialDirectory, const wchar_t* Filter, void* WindowHandle)
{
#if defined(_WIN32)
	OPENFILENAME ofn;
	wchar_t fileName[MAX_PATH] = L"";
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)WindowHandle;
	ofn.lpstrFilter = Filter;
	ofn.lpstrFile = fileName;
	ofn.lpstrInitialDir = s2ws(InitialDirectory).c_str();
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = L"";

	std::wstring fileNameStr = L"";

	if (GetOpenFileName(&ofn))
		fileNameStr = fileName;

	return ws2s(fileNameStr);
#else
	Com_Error(ERR_FATAL, "FileIO::GetFileOpen not implemented!");
	return "";
#endif
}
