# asio_chat_app
A simple C++ program that demonstrates using ASIO with SSL to set up a chat server and client.

This is a CLI program so it will handle the stdout so that it can keep the input buffer while printing the recieved message from another client.

## How to use
Run the compiled chat server and then chat client. It aims for client to client chat so you need to run 2 clients. All server and clients need to be ran on the same host so far.

Once a client is connected to the server, server will assign a index for this client (Starting from 0). Client can enter **get index** to get index information from the server.

To enable send message to another client. You need to enter **set receiver [index]** where [index] is the index of the receiver. Then, we can start sending message to the receiver.
## Compile
Please follow https://github.com/Hill-Tsang/cplusplus_sample/tree/main/asio_ssl to compile openssl and prepare the required certs in cert folder.

ASIO library downloaded from https://think-async.com/Asio/Download.html. ASIO version: 3.5.2
### Server
```
g++ -I include .\chat_server.cpp -I openssl-3.5.2\build\include\ -L openssl-3.5.2\build\lib\ -o build\chat_server.exe -l ws2_32 -l wsock32 -lssl -lcrypto
```

### Client
```
g++ -I include .\chat_client.cpp -I openssl-3.5.2\build\include\ -L openssl-3.5.2\build\lib\ -o build\chat_client.exe -l ws2_32 -l wsock32 -lssl -lcrypto
```