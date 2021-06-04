all: client

SERVER_OBJS = server.o
CLIENT_OBJS = client.o

LD_FLAGS = -pthread

server: $(SERVER_OBJS)
	g++ $(SERVER_OBJS) -o server -O0 -ggdb $(LD_FLAGS)

client: $(CLIENT_OBJS)
	g++ $(CLIENT_OBJS) -o client -O0 -ggdb $(LD_FLAGS)

.c.o:
	g++ $< -c 