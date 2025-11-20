#include "console.hpp"

#ifdef WIN
#include <iostream>
#include <conio.h>
#else
#include <ncurses.h>
#endif

#ifdef WIN
#define ENTER 13
#define BACKSPACE 8
#else
#define ENTER 10
#define BACKSPACE 127
#endif

string console::input_buffer = "";
int console::input_buffer_size = 0;

console::console(client* client) : client_ptr(client) {
#ifndef WIN
    initscr();
    cbreak();
    noecho();
#endif

    show_banner();

    start_cli();
}

console::~console() {
#ifndef WIN
    endwin();
#endif
}

void console::show_banner() {
#ifdef WIN
    cout << ("==================================") << endl;
    cout << ("        ASIO Chat App CLI\n");
    cout << ("----------------------------------") << endl;
    cout << ("After setting your name, you can") << endl;
    cout << ("enter -h to list all commands.") << endl;
    cout << ("==================================") << endl;
#else
    printw("==================================\n");
    printw("        ASIO Chat App CLI\n");
    printw("----------------------------------\n");
    printw("After setting your name, you can\n");
    printw("enter -h to list all commands.\n");
    printw("==================================\n");
#endif
}

void console::start_cli() {
    int input_char;

    while (true) {
#ifdef WIN
        printf("cli> ");
        fflush(stdout);
#else
        printw("cli> ");
#endif

        while (input_char != ENTER) {
#ifdef WIN
            input_char = _getch();
#else
            input_char = getch();
#endif
            
            // Press backsapce, remove last character
            if ((input_char == BACKSPACE) && (input_buffer_size > 0)) {
                printf("\b \b");
                fflush(stdout);
                del_from_input_buffer();
                continue;
            }
            
            // Press printable key, display new input character
            if (isprint(input_char)) {
                printf("%c", input_char);
                fflush(stdout);
                add_to_input_buffer(input_char);
            }
        }

        // Press Enter
        printf("\n\x1b[0G");
        fflush(stdout);
        client_ptr->process_input();

        input_char = 0;
        input_buffer = "";
        input_buffer_size = 0;
    }
}

void console::add_to_input_buffer(int c) {
    if (isprint(c)) {
        input_buffer.insert(input_buffer_size, 1, c);
        ++input_buffer_size;
    }
}

void console::del_from_input_buffer() {
    if (!input_buffer.empty()) {
        input_buffer.pop_back(); // Removes the last character
        --input_buffer_size;
    }
}

void console::print_received_msg(string sender, string message) {
    printf("\x1b[2K\x1b[0G%s: %s\n\x1b[0Gcli> %s\x1b[%dG", sender.c_str(), message.c_str(), input_buffer.c_str(), input_buffer_size + 6);
    fflush(stdout);
}