#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <sstream>
#include <map>
#include <list>
#include "asio.hpp"
#include "asio/ssl.hpp"
#include "message.hpp"

#include <ctime>

using asio::ip::tcp;
using namespace std;

class session : public enable_shared_from_this<session> {
    public:
        session(asio::ssl::stream<tcp::socket> socket);

        void start();

        asio::ssl::stream<tcp::socket>& get_socket();

        asio::ssl::stream<tcp::socket> socket_;

        void do_handshake();

        void do_read();

        void register_user(string name);

        void list_user();

        void set_receiver(string receiver);

        void forward_msg(Message message);

        char data_[sizeof(Message)];
        int index;
        string username;
};

class server {
    public:
        server(asio::io_context& io_context, unsigned short port);

    private:
        string get_password();

        void do_accept();

        tcp::acceptor acceptor_;
        asio::ssl::context context_;

};

extern map<string, int> user_index;
extern int user_num;
extern map<int, shared_ptr<session>> session_map;