#include "Timer.h"


static double frequency = 0.0f;

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

Timer::Timer()
{
	if (frequency == 0.0f)
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	}

    QueryPerformanceCounter((LARGE_INTEGER*)&start);
}

void Timer::Reset()
{
    QueryPerformanceCounter((LARGE_INTEGER*)&start);
}

double Timer::GetTime() const
{
    double counter;
    QueryPerformanceCounter((LARGE_INTEGER*)&counter);
    counter = (counter - start) / frequency;
    return counter;
}

float Timer::GetMilliseconds() const
{
    double counter;
    QueryPerformanceCounter((LARGE_INTEGER*)&counter);
	float ms = (float)(1000.0f * (counter - start) / frequency);
    return ms;
}
