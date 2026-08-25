/*
*   Class emulating a terminal peripheral.
*/

#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <thread>

class Terminal {
private:
    std::thread terminal;
};

#endif