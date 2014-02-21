#ifndef RX_UTILS_H
#define RX_UTILS_H

#include <cstring>
//#include <locale>
#include <memory>
#ifdef _WIN32
#include <wtypes.h>
#include <intrin.h>
#include <stringapiset.h>
#else
    #error Unsupported OS
#endif

class RamseyXUtils
{
public:
    static std::string to_string(const std::wstring &ws)
	{
        //std::locale oldLocale(std::locale::global(std::locale("zh_CN.utf8")));
        //std::setlocale(LC_ALL, "CHS");

		const wchar_t *_Source = ws.c_str();
		std::size_t _DSize = sizeof (wchar_t) * (ws.size() + 1);

        std::unique_ptr<char[]> _Dest(new char[_DSize]{});
		std::wcstombs(_Dest.get(), _Source, _DSize);

        //std::locale::global(oldLocale);
        //std::setlocale(LC_ALL, "");

		return std::string(_Dest.get());
	}

    static std::wstring to_wstring(const std::string &s)
    {
        //std::locale oldLocale(std::locale::global(std::locale("zh_CN.utf8")));
        //std::setlocale(LC_ALL, "CHS");

		const char *_Source = s.c_str();
		std::size_t _DSize = sizeof (char) * (s.size() + 1);

        std::unique_ptr<wchar_t[]> _Dest(new wchar_t[_DSize]{});
		std::mbstowcs(_Dest.get(), _Source, _DSize);

        //std::locale::global(oldLocale);
        // std::setlocale(LC_ALL, "");

		return std::wstring(_Dest.get());
	}

    static std::wstring getCPUBrandString()
	{
#ifdef _WIN32
		int CPUInfo[4] = {-1};
		__cpuid(CPUInfo, 0x80000000);
        if (static_cast<unsigned int>(CPUInfo[0]) < 0x80000004)
			return std::wstring();
		char CPUBrandString[0x40] = {};
		__cpuid(CPUInfo, 0x80000002);
		std::memcpy(CPUBrandString, CPUInfo, sizeof (CPUInfo));
		__cpuid(CPUInfo, 0x80000003);
		std::memcpy(CPUBrandString + 16, CPUInfo, sizeof (CPUInfo));
		__cpuid(CPUInfo, 0x80000004);
		std::memcpy(CPUBrandString + 32, CPUInfo, sizeof (CPUInfo));
		int i = 0;
		for (; CPUBrandString[i] == ' '; ++i);
		return RamseyXUtils::to_wstring(CPUBrandString + i);
#else
    #error Unsupported OS
#endif
	}

    static std::unique_ptr<char[]> UTF8ToUnicode(const char *mbcsStr)
	{
#ifdef _WIN32
		int charLen = ::MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, nullptr, 0);
        std::unique_ptr<wchar_t[]> wideStr(new wchar_t[charLen + 1]);

		::MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, wideStr.get(), charLen);

		charLen = ::WideCharToMultiByte(CP_ACP, 0, wideStr.get(), -1, nullptr, 0, nullptr, nullptr);
        std::unique_ptr<char[]> unicodeStr(new char[charLen + 1]);

		::WideCharToMultiByte(CP_ACP, 0, wideStr.get(), -1, unicodeStr.get(), charLen, nullptr, nullptr);

		return unicodeStr;
#else
    #error Unsupported OS
#endif
	}

    static std::unique_ptr<char[]> UnicodeToUTF8(const char *mbcsStr)
	{
#ifdef _WIN32
		int charLen = ::MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, nullptr, 0);
        std::unique_ptr<wchar_t[]> wideStr(new wchar_t[charLen + 1]);

		::MultiByteToWideChar(CP_ACP, 0, mbcsStr, -1, wideStr.get(), charLen);

		charLen = WideCharToMultiByte(CP_UTF8, 0, wideStr.get(), -1, nullptr, 0, nullptr, nullptr);
        std::unique_ptr<char[]> utf8Str(new char[charLen + 1]);

		::WideCharToMultiByte(CP_UTF8, 0, wideStr.get(), -1, utf8Str.get(), charLen, nullptr, nullptr);

		return utf8Str;
#else
    #error Unsupported OS
#endif
	}
};

#endif
