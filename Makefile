CXX = g++

#OPENSSL_DIR = /Volumes/Backup/openssl-3.5.4-static
OPENSSL_DIR = J:\Development\program_test\openssl-3.5.2

ifeq ($(OS),Windows_NT)
	fixPATH = $(subst /,\,$1)

	INCLUDE = -Iinclude -I$(OPENSSL_DIR)\include
	CXXFLAGS = $(INCLUDE) -std=c++11 -DWIN
	LDFLAG = -L$(OPENSSL_DIR)\lib
	LIBS = -lws2_32 -lwsock32 -lssl -lcrypto

	CLIENT_TARGET = build\chat_client.exe
	SERVER_TARGET = build\chat_server.exe

	RM = del
else
	fixPATH = $1

	NCURSES_DIR = /Volumes/Backup/ncurses-6.5-static

	INCLUDE = -Iinclude -I$(OPENSSL_DIR)/include/ -I$(NCURSES_DIR)/include
	CXXFLAGS = $(INCLUDE) -std=c++11
	LDFLAG = -L$(OPENSSL_DIR)/lib/ -L$(NCURSES_DIR)/lib
	LIBS = -lssl -lcrypto -lncurses

	CLIENT_TARGET = build/chat_client
	SERVER_TARGET = build/chat_server

	RM = rm
endif

CLIENT_SRCS = $(call fixPATH,src/client/client.cpp) $(call fixPATH,src/client/main.cpp) $(call fixPATH,src/client/console.cpp)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)

SERVER_SRCS = $(call fixPATH,src/server/server.cpp) $(call fixPATH,src/server/session.cpp) $(call fixPATH,src/server/main.cpp)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)


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
	$(RM) $(CLIENT_TARGET) $(SERVER_TARGET) $(CLIENT_OBJS) $(SERVER_OBJS)