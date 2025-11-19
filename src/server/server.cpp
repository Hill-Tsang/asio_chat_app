#include "chat_server.hpp"

server::server(asio::io_context& io_context, unsigned short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
            context_(asio::ssl::context::sslv23) {
    context_.set_options(
        asio::ssl::context::default_workarounds
        | asio::ssl::context::no_sslv2
        | asio::ssl::context::single_dh_use);
    context_.set_password_callback(bind(&server::get_password, this));
    
    context_.load_verify_file("../cert/ca.crt");
    context_.use_certificate_chain_file("../cert/server.crt");
    context_.use_private_key_file("../cert/server.key", asio::ssl::context::pem);
    
    context_.use_tmp_dh_file("../cert/dh4096.pem");

    do_accept();
}

string server::get_password() {
    return "test";
}

void server::do_accept() {
    acceptor_.async_accept(
        [this](const error_code& error, tcp::socket socket) {
            if (!error) {
                cout << "Create session" << endl;
                cout << "Add session pointer to list index " << user_num << endl;
                session_map[user_num] = make_shared<session>(asio::ssl::stream<tcp::socket>(move(socket), context_));
                session_map[user_num]->index = user_num;
                session_map[user_num]->start();
                
                user_num++;
                
            }
            do_accept();
        }
    );
}