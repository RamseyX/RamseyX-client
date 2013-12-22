#pragma once


// CSplash

class CSplash : public CWnd
{
	DECLARE_DYNAMIC(CSplash)

public:
	CSplash();
	virtual ~CSplash();

protected:
	DECLARE_MESSAGE_MAP()

private:
	CBitmap m_bmp;

public:
	void Create(UINT nBitmapID);

	afx_msg void OnPaint();
	afx_msg LRESULT OnInit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBenchmark(WPARAM wParam, LPARAM lParam);
};


