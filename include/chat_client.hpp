#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <fstream>
#include <thread>
#include <sstream>
//#include <ncurses.h>
#include "asio.hpp"
#include "asio/ssl.hpp"
#include "message.hpp"
#include "console_output.hpp"

using namespace std;
using asio::ip::tcp;
using placeholders::_1;
using placeholders::_2;

enum { max_length = sizeof(Message) };

class client {
    public:
        client(asio::io_context& io_context,
                asio::ssl::context& context,
                const tcp::resolver::results_type& endpoints);

        ~client();

        void show_banner();

        void add_to_input_buffer(int c);

        void del_from_input_buffer();

        void process_input();

        void register_user();

        void list_user();

        void set_receiver(string receiver_name);

        void send_chat_msg();

        asio::ssl::stream<tcp::socket> socket_;

    private:
        bool verify_certificate(bool preverified, asio::ssl::verify_context& ctx);

        void connect(const tcp::resolver::results_type& endpoints);

        void handshake();

        void receive_message();

        void console_output(string sender, string message);

        string input_buffer;
        int cursor_position = 0;

        ofstream log_file;

        // Messages send to and receive from server
        char request_[max_length];
        char reply_[max_length];
        
        string username;
        string receiver;

        thread t;
};