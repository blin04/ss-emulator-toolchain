#include <termios.h>
#include <unistd.h>

#include "../inc/terminal.hpp"
#include "../inc/cpu.hpp"

Terminal::Terminal() 
    : term_out(0) {
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

int Terminal::readIn() { return term_in; }

void Terminal::writeOut(int word) { term_out = word; }

bool Terminal::poll() {

    if (term_out != 0) {
        write(STDOUT_FILENO, &term_out, 1);
        term_out = 0;
    }

    int ret = read(STDIN_FILENO, &term_in, 1);
    return ret != 0;
}