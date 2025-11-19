#include "chat_client.hpp"
#include <ncurses.h>

string get_password() {
    return "test";
}

int main() {
    try {
        

        asio::io_context io_context;
        tcp::resolver resolver(io_context);
        //auto endpoints = resolver.resolve("10.8.0.6", "5000");
        auto endpoints = resolver.resolve("192.168.0.11", "5000");

        asio::ssl::context ctx(asio::ssl::context::sslv23);
        ctx.set_password_callback(bind(get_password));
        ctx.load_verify_file("../cert/ca.crt");
        ctx.use_certificate_chain_file("../cert/client.crt");
        ctx.use_private_key_file("../cert/client.key", asio::ssl::context::pem);

        cout << "init client" << endl;
        client c(io_context, ctx, endpoints);

        initscr();
        cbreak();
        noecho();
        c.show_banner();

        int input_char;
        int input_time;

        while (true) {
            printw("cli> ");
            while (input_char != 10) {
                input_char = getch();

                if ((input_char == 127) && (input_time > 0)) {
                    printf("\b \b");
                    fflush(stdout);
                    input_time--;
                    c.del_from_input_buffer();
                    continue;
                }

                if (isprint(input_char)) {
                    printf("%c", input_char);
                    fflush(stdout);
                    //printw("%c", input_char);
                    c.add_to_input_buffer(input_char);
                    input_time++;
                }
            }
            printf("\x1b[0G");
            fflush(stdout);
            c.process_input();
            input_char = 0; // Reset 
            input_time = 0;
        }

    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    endwin();
    return 0;
}