#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <thread>
#include <iostream>
#include <vector>
#include <csignal>
#include <functional>
#include <unordered_map>
#include "tui.hpp"
using namespace tui::text_literals;

//Using operators ms, ns, etc..
#include <chrono>
using namespace std::chrono_literals;

#include "socket.hpp"

#include "server.hpp"

#include "server_tui.hpp"


/**
 * Função auxiliar que faz o parse para identificar o nome do cliente
 * 
 * Parâmetros:	SocketBuffer *recBuf	=>	Socket que recebe o buffer
 * 				char *command			=>	Comando recebido da mensagem
 * 				char *nick				=>	Nickname do cliente
 * 
 * Return: void
*/
void parseClientName(SocketBuffer *recBuf, char *command, char *nick){
	for (size_t i = 0; i < recBuf->len; i++)
    {
        if (recBuf->buf[i] == '=')
        {
            recBuf->buf[i] = '\0';
            nick = recBuf->buf + i + 1;
            return;
        }
    }
}

//TODO: ver se ta certo
/**
 * Função auxiliar que da a confirmação da conexão de um cliente
 * ao servidor e o adiciona no mapeamento
 * 
 * Parâmetros: ServerInfo& server			=>	Servidor em questão
 * 			   const Socket &clientSocket	=>	Socket do novo cliente
 * 
 * Return: bool	=>	Caso a adição seja validada - true
 * 					Se não, false
*/
bool Server::LoginUser(const Socket &clientSocket)
{
    SocketBuffer recBuf = clientSocket.Read();

    char *command = recBuf.buf;
    char *nick = recBuf.buf;
    
	parseClientName(&recBuf, command, nick);

    std::string strCommand(command);
    std::string strNick(nick);

    //Wrong command
    if (strCommand != "nick")
        return false;

    //No nick provided (nick pointer not changed)
    if (nick == recBuf.buf)
        return false;

    if (m_UserSockets.IsUserRegistered(nick))
    {
        //TODO: rejeitar usuários se o nick ja existir
    }

    m_UserSockets.RegisterUser(strNick, clientSocket);

    return true;
}

/**
 * Função que permite que o servidor aceite a entrada de clientes
 * e armazena sua thread no vector
 * 
 * Parâmetros: ServerInfor *server_p	=>	Servidor em questão
 * 
 * Return: void
*/
void Server::AcceptLoop()
{
    std::cout << "Aceitando clientes!" << std::endl;
    while (m_Running)
    {
        Socket clientSocket = m_Socket.Accept();
        m_ConnectedSockets.push_back(clientSocket);

        NotifyTUI("Nova conexão: " + clientSocket.GetAddress().ToString());

        //TODO: thread separada para evitar DoS com hanging auth
        while (!LoginUser(clientSocket))
        {
            static const char *errorMessage = "INVALID_NICK\0";
            static const size_t errorMessageLen = strlen(errorMessage) + 1;
            clientSocket.Send(SocketBuffer{errorMessage, errorMessageLen});
            std::this_thread::sleep_for(1s);
        }

        auto &nickname = m_UserSockets.GetUserNick(clientSocket);

        NotifyTUI("Conexão autenticada com sucesso, " + clientSocket.GetAddress().ToString() + " agora é identificado como " + nickname);

        std::thread *serverListenThread = new std::thread(std::bind(&Server::ClientLoop, this, std::ref(clientSocket)));
        m_Threads.push_back(serverListenThread);
    }
}

/**
    * Função responsável por receber e armazenar informações mandadas pelo cliente 
    *
    * Parâmetros: ServerInfo &server => armazena informações recebidas
    *             Socket clientSocket  => possui as informações a serem armazenadas
    *
    * Retorno: void
 */
void Server::ClientLoop(Socket &clientSocket)
{
    Socket &serverSocket = m_Socket;

    std::cout << "Escutando mensagens de "
              << clientSocket.GetAddress() << std::endl;
    while (m_Running)
    {
        try
        {
            if (!m_UserSockets.IsUserRegistered(clientSocket))
            {
                continue;
            }

            auto &nickname = m_UserSockets.GetUserNick(clientSocket);

            SocketBuffer recBuf = clientSocket.Read();
            Message message(nickname, recBuf);
            NotifyTUI(std::string("Mensagem recebida (nickname = " + nickname + "): from=") + message.FromUser + ", to=" + message.ToUser + ", content = \"" + message.Content + "\"");

            //TODO: change clientSocket for real destination
            m_MessagesToSend.push_back(message);
        }
        catch (ConnectionClosedException &e)
        {
            std::cout << "Server message receiving socket has closed" << std::endl;
            std::cout << "Reason: \t" << e.what() << std::endl;
            return;
        }
    }
}

