// Splash.cpp : 实现文件
//

#include "stdafx.h"
#include "RamseyX.h"
#include "Splash.h"


// CSplash

IMPLEMENT_DYNAMIC(CSplash, CWnd)

CSplash::CSplash()
{

}

CSplash::~CSplash()
{
}


BEGIN_MESSAGE_MAP(CSplash, CWnd)
	ON_WM_PAINT()
	ON_MESSAGE(WM_INIT, CSplash::OnInit)
	ON_MESSAGE(WM_BENCHMARK, CSplash::OnBenchmark)
END_MESSAGE_MAP()



// CSplash 消息处理程序




void CSplash::Create(UINT nBitmapID)
{
	m_bmp.LoadBitmap(nBitmapID);
	BITMAP bmp;
	m_bmp.GetBitmap(&bmp);
	int x = (::GetSystemMetrics(SM_CXSCREEN) - bmp.bmWidth) / 2;
	int y = (::GetSystemMetrics(SM_CYSCREEN) - bmp.bmHeight) / 2;
	CRect rect(x, y, x + bmp.bmWidth, y + bmp.bmHeight);
	CreateEx(0, AfxRegisterWndClass(0), _T(""), WS_POPUP | WS_VISIBLE, rect, NULL, 0);
}


void CSplash::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	BITMAP bmp;
	m_bmp.GetBitmap(&bmp);

	CDC dcComp;
	dcComp.CreateCompatibleDC(&dc);
	dcComp.SelectObject(&m_bmp);

	dc.BitBlt(0, 0, bmp.bmWidth, bmp.bmHeight, &dcComp, 0, 0, SRCCOPY);
}

LRESULT CSplash::OnInit(WPARAM wParam, LPARAM lParam)
{
	CClientDC dc(this);

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(255, 255, 255));
	CString strInit(_T("Initializing..."));
	if (wParam)
		strInit += _T("OK");
	dc.TextOut(520, 220, strInit, strInit.GetLength());

	return 0;
}

LRESULT CSplash::OnBenchmark(WPARAM wParam, LPARAM lParam)
{
	CClientDC dc(this);

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(255, 255, 255));
	CString strBenchmark(_T("Benchmarking..."));
	if (wParam)
		strBenchmark += _T("OK");
	dc.TextOut(520, 240, strBenchmark, strBenchmark.GetLength());

	return 0;
}
