#include <cstdlib>
#include <string>

using namespace std;

struct Message {
    int type;
    char sender[16];
    char receiver[16];
    char text[1024];
};

enum message_type {
    REGISTER,
    SET_RECEIVER,
    LIST_USER,
    CHAT,
};