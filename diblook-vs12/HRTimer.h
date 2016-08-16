// HRTimer.h: interface for the HRTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HRTIMER_H__71233007_9BD7_4E74_9440_E3D6C8334E11__INCLUDED_)
#define AFX_HRTIMER_H__71233007_9BD7_4E74_9440_E3D6C8334E11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class HRTimer  
{
public:
	HRTimer();
	virtual ~HRTimer();

	__int64 GetFrequency(void);
    void StartTimer(void) ;
    double StopTimer(void);

private:

    LARGE_INTEGER start;
    LARGE_INTEGER stop;

    double frequency;
    //..

};

#endif // !defined(AFX_HRTIMER_H__71233007_9BD7_4E74_9440_E3D6C8334E11__INCLUDED_)
