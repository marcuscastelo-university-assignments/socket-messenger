/**
 * Header auxiliar responsável pela Text User Interface
 */

#pragma once

#include "server.hpp"

namespace tui
{

	//Classe ServerTUI, que é responsável por formatar e personalizar as mensagens do servidor
    class ServerTUI
    {
        Server &m_Server;

        bool m_Running = false;

    public:
        ServerTUI(Server &server) : m_Server(server) {}

        inline void RequestStop() { m_Running = false; }

		//Função principal que faz todo tratamento e edição das mensagens do servidor
        void Enter();
    };

}