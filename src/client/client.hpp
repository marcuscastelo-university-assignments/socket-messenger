#pragma once

#include <vector>
#include <thread>
#include "socket.hpp"

//Para uso do ponteiro opaco ClientTUI* (dependência cíclica)
#include "types.fwd.hpp"

class Client
{
    Socket m_Socket;
    std::string m_Nickname = "guest";

    bool m_Exiting = false;

    mutable std::vector<Message> m_ReceivedMessages = {};

    std::thread *m_ServerSlaveThread = nullptr;
    tui::ClientTUI *m_CurrentTUI = nullptr;

    inline void CloseSockets() { m_Socket.Close(); }
    void ServerSlaveLoop();

public:
    Client() : m_Socket(SocketType::TCP) {}

    inline bool IsExiting() const { return m_Exiting; }
    inline const std::string& GetNickname() const { return m_Nickname; }
    inline std::vector<Message> GetReceivedMessages() const { return m_ReceivedMessages; }

    void ConnectAndLogin(const IPADDR4 &serverAddr, const std::string &nickname, size_t maxTrials = 3);
    void SendMessage(const Message &message, size_t maxTries = 3);

    void Start();

    void RequestExit();

    //TODO: check if properly stopped
    virtual ~Client();
};

void listenServerPackets(Client &client);