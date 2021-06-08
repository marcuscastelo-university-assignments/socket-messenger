/**
 * Header criado para facilitar a organização das funções responsáveis
 * por printar na interface do usuário
*/
#pragma once

#include "client.hpp"
#include "tui.hpp"
#include <termios.h>
#include <functional>

//Using operators ms, ns, etc..
#include <chrono>
using namespace std::chrono_literals;

using namespace tui::text;
using namespace tui::text_literals;

namespace tui
{
	//Classe ClientTUI, responsável por todo tratamento de print out para o client
	class ClientTUI
	{
		//Armazena o cliente em questão
		Client &m_Client;

		//Verificação se está ativo
		bool m_Running = false;

		//Função que atualiza o header do cliente, sempre mostrando seu nickname, quem está online
		// e também uma frase do dia
		void UpdateScreen();


		int headerStartY = 3, headerLenY = 9, headerMarginB = 1, headerMarginX = 2;

		std::string m_OnlineStr;

	public:
		//Construtor da classe ClientTUI
		ClientTUI(Client &client) : m_Client(client) {}

		//Função que retorna um bool; Verifica se o cliente está online
		inline bool IsRunning() { return m_Running; }

		/**
		 * Função que atualiza os nicknames de todos os clientes onlines nos headers de todos os clientes
		 * 
		 * Parâmetros:	const std::string &onlineStr	=>	String que armazena os nicknames de usuários online
		 * 
		 * Retorno: void
		*/
		void SetOnline(const std::string &onlineStr);

		/**
		 * Função que recepciona o cliente e faz a chamada das outras funções essenciais para a interface do cliente
		 * 
		 * Parâmetros: void
		 * 
		 * Retorno: void
		*/
		void Enter();

		/**
		 * Função que setta o usuário como desconectado
		 * 
		 * Parâmetros:	void
		 * 
		 * Retorno:	void
		*/
		void RequestExit();

		/**
		 * Função que notifica (printa na tela do usuário) as informações recebidas do servidor
		 * 
		 * Parâmetros:	const std::string &serverNotification	=>	String do servidor com as informações
		 * 
		 * Retorno:	void
		*/
		void Notify(const std::string &serverNotification);

		/**
		 * Função que notifica (printa na tela do usuário) as mensagens recebidas
		 * 
		 * Parâmetros:	const std::string &fromUser	=>	usuário que mandou
		 * 
		 * Retorno:	void
		*/
		void PrintMessages(const std::string& fromUser);
	};
}