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

    bool LoginUser(const Socket &clientSocket);

    void AcceptLoop();
    void ClientLoop(Socket &clientSocket);

    void ForwardMessageLoop();

    void CloseAllSockets();

public:
    Server(IPADDR4 address, int maxClients) : m_Socket(SocketType::TCP), m_Address(address), m_MaxClients(maxClients)
    {
        try
        {
            m_Socket.Bind(m_Address);
        }
        catch (SocketBindException &e)
        {
            std::cerr << e.what() << std::endl;
            exit(-1);
        }
    }

    void Start();
    void EnterTUI();

    inline bool IsRunning() { return m_Running; }

    void NotifyTUI(const std::string &notification);

    void RequestStop();
    void RequestStopSlave();
    void RequestStopTUI();

    //TODO: delete threads, tui, etc.
    virtual ~Server();
};