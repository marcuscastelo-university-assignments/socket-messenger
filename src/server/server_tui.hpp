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
        ServerInfo &m_Server;

    public:
        ServerTUI(ServerInfo &server) : m_Server(server) {}

		//Função principal que faz todo tratamento e edição das mensagens do servidor
        void Enter();
    };

}