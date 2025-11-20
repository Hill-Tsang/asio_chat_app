#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include <cstring>
#include <fstream>
#include <thread>
#include "asio.hpp"
#include "asio/ssl.hpp"
#include "message.hpp"

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

        ofstream log_file;

        // Messages send to and receive from server
        char request_[max_length];
        char reply_[max_length];
        
        string username;
        string receiver;

        thread t;
};

#endif