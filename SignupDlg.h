#pragma once

UINT SmThreadProc(LPVOID lpParam);

// CSignupDlg 对话框

class CSignupDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSignupDlg)

public:
	CSignupDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSignupDlg();

// 对话框数据
	enum { IDD = IDD_SIGNUP };

public:
	CWinThread* m_pSubmitThread;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSubmit();
	afx_msg LRESULT OnSel(WPARAM wParam, LPARAM lParam);
	BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//afx_msg void OnPaint();
	afx_msg LRESULT OnEnableCtrl(WPARAM wParam, LPARAM lParam);
};
