#include <ncurses.h>
#include "console_output.hpp"

using namespace std;

void print_to_console(const char* text) {
    printw(text);
}