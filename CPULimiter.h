#pragma once

#include "stdafx.h"

class CPULimiter
{
	LARGE_INTEGER m_lastTotalSystemTime;
    LARGE_INTEGER m_lastProcessUsageTime;

    int m_nRatio;

public:

	CPULimiter() : m_nRatio(100)
	{
	}

	CPULimiter(int nRatio) : m_nRatio(nRatio)
	{
	}

	inline void SetRatio(int nRatio)
	{
		m_nRatio = nRatio;
	}

	inline BOOL CalculateAndSleep()
	{
		//Declare variables;
		FILETIME sysidle, kerusage, userusage, processkern, processuser, processcreat, processexit;
		LARGE_INTEGER tmpvar, thissystime, thisprocesstime;
		
		//Get system kernel, user and idle times
		if (!::GetSystemTimes(&sysidle, &kerusage, &userusage))
			return FALSE;
		
		//Get process user and kernel times
		if (!::GetProcessTimes(::GetCurrentProcess(), &processcreat, &processexit, &processkern, &processuser))
			return FALSE;
		
		//Calculates total system times
		//This is sum of time used by system in kernel, user and idle mode.
		
		tmpvar.LowPart = sysidle.dwLowDateTime;
		tmpvar.HighPart = sysidle.dwHighDateTime;
		thissystime.QuadPart = tmpvar.QuadPart;
		
		tmpvar.LowPart = kerusage.dwLowDateTime;
		tmpvar.HighPart = kerusage.dwHighDateTime;
		thissystime.QuadPart = thissystime.QuadPart + tmpvar.QuadPart;
		
		tmpvar.LowPart = userusage.dwLowDateTime;
		tmpvar.HighPart = userusage.dwHighDateTime;
		thissystime.QuadPart = thissystime.QuadPart + tmpvar.QuadPart;
		
		//Calculates time spent by this process in user and kernel mode.
		
		tmpvar.LowPart = processkern.dwLowDateTime;
		tmpvar.HighPart = processkern.dwHighDateTime;
		thisprocesstime.QuadPart = tmpvar.QuadPart;
		
		tmpvar.LowPart = processuser.dwLowDateTime;
		tmpvar.HighPart = processuser.dwHighDateTime;
		thisprocesstime.QuadPart = thisprocesstime.QuadPart + tmpvar.QuadPart;
		
		//Check if this is first time this function is called
		//if yes, escape rest after copying current system and process time
		//for further use
		//Also check if the ratio of differences between current and previous times
		//exceeds the specified ratio.
		
		if (thisprocesstime.QuadPart != 0 && (((thisprocesstime.QuadPart - m_lastProcessUsageTime.QuadPart) * 100) - ((thissystime.QuadPart - m_lastTotalSystemTime.QuadPart) * m_nRatio)) > 0)
		{
			//Calculate the time interval to sleep for averaging the extra CPU usage
			//by this process.
			
			LARGE_INTEGER timetosleepin100ns;
			timetosleepin100ns.QuadPart = (((thisprocesstime.QuadPart - m_lastProcessUsageTime.QuadPart) * 100) / m_nRatio)  - (thissystime.QuadPart - m_lastTotalSystemTime.QuadPart);
			
			//check if time is less than a millisecond, if yes, keep it for next time.
			if ((timetosleepin100ns.QuadPart / 10000) <= 0)
				return FALSE;
			
			//Time to sleep
			::Sleep(timetosleepin100ns.QuadPart / 10000);
		}
		
		//Copy usage time values for next time calculations.
		m_lastTotalSystemTime.QuadPart = thissystime.QuadPart;
		m_lastProcessUsageTime.QuadPart = thisprocesstime.QuadPart;
		
		return TRUE;
	}
};

