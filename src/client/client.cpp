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

#include <signal.h>

//Using operators ms, ns, etc..
#include <chrono>
using namespace std::chrono_literals;

#include "socket.hpp"
#include "client_tui.hpp"
#include "tui.hpp"
using namespace tui::text_literals;

#include <functional>

// maxtrials = 0 para infinitas tentativas (nunca desistir)
void Client::ConnectAndLogin(const IPADDR4 &serverAddr, const std::string &nickname, size_t maxTrials)
{
    m_Nickname = nickname;

    bool success = false;
    for (size_t trial = 1; maxTrials == 0 || trial <= maxTrials; trial++)
    {
        try
        {
            m_Socket.Connect(serverAddr);
            success = true;
            break;
        }
        catch (ConnectionFailedException &e)
        {
            std::this_thread::sleep_for(2.5s);
            tui::print(tui::text::Text("Unable to connect to " + serverAddr.ToString() + " Trial " + std::to_string(trial) + " of " + std::to_string(maxTrials) + ". \nReason:\t ").FRed());
            std::cerr << e.what() << std::endl;
        }
    }

    if (!success)
        throw ConnectionFailedException();
    success = false;

    try
    {
        std::string bufContet = "nick=" + nickname;
        SocketBuffer data{bufContet.c_str(), bufContet.length() + 1};
        m_Socket.Send(data);
    }
    catch (ConnectionClosedException &e)
    {
        std::stringstream ss;
        ss << "Unable to send username to server"
           << ".\nReason:\n\t ";
        ss << e.what() << std::endl;
        tui::print(tui::text::Text{ss.str()}.FRed());
        RequestExit();
    }
}

void Client::Start()
{
    //TODO: mutex para exiting
    if (!m_Exiting)
    {
        m_ServerSlaveThread = new std::thread(std::bind(&Client::ServerSlaveLoop, this));
        m_CurrentTUI = new tui::ClientTUI(*this);
        m_CurrentTUI->Enter();
    }
}

void Client::ServerSlaveLoop()
{
    std::this_thread::sleep_for(1s);
    while (!m_Exiting)
    {
        try
        {
            SocketBuffer data = m_Socket.Read();

            int compare = strncmp(data.buf, "msg=", 4 * sizeof(char));
            if (compare == 0)
            {
                Message receivedMessage(data);
                m_Messages.push_back(receivedMessage);
                m_CurrentTUI->Notify("<<<"_fcya + " (" + Text{receivedMessage.FromUser}.FYellow().Bold() + "): " + receivedMessage.Content);
            }
            else if (strncmp(data.buf, "online=", 7) == 0) {
                //TODO: deixar mais bonito
                m_CurrentTUI->SetOnline(data.buf+7);
            }
            else
            {
                m_CurrentTUI->Notify("Unkown data received from server: "_t + data.buf);
            }
        }
        catch (ConnectionClosedException &e)
        {
            tui::printl("Error reading server data, connection closed."_fred);
            //TODO: print errno message
            RequestExit();
        }
    }
}

void Client::SendMessage(const Message &message, size_t maxTries)
{
    try
    {
        m_Socket.Send(message.ToBuffer());
        m_Messages.push_back(message);
    }
    catch (const ConnectionClosedException &e)
    {
        tui::printl("Connection to the server lost while sending message"_fred);
        RequestExit();
    }
}

void Client::RequestExit()
{
    m_Exiting = true;
    CloseSockets();
    if (m_CurrentTUI != nullptr)
        m_CurrentTUI->RequestExit();

    if (m_ServerSlaveThread != nullptr && m_ServerSlaveThread->joinable())
        m_ServerSlaveThread->detach();
}

//TODO: check if properly stopped
Client::~Client()
{
    delete m_CurrentTUI;
    delete m_ServerSlaveThread;
}