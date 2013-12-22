#pragma once

const int S = sizeof (DWORD);
const DWORD BRANDID = 0x80000002;
const LONGLONG GHZ_UNIT = 1000 * 1000 * 1000;

class CCPUInfo
{
public:
	CCPUInfo(void);
	~CCPUInfo(void);

	CString GetVID();	// 获取CPU制造商
	CString GetBrand();	// 获取CPU型号
	LONGLONG GetFrequency(DWORD nCPULimitTime = 1000);	// 获取CPU主频

private:
	void ExecuteCPUID(DWORD dwVEAX);	// 用以执行CCPUInfo指令
	DWORD m_dwEAX;
	DWORD m_dwEBX;
	DWORD m_dwECX;
	DWORD m_dwEDX;
};

