#include "chat_server.hpp"

map<string, int> user_index;
int user_num = 0;
map<int, shared_ptr<session>> session_map;

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