void Server::CloseAllSockets()
{
    for (auto &socket : m_ConnectedSockets)
        socket.Close();
    m_Socket.Close();
}


/**
 * Função responsável por iniciar o servidor, executando o bind
 * e inicializando as threads necessárias
 * 
 * Parâmetros: 	ServerInfo *server_p	=>	Servidor em questão
 * 				bool waitThreads = true	=>	Bool que marca a espera por threads (clientes)
 * 
 * Retorno: void
*/
void Server::Start()
{
    m_Running = true;

    if (m_AcceptThread != nullptr)
        throw std::logic_error("Starting server slave twice!");

    m_Socket.Listen(m_MaxClients);

    m_AcceptThread = new std::thread(std::bind(&Server::AcceptLoop, this));

    std::thread *forwardMessageThread = new std::thread(std::bind(&Server::ForwardMessageLoop, this));
    m_Threads.push_back(forwardMessageThread);
}

void Server::EnterTUI()
{
    if (m_CurrentTUI != nullptr)
        throw std::logic_error("Starting server TUI twice!");
    m_CurrentTUI = new tui::ServerTUI(*this);

    tui::printl("Mudando para o modo interativo - Terminal User Interface (TUI)"_fgre);
    std::this_thread::sleep_for(900ms);

    m_CurrentTUI->Enter();
}

void Server::NotifyTUI(const std::string &notification)
{
    tui::savePos();
    tui::pauseReadline();

    tui::down(2);
    tui::delLineR();
    //TODO: notify TUI
    tui::printl(notification);

    tui::rbPos();
    tui::unpauseReadline();
}

/**
 * Função que encerra o server, fechando os sockets e
 * finalizando as suas threads
 * 
 * Parâmetros:	ServerInfor &server	=>	Servidor a ser fechado
 * 
 * Retorno: void
*/
void Server::RequestStop()
{
    RequestStopSlave();
    RequestStopTUI();
}
void Server::RequestStopSlave()
{
    m_Running = false;
    CloseAllSockets();
    for (auto &thread : m_Threads)
        thread->join();
    m_AcceptThread->join();
}
void Server::RequestStopTUI()
{
    if (m_CurrentTUI != nullptr)
        m_CurrentTUI->RequestStop();
}
/**
    * Função responsável por reenviar a mensagem recebida do cliente para o cliente alvo
    *
    * Parâmetros: Server &server => possui as mensagens a serem reenviadas
    *
    * Retorno: void
*/
void Server::ForwardMessageLoop()
{
    std::vector<Message> &messages = m_MessagesToSend;
    while (m_Running)
    {
        //TODO: mutex para messages
        //itera todas as mensagens pendentes atualmente no servidor
        for (size_t i = 0; i < messages.size(); i++)
        {
            Message &message = messages[i];

            const std::string &destUser = message.ToUser;

            if (!m_UserSockets.IsUserRegistered(destUser))
            {
                message.Content = "User " + message.ToUser + " not found (or offline)";
                message.ToUser = message.FromUser;
                message.FromUser = "System";
                if (!m_UserSockets.IsUserRegistered(message.ToUser))
                    continue;
            }

            auto &destSocket = m_UserSockets.GetUserSocket(destUser);
            NotifyTUI(std::string("Enviando mensagem: ") + message.Content + " para " + message.ToUser);

            //TODO: remove
            std::this_thread::sleep_for(1s);
            try
            {
                destSocket.Send(message.ToBuffer());
            }
            catch (ConnectionClosedException &e)
            {
                std::cout << "Server message resent socket has closed" << std::endl;
                std::cout << "Reason: \t" << e.what() << std::endl;
                return;
            }
            catch (std::exception &e)
            {
                std::cout << "Unexpected exception: \t" << e.what() << std::endl;
                return;
            }
        }

        messages.clear();
        std::this_thread::sleep_for(1s);
    }
}

Server::~Server()
{
    CloseAllSockets();
    m_ConnectedSockets.clear();
    for (auto& thread : m_Threads) { thread->join(); delete thread; }
    m_AcceptThread->join();
    delete m_AcceptThread;
    m_Threads.clear();
    m_AcceptThread = nullptr;
}