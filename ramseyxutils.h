/***************************************************************************
 *
 * RamseyX Client: client program of distributed computing project RamseyX
 *
 * Copyright (C) 2013-2014 Zizheng Tai <zizheng.tai@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/
#ifndef RAMSEYXUTILS_H
#define RAMSEYXUTILS_H

#include <cstring>
#include <memory>
#ifdef RX_QT
#include <QVariant>
#include <limits>
#endif
#if defined(_WIN32)
#include <wtypes.h>
#include <intrin.h>
#include <stringapiset.h>
#elif defined(__unix__) || defined(__linux__)
#include <fstream>
#include <string>
#else
    #error Unsupported OS
#endif

class RamseyXUtils
{
public:
    static std::string to_string(const std::wstring &ws)
    {
        //std::locale oldLocale(std::locale::global(std::locale("zh_CN.utf8")));

        const wchar_t *_Source = ws.c_str();
        std::size_t _DSize = sizeof (wchar_t) * (ws.size() + 1);

        std::unique_ptr<char[]> _Dest(new char[_DSize]{});
        std::wcstombs(_Dest.get(), _Source, _DSize);

        //std::locale::global(oldLocale);

        return std::string(_Dest.get());
    }

    static std::wstring to_wstring(const std::string &s)
    {
        //std::locale oldLocale(std::locale::global(std::locale("zh_CN.utf8")));

        const char *_Source = s.c_str();
        std::size_t _DSize = sizeof (char) * (s.size() + 1);

        std::unique_ptr<wchar_t[]> _Dest(new wchar_t[_DSize]{});
        std::mbstowcs(_Dest.get(), _Source, _DSize);

        //std::locale::global(oldLocale);

        return std::wstring(_Dest.get());
    }

    static std::string getCpuBrandString()
    {
#if defined(_WIN32)
        int cpuInfo[4] = {-1};
        __cpuid(cpuInfo, 0x80000000);
        if (static_cast<unsigned int>(cpuInfo[0]) < 0x80000004)
            return std::string();
        char cpuBrandString[0x40] = {};
        __cpuid(cpuInfo, 0x80000002);
        std::memcpy(cpuBrandString, cpuInfo, sizeof (cpuInfo));
        __cpuid(cpuInfo, 0x80000003);
        std::memcpy(cpuBrandString + 16, cpuInfo, sizeof (cpuInfo));
        __cpuid(cpuInfo, 0x80000004);
        std::memcpy(cpuBrandString + 32, cpuInfo, sizeof (cpuInfo));
        int i = 0;
        for (; cpuBrandString[i] == ' '; ++i);
        return cpuBrandString + i;
#elif defined(__unix__) || defined(__linux__)
        std::ifstream fin("/proc/cpuinfo");
        if (!fin)
            return std::string();
        std::string line;
        while (std::getline(fin, line))
            if (line.find("model name") == 0)
                return line.substr(line.find(':') + 2);
        return std::string();
#else
    #error Unsupported OS
#endif
    }

    static std::unique_ptr<char[]> Utf8ToUnicode(const char *mbcsStr)
    {
#if defined(_WIN32)
        int charLen = MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, nullptr, 0);
        std::unique_ptr<wchar_t[]> wideStr(new wchar_t[charLen + 1]);

        MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, wideStr.get(), charLen);

        charLen = WideCharToMultiByte(CP_ACP, 0, wideStr.get(), -1, nullptr, 0, nullptr, nullptr);
        std::unique_ptr<char[]> unicodeStr(new char[charLen + 1]);

        WideCharToMultiByte(CP_ACP, 0, wideStr.get(), -1, unicodeStr.get(), charLen, nullptr, nullptr);

        return unicodeStr;
#else
        std::unique_ptr<char[]> unicodeStr(new char[std::strlen(mbcsStr) + 1]);
        std::strcpy(unicodeStr.get(), mbcsStr);
        return unicodeStr;
#endif
    }

    static std::unique_ptr<char[]> UnicodeToUtf8(const char *mbcsStr)
    {
#if defined(_WIN32)
        int charLen = MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, nullptr, 0);
        std::unique_ptr<wchar_t[]> wideStr(new wchar_t[charLen + 1]);

        MultiByteToWideChar(CP_ACP, 0, mbcsStr, -1, wideStr.get(), charLen);

        charLen = WideCharToMultiByte(CP_UTF8, 0, wideStr.get(), -1, nullptr, 0, nullptr, nullptr);
        std::unique_ptr<char[]> utf8Str(new char[charLen + 1]);

        WideCharToMultiByte(CP_UTF8, 0, wideStr.get(), -1, utf8Str.get(), charLen, nullptr, nullptr);

        return utf8Str;
#else
        std::unique_ptr<char[]> utf8Str(new char[std::strlen(mbcsStr) + 1]);
        std::strcpy(utf8Str.get(), mbcsStr);
        return utf8Str;
#endif
    }

#ifdef RX_QT
    static QVariant toMinIntegerType(unsigned long long n)
    {
        if (static_cast<unsigned long long>(std::numeric_limits<unsigned int>::max()) >= n)
            return QVariant::fromValue(static_cast<unsigned int>(n));
        else if (static_cast<unsigned long long>(std::numeric_limits<unsigned long>::max()) >= n)
            return QVariant::fromValue(static_cast<unsigned long>(n));
        else
            return QVariant::fromValue(n);
    }
#endif
};

#endif // RAMSEYXUTILS_H
