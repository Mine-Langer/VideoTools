#pragma once
#include <windows.h>
#include <dshow.h>
#include <vector>
#include <string>
#include <stdexcept>

class ListAVDevice
{
public:
	ListAVDevice();
	~ListAVDevice();

	HRESULT DS_GetAudioVideoInputDevices(std::vector<std::string>& vectorDevices, std::string deviceType);

	std::string DS_GetDefaultDevice(std::string type);

private:
	std::string GbkToUtf8(const char* src_str);
	std::string Utf8ToGbk(const char* src_str);
	std::string unicode2utf8(const WCHAR* uni);
};

