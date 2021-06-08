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
		//Construtor da classe ServerTUI
        ServerTUI(Server &server) : m_Server(server) {}

		//Método que retorna se o servidor está online
        inline void RequestStop() { m_Running = false; }

		/**
		 * Função responsável por printar na tela do servidor informações atualizadas dos usuários (quantos conectados e seus nicknames)
		 * 
		 * Parâmetros:	const std::string &notification	=>	string com as informações
		 * 
		 * Retorno: void
		*/
        void Notify(const std::string &notification);

		/**
		 * Função principal que faz todo tratamento e edição das mensagens do servidor
		 * 
		 * Parâmetros:	void
		 * 
		 * Retorno:	void
		*/
        void Enter();
    };

}