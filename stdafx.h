
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展





#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持


#include <afxsock.h>            // MFC 套接字扩展







#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "curl/curl.h"
#pragma comment(lib, "libcurl_imp.lib")

#include "atlrx.h"

#include "MyCurlWrapper.h"

#define WM_SEL			(WM_USER + 0x0001)
#define WM_NI			(WM_USER + 0x0002)
#define WM_THREADOPEN	(WM_USER + 0x0003)
#define WM_UPDATESTATUS	(WM_USER + 0x0004)
#define WM_ENABLECTRL	(WM_USER + 0x0005)
#define WM_CLEARALLLIST	(WM_USER + 0x0006)
#define WM_CLEARMYLIST	(WM_USER + 0x0007)
#define WM_NEWTASK		(WM_USER + 0x0008)
#define WM_AUTOUPLOAD	(WM_USER + 0x0009)
#define WM_WRITELOG		(WM_USER + 0x000a)
#define WM_INIT			(WM_USER + 0x000b)
#define WM_BENCHMARK	(WM_USER + 0x000c)
#define WM_CLROUTDATED	(WM_USER + 0x000d)
#define WM_NEWBATCH		(WM_USER + 0x000e)

#define PRIMARY_VERSION		4
#define SECONDARY_VERSION	5
#define TERTIARY_VERSION	0

#define ERR_SUCCESS					0
#define ERR_INVALID_ARGUMENTS		1
#define ERR_CONNECTION_FAILED		2
#define ERR_LOGIN_FAILED			3
#define ERR_WRONG_USR_PWD			4
#define ERR_GET_FAILED				5
#define ERR_NO_NEW_TASK				6
#define ERR_UPLOAD_FAILED			7
#define ERR_USER_INFO_UNAVAILABLE	8
#define ERR_USER_ALREADY_EXIST		9
#define ERR_EMAIL_ALREADY_EXIST		10
#define ERR_INVALID_RECOMMENDER		11
#define ERR_SIGNUP_FAILED			12
#define ERR_TASK_OUTDATED			13

char* UTF8ToUnicode(const char* mbcsStr);
char* UnicodeToUTF8(const char* mbcsStr);

extern CCriticalSection csWriteLog;
extern CCriticalSection csRunning;
extern CCriticalSection csTodo;
extern CCriticalSection csCompleted;
extern CCriticalSection csBatch;

#define SVR_API(s) "localhost/"##s