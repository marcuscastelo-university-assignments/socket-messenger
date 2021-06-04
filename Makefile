all: client

SERVER_OBJS = server.o socket.o
CLIENT_OBJS = client.o socket.o

LD_FLAGS = -pthread

server: $(SERVER_OBJS)
	g++ $(SERVER_OBJS) -o server -O0 -ggdb $(LD_FLAGS)

client: $(CLIENT_OBJS)
	g++ $(CLIENT_OBJS) -o client -O0 -ggdb $(LD_FLAGS)

%.o: %.cpp
	g++ $< -c -ggdb 