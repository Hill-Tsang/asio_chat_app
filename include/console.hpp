#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include "chat_client.hpp"

using namespace std;

#define ENTER 10
#define BACKSAPCE 127

class console {
    public:
        console(client* client_ptr);

        ~console();

        static void print_received_msg(string sender, string message);
        static string input_buffer;

    private:
        void show_banner();

        void start_cli();

        void add_to_input_buffer(int c);

        void del_from_input_buffer();

        static int input_buffer_size;

        client* client_ptr;
};

#endif