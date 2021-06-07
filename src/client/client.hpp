#pragma once

#include <vector>
#include "socket.hpp"

struct Client {
    Socket m_Socket;
    std::string m_Nickname;
    
    std::vector<Message> m_ReceivedMessages;

    std::vector<Message> m_MessagesToBeSent;

    Client(const Socket& serverSocket, const std::string& nickname): m_Socket(serverSocket), m_Nickname(nickname) {}
};