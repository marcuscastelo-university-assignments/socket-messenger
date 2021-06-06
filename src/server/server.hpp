#pragma once
#include "socket.hpp"

#include <vector>
#include <thread>
#include <unordered_map>

//Estrutura auxiliar que armazena informações essenciais para o server
struct ServerInfo
{
    Socket socket;
    IPADDR4 address;

    int maxClients;
    bool running = false;

    std::vector<std::thread *> threadsVector = {};

    std::vector<Message> pendingMessages = {};

    struct
    {
        std::unordered_map<std::string, Socket> clientSockets = {};
        std::unordered_map<Socket, std::string> socketClients = {};

        std::unordered_map<std::string, Socket>::iterator find(const std::string &nick) { return clientSockets.find(nick); }
        std::unordered_map<Socket, std::string>::iterator find(const Socket &socket) { return socketClients.find(socket); }

    } biMapClientSocket;

    ServerInfo(Socket serverSocket, IPADDR4 address, int maxClients) : socket(serverSocket), address(address), maxClients(maxClients) {}
};