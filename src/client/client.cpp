#include "chat_client.hpp"

client::client(asio::io_context& io_context,
        asio::ssl::context& context,
        const tcp::resolver::results_type& endpoints)
    :   socket_(io_context, context),
        log_file("log.txt")
{
    socket_.set_verify_mode(asio::ssl::verify_peer);
    socket_.set_verify_callback(bind(&client::verify_certificate, this, _1, _2));

    connect(endpoints);

    t = thread([&](){ io_context.run(); });

    sleep(1);
    register_user();
}

client::~client() {
    cout << "close client" << endl;
    log_file.close();
    t.join();
}

void client::show_banner() {
    print_to_console("==================================\n");
    print_to_console("        ASIO Chat App CLI\n");
    print_to_console("----------------------------------\n");
    print_to_console("After setting your name, you can\n");
    print_to_console("enter -h to list all commands.\n");
    print_to_console("==================================\n");
}

void client::add_to_input_buffer(int c) {
    if (isprint(c)) {
        input_buffer.insert(cursor_position, 1, c);
        ++cursor_position;
    }
}

void client::del_from_input_buffer() {
    if (!input_buffer.empty()) {
        input_buffer.pop_back(); // Removes the last character
    }
}

void client::process_input() {
    if (input_buffer == "exit") {
        exit(EXIT_SUCCESS);
    } else if (input_buffer == "-l") {
        list_user();
    } else if (input_buffer.length() >= 3 && input_buffer.substr(0, 3) == "-s ") {
        string receiver = input_buffer.substr(3);
        set_receiver(receiver);
    } else {
        send_chat_msg();
    }
}

void client::register_user() {
    string name;
    cout << "Enter your name: " ;
    cin >> name;

    Message message_to_send;
    message_to_send.type = REGISTER;
    strcpy(message_to_send.text, name.c_str());

    username = name;

    /*
    cout << "Size of message struct: " << sizeof(Message) << endl;
    cout << "Size of message struct type: " << sizeof(Message::type) << endl;
    cout << "Size of message struct text: " << sizeof(Message::text) << endl;
    cout << "Size of message: " << sizeof(message_to_send) << endl;
    cout << "Size of type: " << sizeof(message_to_send.type) << endl;
    cout << "Size of text: " << sizeof(message_to_send.text) << endl;
    */
    
    char serialized_message[sizeof(Message)];
    memcpy(serialized_message, &message_to_send, sizeof(Message));

    asio::async_write(socket_, asio::buffer(serialized_message, sizeof(Message)),
                        [this](const error_code& error, size_t length) {
                            if (!error) {
                                input_buffer = "";
                                cursor_position = 0;
                            }
                        }
    );
}

void client::list_user() {
    Message message_to_send;
    message_to_send.type = LIST_USER;

    char serialized_message[sizeof(Message)];
    memcpy(serialized_message, &message_to_send, sizeof(Message));

    asio::async_write(socket_, asio::buffer(serialized_message, sizeof(Message)),
                        [this](const error_code& error, size_t length) {
                            if (!error) {
                                input_buffer = "";
                                cursor_position = 0;
                            }
                        }
    );
}

void client::set_receiver(string receiver_name) {
    Message message_to_send;
    message_to_send.type = SET_RECEIVER;
    strcpy(message_to_send.text, receiver_name.c_str());

    char serialized_message[sizeof(Message)];
    memcpy(serialized_message, &message_to_send, sizeof(Message));

    asio::async_write(socket_, asio::buffer(serialized_message, sizeof(Message)),
                        [this](const error_code& error, size_t length) {
                            if (!error) {
                                input_buffer = "";
                                cursor_position = 0;
                            }
                        }
    );
}

void client::send_chat_msg() {
    Message message_to_send;
    message_to_send.type = CHAT;
    strcpy(message_to_send.text, input_buffer.c_str());
    strcpy(message_to_send.receiver, receiver.c_str());
    strcpy(message_to_send.sender, username.c_str());

    char serialized_message[sizeof(Message)];
    memcpy(serialized_message, &message_to_send, sizeof(Message));

    asio::async_write(socket_, asio::buffer(serialized_message, sizeof(Message)),
                        [this](const error_code& error, size_t length) {
                            if (!error) {
                                input_buffer = "";
                                cursor_position = 0;
                            }
                        }
    );
}


bool client::verify_certificate(bool preverified, asio::ssl::verify_context& ctx) {
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
    //cout << "Verifying " << subject_name << "\n";

    return preverified;
}

void client::connect(const tcp::resolver::results_type& endpoints) {
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

void client::handshake() {
    socket_.async_handshake(asio::ssl::stream_base::client,
                            [this](const error_code& error) {
                                if (!error) {
                                    receive_message();
                                } else {
                                    cout << "[Error] Handshake failed: " << error.message() << "\n";
                                }
                            }
    );
}

void client::receive_message() {
    socket_.async_read_some(asio::buffer(reply_),
                            [this](const error_code& ec, size_t length) {
                                if (!ec) {
                                    Message received_message;
                                    memcpy(&received_message, reply_, sizeof(Message));
                                    string message_text(received_message.text, sizeof(received_message.text));
                                    string sender(received_message.sender, sizeof(received_message.sender));

                                    if (received_message.type == SET_RECEIVER) {
                                        console_output("Sever", message_text);
                                        receiver = received_message.receiver;
                                    } else {
                                        console_output(sender, message_text);
                                    }
                                    receive_message();
                                }
                            }
    );
}

void client::console_output(string sender, string message) {
    printf("\x1b[2K\x1b[0G%s: %s\n\x1b[0Gcli> %s\x1b[%dG", sender.c_str(), message.c_str(), input_buffer.c_str(), cursor_position + 6);
    fflush(stdout);
}
