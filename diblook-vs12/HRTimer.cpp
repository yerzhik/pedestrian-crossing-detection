// HRTimer.cpp: implementation of the HRTimer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HRTimer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HRTimer::HRTimer()
{
	frequency = 1.0 / this->GetFrequency();
}

HRTimer::~HRTimer()
{

}

__int64 HRTimer::GetFrequency(void)
{

    LARGE_INTEGER proc_freq;

    QueryPerformanceFrequency(&proc_freq);

    return proc_freq.QuadPart;

}

void HRTimer::StartTimer(void)
{

    unsigned long oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
    ::QueryPerformanceCounter(&start);
    ::SetThreadAffinityMask(::GetCurrentThread(), oldmask);

}

double HRTimer::StopTimer(void)
{

    unsigned long oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
    ::QueryPerformanceCounter(&stop);
    ::SetThreadAffinityMask(::GetCurrentThread(), oldmask);

    return ((stop.QuadPart - start.QuadPart) * frequency*1000);
	// timpul intors este in ms

} 