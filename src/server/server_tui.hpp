#pragma once

#include "server.hpp"

namespace tui
{

    class ServerTUI
    {
        Server &m_Server;

    public:
        ServerTUI(Server &server) : m_Server(server) {}

        void Enter();
    };

}