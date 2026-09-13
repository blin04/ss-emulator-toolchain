#include "../inc/timer.hpp"

Timer::Timer() {
    configureTimer(0);
    lastPoll = std::chrono::steady_clock::now();
}

void Timer::configureTimer(int value) {
    switch (value) {
        case 0:
            period = std::chrono::milliseconds(500);
            break;
        case 1:
            period = std::chrono::milliseconds(1000);
            break;
        case 2:
            period = std::chrono::milliseconds(1500);
            break;
        case 3:
            period = std::chrono::milliseconds(2000);
            break;
        case 4:
            period = std::chrono::milliseconds(5000);
            break;
        case 5:
            period = std::chrono::milliseconds(10000);
            break;
        case 6:
            period = std::chrono::milliseconds(30000);
            break;
        case 7:
            period = std::chrono::milliseconds(60000);
            break;
    }
}

bool Timer::poll() {
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    if (now - lastPoll < period)
        return false;
    lastPoll = now;
    return true;
}