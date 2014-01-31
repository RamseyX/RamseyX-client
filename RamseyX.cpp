﻿
// RamseyX.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "RamseyX.h"
#include "RamseyXDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRamseyXApp

BEGIN_MESSAGE_MAP(CRamseyXApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CRamseyXApp 构造

CRamseyXApp::CRamseyXApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CRamseyXApp 对象

CRamseyXApp theApp;


// CRamseyXApp 初始化

BOOL CRamseyXApp::InitInstance()
{
	/*命令行参数：
	-auto 开机启动(静默)
	*/
	

	// 防止双开
	HANDLE hObject = ::CreateMutex(nullptr, FALSE, _T("CRamseyXApp"));
	if(::GetLastError() == ERROR_ALREADY_EXISTS)
	{
		::CloseHandle(hObject);
		::MessageBox(nullptr, _T("程序已经运行。"), _T("RamseyX 运算客户端"), MB_OK | MB_ICONINFORMATION);

		return FALSE;
	}

	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("RamseyX"));

	CRamseyXDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

