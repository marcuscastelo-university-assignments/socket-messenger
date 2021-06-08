#include "user_sockets.hpp"

#include <unordered_map>
#include <string>
#include "socket.hpp"

void UserSockets::RegisterUser(const std::string &nickname, const Socket &socket)
{
    m_NickToSocket.insert({nickname, socket});
    m_SocketToNick.insert({socket, nickname});
}

void UserSockets::UnregisterUser(const Socket &userSocket)
{
    auto it = m_SocketToNick.find(userSocket);
    if (it == m_SocketToNick.end())
        throw std::logic_error("Cannot unregister user (sock) since it is not registered");
    m_SocketToNick.erase(it);
}
void UserSockets::UnregisterUser(const std::string &nickname)
{
    auto it = m_NickToSocket.find(nickname);
    if (it == m_NickToSocket.end())
        throw std::logic_error("Cannot unregister user (nick) since it is not registered");
    m_NickToSocket.erase(it);
}

bool UserSockets::IsUserRegistered(const Socket &userSocket)
{
    return FindBySocket(userSocket) != m_SocketToNick.end();
}
bool UserSockets::IsUserRegistered(const std::string &nickname)
{
    return FindByNick(nickname) != m_NickToSocket.end();
}

const std::string &UserSockets::GetUserNick(const Socket &userSocket) const
{
    auto it = m_SocketToNick.find(userSocket);
    if (it == m_SocketToNick.end())
        throw std::logic_error("Cannot get user (sock) since it is not registered");
    return it->second;
}
const Socket &UserSockets::GetUserSocket(const std::string &nickname) const
{
    auto it = m_NickToSocket.find(nickname);
    if (it == m_NickToSocket.end())
        throw std::logic_error("Cannot get user (nick) since it is not registered");
    return it->second;
}