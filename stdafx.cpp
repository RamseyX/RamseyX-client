
// stdafx.cpp : 只包括标准包含文件的源文件
// RamseyX.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

CCriticalSection csWriteLog;
CCriticalSection csRunning;
CCriticalSection csTodo;
CCriticalSection csCompleted;
CCriticalSection csBatch;

char* UTF8ToUnicode(const char* mbcsStr)
{
	wchar_t wideStr[60000] = {0};
	static char unicodeStr[60000] = {0};
	int charLen = 0;

	charLen = ::MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, NULL, 0);   
	::MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, wideStr, charLen); 

	charLen = ::WideCharToMultiByte(CP_ACP, 0, wideStr, -1, NULL, 0, NULL, NULL);
    ::WideCharToMultiByte(CP_ACP, 0, wideStr, -1, unicodeStr, charLen, NULL, NULL); 

	return unicodeStr;
}

char* UnicodeToUTF8(const char* mbcsStr)
{
	wchar_t wideStr[60000] = {0};
	static char utf8Str[60000] = {0};
	int charLen;

	charLen = ::MultiByteToWideChar(CP_UTF8, 0, mbcsStr, -1, NULL, 0);
	::MultiByteToWideChar(CP_ACP, 0, mbcsStr, -1, wideStr, charLen);
	
	charLen = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
	
	::WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, utf8Str, charLen, NULL, NULL);

	return utf8Str;
}


