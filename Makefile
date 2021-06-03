all: server

SERVER_OBJS = server.o

LD_FLAGS = -pthread

server: $(SERVER_OBJS)
	g++ $(SERVER_OBJS) -o server -O0 -ggdb $(LD_FLAGS)

.c.o:
	g++ $< -c 