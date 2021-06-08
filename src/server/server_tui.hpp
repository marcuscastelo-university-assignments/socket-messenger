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

		
		//Verificação se está ativo
        bool m_Running = false;

		//Função que atualiza o header do servidor, com o status (online/offline) e quem está online
        void UpdateHeader();

        int headerStartY = 3, headerLenY = 7, headerMarginB = 1, headerMarginX = 2;

        std::string m_OnlineStr;
    public:
		//Construtor da classe ServerTUI
        ServerTUI(Server &server) : m_Server(server) {}

		/**
		 * Função que atualiza os nicknames de todos os clientes onlines nos headers de todos os clientes
		 * 
		 * Parâmetros:	const std::string &onlineStr	=>	String que armazena os nicknames de usuários online
		 * 
		 * Retorno: void
		*/
        void SetOnline(const std::string &onlineStr);

		//Método que retorna se a TUI está em execução
        inline bool IsRunning() { return m_Running; }

		//Método que solicita o encerramento da TUI (geralmente chamado a partir de outra thread)
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