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
    * Função responsável por reenviar a mensagem recebida do cliente para o cliente alvo
    *
    * Parâmetros: ServerInfo *server_p => possui as mensagens a serem reenviadas
    *
    * Retorno: void
*/
void resendmessage(ServerInfo *server_p)
{
    ServerInfo &server = *server_p;

    std::vector<Message> &messages = server.pendingMessages;
    while (server.running)
    {
        //TODO: mutex para messages
        //itera todas as mensagens pendentes atualmente no servidor
        for (size_t i = 0; i < messages.size(); i++)
        {
            Message &message = messages[i];

            const std::string &destUser = message.ToUser;
            auto destSocket = server.biMapClientSocket.clientSockets.find(destUser);

            if (destSocket == server.biMapClientSocket.clientSockets.end())
            {
                message.Content = "User " + message.ToUser + " not found (or offline)";
                message.ToUser = message.FromUser;
                message.FromUser = "System";
                destSocket = server.biMapClientSocket.clientSockets.find(message.ToUser);
                if (destSocket == server.biMapClientSocket.clientSockets.end())
                    continue;
            }

            std::cout << "Enviando mensagem: " << message.Content << " para " << message.ToUser << std::endl;
            try
            {
                destSocket->second.Send(message.ToBuffer());
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

/**
    * Função responsável por receber e armazenar informações mandadas pelo cliente 
    *
    * Parâmetros: ServerInfo *server_p => armazena informações recebidas
    *             Socket clientSocket  => possui as informações a serem armazenadas
    *
    * Retorno: void
 */
void listenMessages(ServerInfo *server_p, Socket clientSocket)
{
    ServerInfo &server = *server_p;
    Socket &serverSocket = server.socket;

    std::cout << "Escutando mensagens de "
              << clientSocket.GetAddress() << std::endl;
    while (server.running)
    {
        try
        {
            auto it = server.biMapClientSocket.socketClients.find(clientSocket);
            if (it == server.biMapClientSocket.socketClients.end())
                continue;

            SocketBuffer recBuf = clientSocket.Read();
            Message message(it->second, recBuf);
            printf("Recebido dados: %s\n", recBuf.buf);
            printf("Convertido de volta dados: %s\n", message.ToBuffer().buf);
            //TODO: change clientSocket for real destination
            server.pendingMessages.push_back(message);
        }
        catch (ConnectionClosedException &e)
        {
            std::cout << "Server message receiving socket has closed" << std::endl;
            std::cout << "Reason: \t" << e.what() << std::endl;
            return;
        }
    }
}

/**
     * Função responsável por criar o server
     *
     * Parâmetros: IPADDR4 address = {"0.0.0.0", 4545} => valores de porta 
     *             
     *
     * Retorno: ServerInfo => o server criado
    */
ServerInfo createServer(IPADDR4 address = {"0.0.0.0", 4545})
{
    Socket serverSocket(SocketType::TCP);

    ServerInfo server(serverSocket, address, 10); //maxClients = 10

    return server;
}

void joinServerThreads(const ServerInfo &server)
{
    for (size_t i = 0; i < server.threadsVector.size(); i++)
    {
        server.threadsVector[i]->join();
    }
}

void printClientStats(Socket clientSocket)
{
    std::cout << "Um cliente se conectou! IP = [";
    std::cout << clientSocket.GetAddress();
    std::cout << "]" << std::endl;
}

bool waitForIdentification(ServerInfo& server, const Socket &clientSocket)
{
    SocketBuffer recBuf = clientSocket.Read();

    //TODO: function for this ugly logic
    char *command = recBuf.buf;
    char *nick = recBuf.buf;
    for (size_t i = 0; i < recBuf.len; i++)
    {
        if (recBuf.buf[i] == '=')
        {
            recBuf.buf[i] = '\0';
            nick = recBuf.buf + i + 1;
            break;
        }
    }
    std::string strCommand(command);
    std::string strNick(nick);

    //Wrong command
    if (strCommand != "nick")
        return false;

    //No nick provided (nick pointer not changed)
    if (nick == recBuf.buf)
        return false;

    server.biMapClientSocket.insert(strNick, clientSocket);

    return true;
}

void acceptClients(ServerInfo *server_p)
{
    ServerInfo &server = *server_p;
    std::cout << "Aceitando clientes!" << std::endl;
    while (server.running)
    {
        Socket clientSocket = server.socket.Accept();

        //TODO: tirar
        printf("\a\n");

        while (!waitForIdentification(server, clientSocket))
        {
            static const char *errorMessage = "INVALID_NICK\0";
            static const size_t errorMessageLen = strlen(errorMessage) + 1;
            clientSocket.Send(SocketBuffer{errorMessage, errorMessageLen}); 
            std::this_thread::sleep_for(1s);
        }

        printClientStats(clientSocket);

        std::thread *serverListenThread = new std::thread(listenMessages, &server, clientSocket);
        server.threadsVector.push_back(serverListenThread);
    }
}

void startServer(ServerInfo *server_p, bool waitThreads = true)
{
    ServerInfo &server = *server_p;
    server.running = true;
    try
    {
        server.socket.Bind(server.address);
    }
    catch (SocketBindException &e)
    {
        std::cerr << e.what() << std::endl;
        exit(-1);
    }

    std::thread *serverResendMessagesThread = new std::thread(resendmessage, &server);
    server.threadsVector.push_back(serverResendMessagesThread);

    server.socket.Listen(server.maxClients);

    std::thread *acceptClientThread = new std::thread(acceptClients, &server);
    server.threadsVector.push_back(acceptClientThread);

    if (waitThreads)
        joinServerThreads(server);
}

void endServer(ServerInfo &server)
{
    server.running = false;

    for (size_t i = 0; i < server.threadsVector.size(); i++)
        delete (server.threadsVector[i]);

    shutdown(server.socket.GetFD(), 2);
    close(server.socket.GetFD());

    server.threadsVector.clear();
}

int main(int argc, char const *argv[])
{
    tui::clear();
    ServerInfo server = createServer();

    tui::printl("Inicializando o Zaplan Server..."_fgre);
    tui::down(2);

    std::thread serverThread(startServer, &server, true);
    server.serverThread = &serverThread;
    
    tui::ServerTUI serverTui(server);
    
    tui::printl("Mudando para o modo interativo - Terminal User Interface (TUI)"_fgre);
    std::this_thread::sleep_for(900ms);
    serverTui.Enter();

    // endServer(server);

    tui::printl("Todas as threads foram encerradas com sucesso!"_fblu);
    return 0;
}
