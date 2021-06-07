#pragma once

#include "server.hpp"

namespace tui
{

    class ServerTUI
    {
        Server &m_Server;

        bool m_Running = false;

    public:
        ServerTUI(Server &server) : m_Server(server) {}

        inline void RequestStop() { m_Running = false; }

        void Enter();
    };

}