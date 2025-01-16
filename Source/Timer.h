#pragma once

class Timer
{
public:
    Timer();
    void Reset();
    double GetTime() const;
    float GetMilliseconds() const;

private:
    double start;
};
