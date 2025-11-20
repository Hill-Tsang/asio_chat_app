#include <iostream>
#include "chat_client.hpp"
#include "console.hpp"

string get_password() {
    return "test";
}

int main() {
    try {
        asio::io_context io_context;
        tcp::resolver resolver(io_context);
        //auto endpoints = resolver.resolve("10.8.0.10", "5000");
        auto endpoints = resolver.resolve("192.168.0.11", "5000");

        asio::ssl::context ctx(asio::ssl::context::sslv23);
        ctx.set_password_callback(bind(get_password));
        ctx.load_verify_file("../cert/ca.crt");
        ctx.use_certificate_chain_file("../cert/client.crt");
        ctx.use_private_key_file("../cert/client.key", asio::ssl::context::pem);

        cout << "init client" << endl;

        client c(io_context, ctx, endpoints);

        console cli(&c);

    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}