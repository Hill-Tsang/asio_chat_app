#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <fstream>
#include <thread>
#include <conio.h>
#include <sstream>
#include "asio.hpp"
#include "asio/ssl.hpp"

using namespace std;
using asio::ip::tcp;
using placeholders::_1;
using placeholders::_2;

enum { max_length = 1024 };

class client {
    public:
        client(asio::io_context& io_context,
                asio::ssl::context& context,
                const tcp::resolver::results_type& endpoints)
            :   socket_(io_context, context),
                log_file("log.txt")
        {
            socket_.set_verify_mode(asio::ssl::verify_peer);
            socket_.set_verify_callback(bind(&client::verify_certificate, this, _1, _2));

            connect(endpoints);
        }

        ~client() {
            cout << "close client" << endl;
            log_file.close();
        }

        void add_to_input_buffer(int c) {
            if (isprint(c)) {
                input_buffer.insert(cursor_position, 1, c);
                ++cursor_position;
            }
        }

        void send_message() {
            cout << input_buffer << endl;
            if (input_buffer == "exit") {
                exit(EXIT_SUCCESS);
            }
            
            strcpy(request_ , input_buffer.c_str());
            size_t request_length = strlen(request_);

            asio::async_write(socket_, asio::buffer(request_, request_length),
                                [this](const error_code& error, size_t length) {
                                    if (!error) {
                                        input_buffer = "";
                                        cursor_position = 0;
                                    }
                                }
            );
        }

        asio::ssl::stream<tcp::socket> socket_;

    private:
        bool verify_certificate(bool preverified, asio::ssl::verify_context& ctx) {
            // The verify callback can be used to check whether the certificate that is
            // being presented is valid for the peer. For example, RFC 2818 describes
            // the steps involved in doing this for HTTPS. Consult the OpenSSL
            // documentation for more details. Note that the callback is called once
            // for each certificate in the certificate chain, starting from the root
            // certificate authority.

            // In this example we will simply print the certificate's subject name.
            char subject_name[256];
            X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
            X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
            cout << "Verifying " << subject_name << "\n";

            return preverified;
        }

        void connect(const tcp::resolver::results_type& endpoints) {
            asio::async_connect(socket_.lowest_layer(),
                                endpoints,
                                [this](const error_code& error, const tcp::endpoint& /*endpoint*/) {
                                    if (!error) {
                                        handshake();
                                    } else {
                                        cout << "Connect failed: " << error.message() << "\n";
                                    }
                                }
            );
        }

        void handshake() {
            socket_.async_handshake(asio::ssl::stream_base::client,
                                    [this](const error_code& error) {
                                        if (!error) {
                                            receive_message();
                                        }
                                        else {
                                            cout << "Handshake failed: " << error.message() << "\n";
                                        }
                                    }
            );
        }

        void receive_message() {
            socket_.async_read_some(asio::buffer(reply_),
                                    [this](const error_code& ec, size_t length) {
                                        if (!ec) {
                                            string message(reply_, length);
                                            console_output(message);
                                            receive_message();
                                        }
                                    }
            );
        }

        void console_output(string message) {
            printf("\x1b[2K\x1b[0G%s\n%s\x1b[%zuG", message.c_str(), input_buffer.c_str(), cursor_position+1);
            fflush(stdout);
        }

        string input_buffer;
        int cursor_position = 0;

        ofstream log_file;

        // Messages send to and receive from server
        char request_[max_length];
        char reply_[max_length];

        int receiver_index;
};

string get_password() {
    return "test";
}

int main() {
    try {
        asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "5000");

        asio::ssl::context ctx(asio::ssl::context::sslv23);
        ctx.set_password_callback(bind(get_password));
        ctx.load_verify_file("..\\cert\\ca.crt");
        ctx.use_certificate_chain_file("..\\cert\\client.crt");
        ctx.use_private_key_file("..\\cert\\client.key", asio::ssl::context::pem);

        client c(io_context, ctx, endpoints);

        thread t([&](){ io_context.run(); });

        int input_char;
        while (true) {
            while (input_char != '\r') {
                input_char = _getch();
                printf("%c", input_char);
                fflush(stdout);

                c.add_to_input_buffer(input_char);
            }

            c.send_message();
            input_char = 0; // Reset 
        }

        t.join();
        
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}