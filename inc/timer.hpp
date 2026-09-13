#ifndef _TIMER_H_
#define _TIMER_H_

#include <thread>

class Timer {
public:
    Timer();

    void configureTimer(int tim_cfg);
    bool poll();
private:
    std::chrono::milliseconds period;     // ms
    std::chrono::time_point<std::chrono::steady_clock> lastPoll;
};

#endif