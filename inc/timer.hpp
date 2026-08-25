/*
*   Class emulating a timer peripheral.
*/

#ifndef _TIMER_H_
#define _TIMER_H_

#include <thread>

class Timer {
private:
    std::thread timer;
};

#endif