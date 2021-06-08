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

#include <memory>

Server::Server(IPADDR4 address, int maxClients) : m_Socket(SocketType::TCP), m_Address(address), m_MaxClients(maxClients)
{
    m_CurrentTUI = new tui::ServerTUI(*this);

    try
    {
        m_Socket.Bind(m_Address);
    }
    catch (SocketBindException &e)
    {
        tui::print(e.what()+"\n"_fred);
        exit(-1);
    }
}

/**
 * Função auxiliar que faz o parse para identificar o nome do cliente
 * 
 * Parâmetros:	SocketBuffer *recBuf	=>	Socket que recebe o buffer
 * 				char *command			=>	Comando recebido da mensagem
 * 				char *nick				=>	Nickname do cliente
 * 
 * Return: void
*/
void parseClientName(SocketBuffer *recBuf, char *&command, char *&nick)
{
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
 * Função auxiliar que registra o nick do usuário e seu socket correspondente impedindo repetições
 * 
 * Parâmetros:  const Socket &clientSocket	=>	Socket do  cliente 
 * 
 * Return: bool	=>	Caso o login seja bem sucedido - true
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
 * Função que permite a entrada de clientes, armazenando seus sockets e nicknames e 
 * inicializa o thread de recebimento de mensagens 
 *
 * Parâmetros: Nenhum
 * 
 * Return: Void
*/
void Server::AcceptLoop()
{
    m_CurrentTUI->Notify("Aguardando conexões de clientes..."_fgre);
    while (m_Running)
    {
        std::unique_ptr<Socket> clientSocket;
        try
        {
            clientSocket.reset(new Socket(m_Socket.Accept()));
        }
        catch (SocketAcceptException &e)
        {
            m_CurrentTUI->Notify("O servidor não mais aceita clientes!"_fmag);
            continue;
        }
        m_ConnectedSockets.push_back(*clientSocket);

        m_CurrentTUI->Notify("Nova conexão: "_fgre + clientSocket->GetAddress().ToString());
        std::this_thread::sleep_for(2s);

        //TODO: thread separada para evitar DoS com hanging auth
        try
        {
            while (!LoginUser(*clientSocket))
            {
                static const char *errorMessage = "INVALID_NICK\0";
                static const size_t errorMessageLen = strlen(errorMessage) + 1;
                clientSocket->Send(SocketBuffer{errorMessage, errorMessageLen});

                std::this_thread::sleep_for(1s);
            }
        }
        catch (ConnectionClosedException &e)
        {
            m_CurrentTUI->Notify(tui::text::Text{e.what()}.FRed());
            std::this_thread::sleep_for(5s);
            continue;
        }
        catch (std::runtime_error &e)
        {
            m_CurrentTUI->Notify("Buffer recebido e enviado diferem!!"_fred + "\n\tDetalhes: " + e.what());
            std::this_thread::sleep_for(5s);
        }

        auto &nickname = m_UserSockets.GetUserNick(*clientSocket);

        m_CurrentTUI->Notify("Conexão autenticada com sucesso, "_fgre + tui::text::Text{clientSocket->GetAddress().ToString()}.FWhite() + " agora é identificado como "_fgre + tui::text::Text{nickname}.FYellow().Bold());

        std::thread *serverListenThread = new std::thread(std::bind(&Server::ClientLoop, this, *clientSocket));
        m_Threads.push_back(serverListenThread);
    }
}

/**
    * Função responsável por receber e armazenar mensagens enviadas pelo cliente 
    *
    * Parâmetros: Socket clientSocket  => possui as informações a serem armazenadas
    *
    * Retorno: Void
 */
void Server::ClientLoop(Socket clientSocket)
{
    Socket &serverSocket = m_Socket;

    // std::cout << "Escutando mensagens de "
    //           << clientSocket.GetAddress() << std::endl;
    while (m_Running)
    {
        try
        {
            if (!m_UserSockets.IsUserRegistered(clientSocket))
                continue;

            auto &nickname = m_UserSockets.GetUserNick(clientSocket);

            SocketBuffer recBuf = clientSocket.Read();
            Message message(nickname, recBuf);
            m_CurrentTUI->Notify("Mensagem enfileirada: "_fwhi + tui::text::Text{nickname}.FYellow().Bold() + " -> "_fcya + tui::text::Text{message.ToUser}.FYellow().Bold() + " = \"" + message.Content + "\"");
            //TODO: remover esta e outras linhas de sleep_for para notify (fazer um esquema de buffer de print)
            std::this_thread::sleep_for(2s);
            
            //TODO: change clientSocket for real destination
            m_MessagesToSend.push_back(message);
        }
        catch (ConnectionClosedException &e)
        {
            m_CurrentTUI->Notify("Server message receiving socket has closed\n"_fred);
            m_CurrentTUI->Notify(tui::text::Text{"Reason:\t"_t + e.what() + "\n"}.FRed());
            return;
        }
    }
}

/**
 * Função que fecha os sockets de todos os clientes logados e o do proprio servidor
 * 
 * Parâmetros: Nenhum
 * 
 * Retorno: Void
*/
void Server::CloseAllSockets()
{
    for (auto &socket : m_ConnectedSockets)
        socket.Close();
    m_Socket.Close();
}

/**
 * Função responsável por iniciar o servidor inicializando as threads necessárias para 
 * gerenciar o recebimento e envio de informações
 *
 * Parâmetros: Nenhum			
 * 
 * Retorno: Void
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

/**
 * Função que inicializa a TUI
 * 
 * Parâmetros: Nenhum
 * 
 * Retorno: Void
*/
void Server::EnterTUI()
{

    tui::printl("Mudando para o modo interativo - Terminal User Interface (TUI)"_fgre);
    std::this_thread::sleep_for(900ms);

    m_CurrentTUI->Enter();
}
/**
 * Função que encerra o server e a TUI
 * 
 * Parâmetros: Nenhum
 * 
 * Retorno: Void
*/
void Server::RequestStop()
{
    RequestStopSlave();
    RequestStopTUI();
}
/**
 * Função que fecha todos os sockets, bloqueia todos os threads 
 * e para os as funções de loop 
 *
 * Parâmetros: Nenhum
 * 
 * Retorno: Void
*/
void Server::RequestStopSlave()
{
    m_Running = false;
    CloseAllSockets();
    for (auto &thread : m_Threads)
        thread->join();
    m_AcceptThread->join();
}

/**
 * Função que encerra a TUI
 * 
 * Parâmetros: Nenhum
 * 
 * Retorno: Void
*/
void Server::RequestStopTUI()
{
    if (m_CurrentTUI != nullptr)
        m_CurrentTUI->RequestStop();
}

/**
    * Função responsável por reenviar a mensagem recebida do cliente para o cliente alvo
    *
    * Parâmetros: Nenhum
    *
    * Retorno: Void
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
            m_CurrentTUI->Notify("Enviando mensagem:"_fwhi +  tui::text::Text{" \"" + message.Content + "\""}.FCyan() + " de " + tui::text::Text{message.FromUser}.FYellow().Bold() +" para " + tui::text::Text{message.ToUser}.FYellow().Bold());

            //TODO: remove
            std::this_thread::sleep_for(1s);
            try
            {
                destSocket.Send(message.ToBuffer());
            }
            catch (ConnectionClosedException &e)
            {
                m_CurrentTUI->Notify("Server message resent socket has closed\n"_fred);
                m_CurrentTUI->Notify(tui::text::Text{"Reason: \t"_t + e.what() + "\n"}.FRed());
                return;
            }
            catch (std::exception &e)
            {
                m_CurrentTUI->Notify(tui::text::Text{"Unexpected exception: \t"_t + e.what()}.FRed());
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
    for (auto &thread : m_Threads)
    {
        if (thread->joinable())
            thread->join();
        delete thread;
    }
    if (m_AcceptThread != nullptr && m_AcceptThread->joinable())
        m_AcceptThread->join();
    delete m_AcceptThread;
    m_Threads.clear();
    m_AcceptThread = nullptr;

    delete m_CurrentTUI;
}