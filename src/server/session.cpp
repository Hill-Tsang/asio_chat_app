#include "chat_server.hpp"

session::session(asio::ssl::stream<tcp::socket> socket) : socket_(move(socket)) {
}

void session::start() {
    do_handshake();
}

asio::ssl::stream<tcp::socket>& session::get_socket() {
    return socket_;
}

void session::do_handshake() {
    auto self(shared_from_this());
    socket_.async_handshake(asio::ssl::stream_base::server,
                            [this, self](const error_code& error) {
                                if (!error) {
                                    cout << "Handshake complete, start listening to client" << endl;
                                    do_read();
                                } else {
                                    cout << "[Error] Handshake failed" << endl;
                                }
                            }
    );
}

void session::do_read() {
    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(data_),
                            [this, self](const error_code& ec, size_t length) {
                                if (!ec) {
                                    Message received_message;
                                    memcpy(&received_message, data_, sizeof(Message));

                                    cout << "Received message from Client. INDEX: " << index << "    ";
                                    
                                    if (received_message.type == REGISTER) {
                                        cout << "ACTION: Register user    VALUE: " << received_message.text << endl;
                                        register_user(received_message.text);
                                    } else if (received_message.type == SET_RECEIVER) {
                                        cout << "ACTION: Set receiver    VALUES: " << received_message.text << endl;
                                        set_receiver(received_message.text);
                                    } else if (received_message.type == LIST_USER) {
                                        cout << "ACTION: List user" << endl;
                                        list_user();
                                    } else if (received_message.type == CHAT) {
                                        cout << "ACTION: Send message to " << received_message.receiver << endl;
                                        forward_msg(received_message);
                                    }

                                    do_read();
                                }
                            }
    );
}

void session::register_user(string name) {
    user_index[name] = index;
    username = name;
}

void session::list_user() {
    string users;
    for (auto& element : user_index) {
        users += (element.first + "\n");
    }

    Message message_to_send;
    message_to_send.type = LIST_USER;
    strcpy(message_to_send.text, users.c_str());

    string sender = "Server";
    strcpy(message_to_send.sender, sender.c_str());

    char serialized_message[sizeof(Message)];
    memcpy(serialized_message, &message_to_send, sizeof(Message));

    asio::async_write(session_map[index]->socket_, asio::buffer(serialized_message, sizeof(Message)),
                        [this](const error_code& error, size_t length) {
                            if (error) {
                                cout << "[Error] Send user list to client failed" << endl;
                            }
                        }
    );
}

void session::set_receiver(string receiver) {
    if (user_index.find(receiver) != user_index.end()) {
        cout << "receiver: " << receiver << " found" << endl;
    }

    Message message_to_send;
    string text = "SUCCESS";
    message_to_send.type = SET_RECEIVER;
    strcpy(message_to_send.text, text.c_str());
    strcpy(message_to_send.receiver, receiver.c_str());

    char serialized_message[sizeof(Message)];
    memcpy(serialized_message, &message_to_send, sizeof(Message));

    asio::async_write(session_map[index]->socket_, asio::buffer(serialized_message, sizeof(Message)),
                        [this](const error_code& error, size_t length) {
                            if (error) {
                                cout << "[Error] Send set receiver message to client failed" << endl;
                            }
                        }
    );
}

void session::forward_msg(Message message) {
    char serialized_message[sizeof(Message)];
    memcpy(serialized_message, &message, sizeof(Message));

    asio::async_write(session_map[user_index[message.receiver]]->socket_, asio::buffer(serialized_message, sizeof(Message)),
                        [this](const error_code& error, size_t length) {
                            if (error) {
                                cout << "[Error] Forward message failed" << endl;
                            }
                        }
    );
}