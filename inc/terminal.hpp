#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <thread>
#include <termios.h>

class CPU;

class Terminal {
public:
    Terminal();
    ~Terminal();

    bool poll();
private:
    termios oldConfig;

    int term_in;
    int term_out;
};

#endif