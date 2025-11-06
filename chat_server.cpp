#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <sstream>
#include <map>
#include <list>
#include "asio.hpp"
#include "asio/ssl.hpp"

#include <ctime>

using asio::ip::tcp;
using namespace std;

class session : public enable_shared_from_this<session> {
    public:
        session(asio::ssl::stream<tcp::socket> socket) : socket_(move(socket)) {
        }

        void start() {
            cout << "Start do handshake" << endl;
            do_handshake();
        }

        asio::ssl::stream<tcp::socket>& get_socket() {
            return socket_;
        }

        asio::ssl::stream<tcp::socket> socket_;

        void do_handshake() {
            auto self(shared_from_this());
            socket_.async_handshake(asio::ssl::stream_base::server,
                                    [this, self](const error_code& error) {
                                        if (!error) {
                                            cout << "handshake complete, do_read" << endl;
                                            do_read();
                                        }
                                    }
            );
        }

        void do_read() {
            auto self(shared_from_this());
            socket_.async_read_some(asio::buffer(data_),
                                    [this, self](const error_code& ec, size_t length) {
                                        if (!ec) {
                                            string received_data;
                                            received_data.assign(data_, length);

                                            cout << "Receive data from client " << index << ": ";
                                            cout << received_data << endl;

                                            if (received_data == "get index") {
                                                cout << "send index info back to client" << endl;
                                                char reply_msg[] = "Your session index is ";
                                                char index_string[3];
                                                sprintf(index_string, "%d", index);
                                                strcat(reply_msg, index_string);
                                                do_write(reply_msg, strlen(reply_msg), index);

                                            } else if (received_data.find("set receiver") != string::npos) {
                                                cout << "set receiver found" << endl;
                                                vector<string> words;
                                                stringstream ss(received_data);
                                                string word;
                                                
                                                while (ss >> word) {
                                                    words.push_back(word);
                                                }

                                                receiver_index = stoi(string(words[2]));
                                                cout << "receiver index set to " << receiver_index << endl;

                                            } else {
                                                do_write(data_, length, receiver_index);
                                            }
                                            do_read();
                                            
                                        }
                                    }
            );
        }

        void do_write(char* data, size_t length, int receiver) {
            auto self(shared_from_this());

            /*
            time_t now;
            struct tm* local_time;
            char time_buffer[80];

            now = time(0);
            local_time = localtime(&now);
            strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", local_time);
            */

            asio::async_write((*session_map)[receiver]->socket_, asio::buffer(data, length),
                                [this, self](const error_code& ec, size_t length) {
                                    if (!ec) {

                                    } else {
                                        cout << "send failed" << endl;
                                    }
                                }
            );
        }

        map<int, shared_ptr<session>> *session_map;
        char data_[1024];
        int index;
        int receiver_index;
};

class server {
    public:
        server(asio::io_context& io_context, unsigned short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
            context_(asio::ssl::context::sslv23)
        {
            context_.set_options(
                asio::ssl::context::default_workarounds
                | asio::ssl::context::no_sslv2
                | asio::ssl::context::single_dh_use);
            context_.set_password_callback(bind(&server::get_password, this));
            
            context_.load_verify_file("..\\cert\\ca.crt");
            context_.use_certificate_chain_file("..\\cert\\server.crt");
            context_.use_private_key_file("..\\cert\\server.key", asio::ssl::context::pem);
            
            context_.use_tmp_dh_file("..\\cert\\dh4096.pem");

            do_accept();
        }

    private:
        string get_password() const {
            return "test";
        }

        void do_accept()  {
            acceptor_.async_accept(
                [this](const error_code& error, tcp::socket socket) {
                    if (!error) {
                        cout << "Create session" << endl;
                        cout << "Add session pointer to list index " << index << endl;
                        session_map[index] = make_shared<session>(asio::ssl::stream<tcp::socket>(move(socket), context_));
                        session_map[index]->session_map = &session_map;
                        session_map[index]->index = index;
                        session_map[index]->start();
                        
                        index++;
                        
                    }
                    do_accept();
                }
            );
        }

        int index = 0;
        map<int, shared_ptr<session>> session_map;
        tcp::acceptor acceptor_;
        asio::ssl::context context_;
};


int main() {
    try {
        asio::io_context io_context;

        cout << "Start server" << endl;
        server s(io_context, 5000);

        io_context.run();
        
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
