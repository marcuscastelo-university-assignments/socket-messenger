#pragma once

#include "server.hpp"

namespace tui
{

    class ServerTUI
    {
        ServerInfo &m_Server;

    public:
        ServerTUI(ServerInfo &server) : m_Server(server) {}

        void Enter();
    };

}