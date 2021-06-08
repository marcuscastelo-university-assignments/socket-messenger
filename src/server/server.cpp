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
#include <mutex>

using guard = std::lock_guard<std::mutex>;

Server::Server(IPADDR4 address, int maxClients) : m_Socket(SocketType::TCP), m_Address(address), m_MaxClients(maxClients)
{
    m_CurrentTUI = new tui::ServerTUI(*this);

    try
    {
        m_Socket.Bind(m_Address);
    }
    catch (SocketBindException &e)
    {
        tui::print(e.what() + "\n"_fred);
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

bool Server::RegisterUser(User &user)
{

    SocketBuffer recBuf = user.m_Socket->Read();

    //Comando incorreto
    if (strncmp(recBuf.buf, "nick", 4) != 0)
        return false;

    char *command = recBuf.buf;
    char *nick = recBuf.buf;
    parseClientName(&recBuf, command, nick);
    std::string strCommand(command);
    std::string strNick(nick);

    //No nick provided (nick pointer not changed)
    if (nick == recBuf.buf)
        return false;

    if (m_UserSockets.IsUserRegistered(nick))
    {
        return false;
    }

    user.m_Nick = strNick;
    m_UserSockets.RegisterUser(user);

    return true;
}

void Server::AcceptLoop()
{
    m_CurrentTUI->Notify("Aguardando conexões de clientes..."_fgre);
    while (m_Running)
    {
        User currentUser;

        try
        {
            currentUser.m_Socket = m_Socket.Accept();
        }
        catch (SocketAcceptException &e)
        {
            const static std::string text = "O servidor não mais aceita clientes!"_fmag;

            //Se o usuário terminou o programa com SIGNIT, talvez a TUI não exista mais,
            //Mas o commando "stop" permite que o servidor seja encerrado sem fechar a TUI. Nesse caso, é preferido que o método Notify seja usado
            if (m_CurrentTUI != nullptr)
                m_CurrentTUI->Notify(text);
            else
                tui::printl(text);
            continue;
        }

        m_CurrentTUI->Notify("Nova conexão: "_fgre + currentUser.m_Socket->GetAddress().ToString());

        //TODO: tirar sleeps
        std::this_thread::sleep_for(2s);

        try
        {
            while (!RegisterUser(currentUser))
            {
                static const char *errorMessage = "INVALID_NICK\0";
                static const size_t errorMessageLen = strlen(errorMessage) + 1;
                currentUser.m_Socket->Send(SocketBuffer{errorMessage, errorMessageLen});

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

        OnClientCountChanged();

        m_CurrentTUI->Notify("Conexão autenticada com sucesso, "_fgre + tui::text::Text{currentUser.m_Socket->GetAddress().ToString()}.FWhite() + " agora é identificado como "_fgre + tui::text::Text{currentUser.m_Nick}.FYellow().Bold());

        std::thread *clientLoopThread = new std::thread(std::bind(&Server::ClientLoop, this, currentUser));
        m_Threads.push_back(clientLoopThread);
    }
}

void Server::OnSocketClosed(SocketRef closedSocket)
{
    m_UserSockets.UnregisterUser(*closedSocket.get());
    OnClientCountChanged();
}

void Server::ClientLoop(User user)
{
    Socket &serverSocket = m_Socket;

    if (!m_UserSockets.IsUserRegistered(user.m_Nick))
        return;

    while (m_Running)
    {
        try
        {
            SocketBuffer recBuf = user.m_Socket->Read();
            Message message(user.m_Nick, recBuf);

            m_CurrentTUI->Notify("Mensagem enfileirada: "_fwhi + tui::text::Text{user.m_Nick}.FYellow().Bold() + " -> "_fcya + tui::text::Text{message.ToUser}.FYellow().Bold() + " = \"" + message.Content + "\"");
            {
                guard g_(m_Mutexes.m_MessagesToSendMutex);
                m_MessagesToSend.push_back(message);
            }
        }
        catch (ConnectionClosedException &e)
        {
            OnSocketClosed(user.m_Socket);

            //FIXME: isso ta certo??
            user.m_Socket->Shutdown();

            if (m_Running)
                m_CurrentTUI->Notify(tui::text::Text{user.m_Nick}.FYellow().Bold() + " desconectou"_fmag);
            return;
        }
    }
}

void Server::CloseAllSockets()
{
    auto usersVecCopy = m_UserSockets.ListUsers();

    while (!usersVecCopy.empty())
    {
        auto &user = usersVecCopy.back();
        Kick(user.m_Socket);
        usersVecCopy.pop_back();
    }

    std::this_thread::sleep_for(4s);

    m_UserSockets.UnregisterAllUsers();

    m_Socket.Shutdown();
}

void Server::OnClientCountChanged()
{
    std::stringstream onlineSS;

    onlineSS << "online=";

    auto onlineUsersCpy = m_UserSockets.ListUsers();
    int len = onlineUsersCpy.size();
    for (const User &onlineUser : onlineUsersCpy)
    {
        onlineSS << onlineUser.m_Nick;
        if (len-- > 1)
            onlineSS << ", ";
    }

    std::string payload(onlineSS.str());

    for (const User &onlineUser : onlineUsersCpy)
    {
        //FIXME: native address is 0 here, why?
        try
        {
            onlineUser.m_Socket->Send({payload.c_str(), payload.length() + 1});
        }
        catch (const std::exception &e)
        {
            //Ignore unavailable users
            //TODO: maybe remove them? IDK
            continue;
        }
    }
    if (m_CurrentTUI != nullptr)
        m_CurrentTUI->SetOnline(payload.c_str() + 7); //Everything after online=
}

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

    tui::printl("Mudando para o modo interativo - Terminal User Interface (TUI)"_fgre);
    std::this_thread::sleep_for(900ms);

    m_CurrentTUI->Enter();
}

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
}

void Server::RequestStopTUI()
{
    if (m_CurrentTUI != nullptr)
        m_CurrentTUI->RequestStop();
}

void Server::ForwardMessageLoop()
{

    while (m_Running)
    {
        std::vector<Message> messages = {};

        {
            guard g_(m_Mutexes.m_MessagesToSendMutex);
            if (m_MessagesToSend.empty()) continue;

            std::swap(messages, m_MessagesToSend);
        }
        
        //itera todas as mensagens pendentes atualmente no servidor
        for (size_t i = 0; i < messages.size(); i++)
        {
            Message &message = messages[i];

            const std::string &destUserNick = message.ToUser;

            if (!m_UserSockets.IsUserRegistered(destUserNick))
            {
                message.Content = "User " + message.ToUser + " not found (or offline)";
                message.ToUser = message.FromUser;
                message.FromUser = "System";
                if (!m_UserSockets.IsUserRegistered(message.ToUser))
                    continue;
            }

            auto &destUser = *m_UserSockets.FindByNick(destUserNick);
            m_CurrentTUI->Notify("Enviando mensagem:"_fwhi + tui::text::Text{" \"" + message.Content + "\""}.FCyan() + " de " + tui::text::Text{message.FromUser}.FYellow().Bold() + " para " + tui::text::Text{message.ToUser}.FYellow().Bold());

            std::this_thread::sleep_for(1s);
            try
            {
                destUser.m_Socket->Send(message.ToBuffer());
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
    if (m_Running)
        RequestStopSlave();
    delete m_CurrentTUI;
}