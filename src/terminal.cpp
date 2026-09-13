#include <termios.h>
#include <unistd.h>

#include "../inc/terminal.hpp"
#include "../inc/cpu.hpp"

Terminal::Terminal() {
    tcgetattr(STDIN_FILENO, &oldConfig);

    termios newConfig = oldConfig;
    newConfig.c_lflag &= ~(ECHO | ICANON);
    newConfig.c_cc[VTIME] = 0;
    newConfig.c_cc[VMIN] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &newConfig);
}

Terminal::~Terminal() {
    // restore terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldConfig);
}

bool Terminal::poll() {
    int ret = read(STDIN_FILENO, &term_in, 1);
    return ret != 0;
}