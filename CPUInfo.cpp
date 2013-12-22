#include "StdAfx.h"
#include "CPUInfo.h"


CCPUInfo::CCPUInfo(void)
	: m_dwEAX(0), m_dwEBX(0), m_dwECX(0), m_dwEDX(0)
{
}


CCPUInfo::~CCPUInfo(void)
{
}

void CCPUInfo::ExecuteCPUID(DWORD dwVEAX)
{
	DWORD dwEAX, dwEBX, dwECX, dwEDX;

	__asm
	{
		MOV EAX, dwVEAX
		CPUID
		MOV dwEAX, EAX
		MOV dwEBX, EBX
		MOV dwECX, ECX
		MOV dwEDX, EDX
	}

	m_dwEAX = dwEAX;
	m_dwEBX = dwEBX;
	m_dwECX = dwECX;
	m_dwEDX = dwEDX;
}

CString CCPUInfo::GetVID()
{
	char szVID[S * 3 + 1] = {0};
	ExecuteCPUID(0);
	memcpy(szVID + S * 0, &m_dwEBX, S); // 复制前四个字符到数组
	memcpy(szVID + S * 1, &m_dwEDX, S); // 复制中间四个字符到数组
	memcpy(szVID + S * 2, &m_dwECX, S); // 复制最后四个字符到数组

	return CString(szVID).Trim(L" ");
}

CString CCPUInfo::GetBrand()
{
	char szBrand[S * 4 * 3 + 1] = {0};

	for (DWORD i = 0; i < 3; i++)
	{
		ExecuteCPUID(BRANDID + i);
		memcpy(szBrand + i * S * 4 + S * 0, &m_dwEAX, S);
		memcpy(szBrand + i * S * 4 + S * 1, &m_dwEBX, S);
		memcpy(szBrand + i * S * 4 + S * 2, &m_dwECX, S);
		memcpy(szBrand + i * S * 4 + S * 3, &m_dwEDX, S);
	}

	return CString(szBrand).Trim(L" ");
}

LONGLONG CCPUInfo::GetFrequency(DWORD nCPULimitTime)
{
	HANDLE hP = ::GetCurrentProcess();
	HANDLE hT = ::GetCurrentThread();
	DWORD dwPC = ::GetPriorityClass(hP);
	DWORD dwTP = ::GetThreadPriority(hT);

	BOOL bFlag1 = FALSE, bFlag2 = FALSE;
	DWORD dwLow1 = 0, dwHigh1 = 0, dwLow2 = 0, dwHigh2 = 0;

	bFlag1 = ::SetPriorityClass(hP, REALTIME_PRIORITY_CLASS);
	bFlag2 = ::SetThreadPriority(hT, THREAD_PRIORITY_HIGHEST);

	::Sleep(10);
	LARGE_INTEGER fq, st, ed;
	::QueryPerformanceFrequency(&fq);
	::QueryPerformanceCounter(&st);

	__asm        //获得当前cpu的时间周期数
	{
		RDTSC
		MOV dwLow1, EAX
		MOV dwHigh1, EDX
	}
	::Sleep(nCPULimitTime);     //将线程挂起片刻
	::QueryPerformanceCounter(&ed);  //获得结束时间
	__asm        //获得当前cpu的时间周期数
	{
		RDTSC
		MOV dwLow2, EAX
		MOV dwHigh2, EDX
	}
 
	if(bFlag1)
		::SetPriorityClass(hP, dwPC);  //将当前进程优先度恢复原样
	if(bFlag2)
		::SetThreadPriority(hT, dwTP);  //将当前线程优先度恢复原样
	::CloseHandle(hP);
	::CloseHandle(hT);
	//将cpu的时间周期数转化成64位整数
	LONGLONG beg = (LONGLONG) dwHigh1 << 32 | dwLow1;
	LONGLONG end = (LONGLONG) dwHigh2 << 32 | dwLow2;
	//将两次获得的cpu时间周期数除以间隔时间，即得到cpu的频率。
	//由于windows的Sleep函数有大约15毫秒的误差，故以windows的精确计时为准
	return (end - beg) * fq.QuadPart / (ed.QuadPart - st.QuadPart);
}

