CXX = g++

OPENSSL_DIR = /Volumes/Backup/openssl-3.5.4-static
NCURSES_DIR = /Volumes/Backup/ncurses-6.5-static

INCLUDE = -Iinclude -I$(OPENSSL_DIR)/include/ -I$(NCURSES_DIR)/include
LDFLAG = -L$(OPENSSL_DIR)/lib/ -L$(NCURSES_DIR)/lib
LIBS = -lssl -lcrypto -lncurses 

CXXFLAGS = $(INCLUDE) -std=c++11

CLIENT_SRCS = src/client/client.cpp src/client/main.cpp src/client/console.cpp
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)

SERVER_SRCS = src/server/server.cpp src/server/session.cpp src/server/main.cpp
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)

CLIENT_TARGET = build/chat_client
SERVER_TARGET = build/chat_server


all: $(CLIENT_TARGET) $(SERVER_TARGET)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CXX) $(LDFLAG) $^ -o $@ $(LIBS)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CXX) $(LDFLAG) $^ -o $@ $(LIBS)

# Rule for compiling .cpp files into .o files
#%.o: %.cpp
#	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target: removes generated files
clean:
	rm $(CLIENT_TARGET) $(SERVER_TARGET) $(CLIENT_OBJS) $(SERVER_OBJS)