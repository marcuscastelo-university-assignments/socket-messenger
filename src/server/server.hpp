/**
 * Header responsável por gerenciar as tarefas do servidor
 */
#pragma once
#include "socket.hpp"
#include "user_sockets.hpp"

#include <vector>
#include <thread>
#include <unordered_map>
#include <iostream>

namespace tui
{
    class ServerTUI;
}
//Classe auxiliar que armazena informações essenciais para o server
class Server
{
    /**
	 * O servidor armazena o seu IP e portas, Socket e
	 * a quantidade de clientes conectados a ele
	 * 
	 * Todas as threads ficam armazenadas em um vector,
	 * bem como as mensagens a serem enviadas
	 **/

    Socket m_Socket;
    IPADDR4 m_Address;

    int m_MaxClients;
    bool m_Running = false;

    tui::ServerTUI *m_CurrentTUI = nullptr;

    std::vector<std::thread *> m_Threads = {};

    //FIXME: Maybe redundant with UserSockets
    std::vector<Socket> m_ConnectedSockets = {};

    std::thread *m_AcceptThread = nullptr;

    std::vector<Message> m_MessagesToSend = {};

    UserSockets m_UserSockets;

    /**
    * Função auxiliar que da a confirmação da conexão de um cliente
    * ao servidor e o adiciona no mapeamento
    * 
    * Parâmetros:  const Socket &clientSocket   =>	Socket do novo cliente
    * 
    * Return: bool	=>	Caso a adição seja validada - true
    * 					Se não, false
    */
    bool LoginUser(const Socket &clientSocket);

    /**
    * Função que permite que o servidor aceite a entrada de clientes
    * e armazena sua thread no vector
    * 
    * Parâmetros: ServerInfor *server_p	=>	Servidor em questão
    * 
    * Return: void
    */
    void AcceptLoop();

    /**
    * Função responsável por receber e armazenar informações mandadas pelo cliente 
    *
    * Parâmetros: ServerInfo &server => armazena informações recebidas
    *             Socket clientSocket  => possui as informações a serem armazenadas
    *
    * Retorno: void
 */
    void ClientLoop(Socket clientSocket);

    /**
    * Função responsável por reenviar a mensagem recebida do cliente para o cliente alvo
    *
    * Parâmetros: Server &server => possui as mensagens a serem reenviadas
    *
    * Retorno: void
    */

    void ForwardMessageLoop();

    void CloseAllSockets();

public:
    Server(IPADDR4 address, int maxClients);

    void Start();

    void EnterTUI();

    inline bool IsRunning() { return m_Running; }

    /**
    * Função que encerra o server, fechando os sockets e
    * finalizando as suas threads
    * 
    * Parâmetros: nenhum
    * Retorno: void
    */
    void RequestStop();
    void RequestStopSlave();
    void RequestStopTUI();

    //TODO: delete threads, tui, etc.
    virtual ~Server();
};