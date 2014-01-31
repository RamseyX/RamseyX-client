
// RamseyXDlg.h : 头文件
//

#pragma once

#include "RamseyXController.h"
//#include "CPULimiter.h"

//UINT ThreadProc(LPVOID lpParam);
UINT FetchProc(LPVOID lpParam);
UINT RefreshRankProc(LPVOID lpParam);
UINT AutoUploadProc(LPVOID lpParam);
UINT GetNewTaskProc(LPVOID lpParam);
UINT UpdateBenchmarkProc(LPVOID lpParam);
VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);

// CRamseyXDlg 对话框
class CRamseyXDlg : public CDialogEx
{
// 构造
public:
	CRamseyXDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_RAMSEYX_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	
public:
	CString GetStatus();

public:
	void StoreLevelSpeed() const;
	void StoreAccount() const;

public:
	RamseyXController m_controller;

	//CPULimiter m_limiter;
	HANDLE m_aTimerHandles[10];
	HANDLE m_hTimerQueue;
	unsigned long long m_uID;
	int m_nCoreNum;
	int m_nThreadNum;
	int m_nCPULimit;
	bool m_bIsAuto;
	bool m_bIsUpdating;
	bool m_bWasRunning;
	bool m_bIsLocked;
	bool m_bIsExiting;
	std::atomic<bool> m_bIsAutoUploading = {false};
	double m_fVAX_MIPS;
	CString m_strUsername;
	CString m_strPassword;
	CString m_strDir;

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedStart();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnNI(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnThreadOpen(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateStatus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEnableCtrl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClearMyList(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewTask(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAutoUpload(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWriteLog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClrOutdated(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShow();
	afx_msg void OnExit();
	afx_msg void OnBnClickedUpload();
	afx_msg void OnBnClickedAuto();
	afx_msg void OnNMCustomdrawLevel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureLevel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedFetchNow();
	afx_msg void OnNMCustomdrawThread(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureThread(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRefreshRank();
	afx_msg void OnNMClickSyslink(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedGetNewTask();
	BOOL PreTranslateMessage(MSG* pMsg);
	

	UINT_PTR SetTimer(UINT_PTR nIDEvent, UINT nElapse,
		void (CALLBACK* lpfnTimer)(HWND, UINT, UINT_PTR, DWORD));
	BOOL KillTimer(UINT_PTR nIDEvent);
};
