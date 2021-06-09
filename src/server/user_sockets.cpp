#include "user_sockets.hpp"

#include <unordered_map>
#include <string>
#include <algorithm>
#include "socket.hpp"

using guard = std::lock_guard<typename std::mutex>;

void UserSockets::RegisterUser(const User &user)
{
    guard gd_(m_Mutex);
    m_Users.push_back(user);
    m_NickToUser.insert({user.m_Nick, user});
    m_SocketToUser.insert({user.m_Socket->GetFD(), user});
}

void UserSockets::UnregisterUser(const Socket &userSocket)
{
    guard gd_(m_Mutex);
    User *userPtr = FindBySocket(userSocket);
    if (userPtr == nullptr) {
        throw std::logic_error("Trying to unregister inexistent user (By Socket)");
    }

    m_SocketToUser.erase(userPtr->m_Socket->GetFD());
    m_NickToUser.erase(userPtr->m_Nick);
    auto vecit = std::find(m_Users.begin(), m_Users.end(), *userPtr);
    m_Users.erase(vecit);
}
void UserSockets::UnregisterUser(const std::string &nickname)
{
    guard gd_(m_Mutex);
    User *userPtr = FindByNick(nickname);
    if (userPtr == nullptr) {
        throw std::logic_error("Trying to unregister inexistent user (By Nick)");
    }

    m_SocketToUser.erase(userPtr->m_Socket->GetFD());
    m_NickToUser.erase(userPtr->m_Nick);
    auto vecit = std::find(m_Users.begin(), m_Users.end(), *userPtr);
    m_Users.erase(vecit);
}

bool UserSockets::IsUserRegistered(const Socket &userSocket)
{
    return FindBySocket(userSocket) != nullptr;
}
bool UserSockets::IsUserRegistered(const std::string &nickname)
{
    return FindByNick(nickname) != nullptr;
}

User *UserSockets::FindByNick(const std::string &nick)
{
    auto it = m_NickToUser.find(nick);
    if (it == m_NickToUser.end())
        return nullptr;
    return &it->second;
}
User *UserSockets::FindBySocket(const Socket &socket)
{
    auto it = m_SocketToUser.find(socket.GetFD());
    if (it == m_SocketToUser.end())
        return nullptr;
    return &it->second;
}