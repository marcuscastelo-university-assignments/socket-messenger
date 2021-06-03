#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <thread>
#include <iostream>
#include <vector>

#include "socket.hpp"

struct ServerInfo
{
    Socket socket;
    IPADDR4 address;

    int maxClients;
    bool running = false;

    std::vector<std::thread *> threadsVector = {};

    std::vector<Message> pendingMessages = {};

    ServerInfo(Socket serverSocket, IPADDR4 address, int maxClients) : socket(serverSocket), address(address), maxClients(maxClients) {}
};

void resendmessage(ServerInfo *server_p)
{
    ServerInfo& server = *server_p;

    std::vector<Message> &messages = server.pendingMessages;
    while (server.running)
    {
        for (size_t i = 0; i < messages.size(); i++)
        {
            const Socket &clientSocket = messages[i].Dest;
            server.socket.Send(clientSocket, messages[i].Data);
        }
    }
}

void listenMessages(ServerInfo *server_p, Socket clientSocket)
{
    ServerInfo &server = *server_p;
    Socket &serverSocket = server.socket;

    while (server.running)
    {
        MessageData data(serverSocket.Read(clientSocket));

        //TODO: change clientSocket for real destination
        Message message(data, clientSocket);
        server.pendingMessages.push_back(message);
    }
}

ServerInfo createServer(IPADDR4 address = {{0, 0, 0, 0}, 4545})
{
    Socket serverSocket(SocketType::TCP);

    ServerInfo server(serverSocket, address, 10); //maxClients = 10

    return server;
}

void joinServerThreads(const ServerInfo &server)
{
    for (size_t i = 0; i < server.threadsVector.size(); i++)
    {
        server.threadsVector[i]->join();
    }
}

void acceptClients(ServerInfo &server)
{
    while (server.running)
    {
        Socket clientSocket = server.socket.Accept();

        std::thread *serverListenThread = new std::thread(listenMessages, &server, clientSocket);
        server.threadsVector.push_back(serverListenThread);
    }
}

void startServer(ServerInfo &server, bool waitThreads = true)
{
    server.socket.Bind(server.address);

    std::thread *serverResendMessagesThread = new std::thread(resendmessage, &server);
    server.threadsVector.push_back(serverResendMessagesThread);

    server.socket.Listen(server.maxClients);
    acceptClients(server);

    if (waitThreads)
        joinServerThreads(server);
}

void endServer(ServerInfo &server)
{

    for (size_t i = 0; i < server.threadsVector.size(); i++)
        delete (server.threadsVector[i]);

    shutdown(server.socket.getFD(), 2);
    close(server.socket.getFD());

    server.threadsVector.clear();
}

int main(int argc, char const *argv[])
{

    std::cout << "Criando servidor..." << std::endl;
    ServerInfo server = createServer({{0, 0, 0, 0}, 4545});

    std::cout << "Iniciando servidor..." << std::endl;
    startServer(server, true);

    std::cout << "Fechando o servidor..." << std::endl;
    endServer(server);

    return 0;
}
