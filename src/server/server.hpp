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

#include <chrono>
using namespace std::chrono_literals;

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
    * Função auxiliar que registra o nick do usuário e seu socket correspondente impedindo repetições
    * 
    * Parâmetros:  const Socket &clientSocket	=>	Socket do  cliente 
    * 
    * Return: bool	=>	Caso o login seja bem sucedido - true
    * 					Se não, false
    */
    bool LoginUser(const Socket &clientSocket);

    /**
    * Função responsavel pela entrada de clientes, armazenando seus sockets e nicknames e 
    * inicializa o thread de recebimento de mensagens 
    *
    * Parâmetros: Nenhum
    * 
    * Return: Void
    */
    void AcceptLoop();

    /**
    * Função responsável por receber e armazenar mensagens enviadas pelo cliente 
    *
    * Parâmetros: Socket clientSocket  => possui as informações a serem armazenadas
    *
    * Retorno: Void
    */
    void ClientLoop(Socket clientSocket);

    /**
    * Função responsável por reenviar a mensagem recebida do cliente para o cliente alvo
    *
    * Parâmetros: Nenhum
    *
    * Retorno: Void
    */
    void ForwardMessageLoop();

    /**
    * Função que fecha os sockets de todos os clientes registrados e o do proprio servidor
    * 
    * Parâmetros: Nenhum
    * 
    * Retorno: Void
    */
    void CloseAllSockets();

    /**
    * Função que atualiza a lista de clientes conectados ao server e as manda para todos os clientes
    * 
    * Parâmetros: Nenhum
    * 
    * Retorno: Void
    */
    void OnClientCountChanged();

public:
    Server(IPADDR4 address, int maxClients);

    inline UserSockets &GetUserSockets() { return m_UserSockets; }

    /**
    * Função que identifica o socket desconectado e o remove da lista de clientes conectados
    * após isso chama onClientChanged para atualizar os headers dos clientes
    * 
    * Parâmetros: Nenhum
    * 
    * Retorno: Void
    */
    void OnSocketClosed(const Socket &closedSocket);

    /**
    * Função responsável por iniciar o servidor inicializando as threads necessárias para 
    * gerenciar o recebimento e envio de informações
    *
    * Parâmetros: Nenhum			
    * 
    * Retorno: Void
    */
    void Start();

    /**
    * Função que inicializa a TUI
    * 
    * Parâmetros: Nenhum
    * 
    * Retorno: Void
    */
    void EnterTUI();

    /**
    * Função que encerra o server, fechando os sockets e
    * finalizando as suas threads
    * 
    * Parâmetros: nenhum
    * Retorno: void
    */
    void RequestStop();

    /**
    * Função que fecha todos os sockets, bloqueia todos os threads 
    * e para os as funções de loop 
    *
    * Parâmetros: Nenhum
    * 
    * Retorno: Void
    */
    void RequestStopSlave();

    /**
    * Função que encerra a TUI
    * 
    * Parâmetros: Nenhum
    * 
    * Retorno: Void
    */
    void RequestStopTUI();

    /**
     * Função que kicka um usuário do servidor inicialmente espera a boa vontade do cliente em se desconectar. 
     * Após 2 segundos, força a desconexão
     * 
     * Parâmetros: Nenhum
     * 
     * Retorno: Void
     */
    inline void Kick(const Socket &clientSocket)
    {
        OnSocketClosed(clientSocket);
        
        std::thread temp([&clientSocket]() {
            clientSocket.Send({"bye", 4});
            std::this_thread::sleep_for(2s);
            clientSocket.Shutdown();
        });
        temp.detach();
    }

    inline bool IsRunning() { return m_Running; }


     /**
     * Função que encera o servidor caso este esteja online e deleta sua TUI
     * 
     * Parâmetros: Nenhum
     * 
     * Retorno: Void
     */
    virtual ~Server();
    //TODO: delete threads, tui, etc.
};