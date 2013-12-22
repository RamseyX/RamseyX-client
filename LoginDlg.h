#pragma once

UINT LgThreadProc(LPVOID lpParam);
UINT ValidateProc(LPVOID lpParam);

// CLoginDlg 对话框

class CLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLoginDlg();

// 对话框数据
	enum { IDD = IDD_LOGIN };

public:
	CWinThread* m_pLoginThread;
	bool m_bIsFirst;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSignup();
	afx_msg void OnBnClickedLogin();
	afx_msg LRESULT OnSel(WPARAM wParam, LPARAM lParam);
	BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg void OnPaint();
	//afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedLock();
	afx_msg LRESULT OnEnableCtrl(WPARAM wParam, LPARAM lParam);
};
