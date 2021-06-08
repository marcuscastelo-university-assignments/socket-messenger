#pragma once

#include <unordered_map>
#include <string>
#include "socket.hpp"

/**
	 * Estrutura criada para facilitar gerenciamento dos clientes
	 * conectados em tempo de execução. Nela são contidas dois mapas
	 * não ordenados que armazenam os clientes e seus sockets respectivos,
	 * facilitando a organização e o fluxo de dados
	 * */

class UserSockets
{
    std::unordered_map<std::string, Socket> m_NickToSocket = {};
    std::unordered_map<Socket, std::string> m_SocketToNick = {};

    inline std::unordered_map<std::string, Socket>::iterator FindByNick(const std::string &nick) { return m_NickToSocket.find(nick); }
    inline std::unordered_map<Socket, std::string>::iterator FindBySocket(const Socket &socket) { return m_SocketToNick.find(socket); }

public:
    void RegisterUser(const std::string &nickname, const Socket &socket);

    void UnregisterUser(const Socket &userSocket);
    void UnregisterUser(const std::string &nickname);

    bool IsUserRegistered(const Socket &userSocket);
    bool IsUserRegistered(const std::string &nickname);

    const std::string &GetUserNick(const Socket &userSocket);
    const Socket &GetUserSocket(const std::string &nickname);
};