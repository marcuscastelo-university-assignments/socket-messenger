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

//Using operators ms, ns, etc..
#include <chrono>
using namespace std::chrono_literals;

#include "socket.hpp"

//Estrutura auxiliar que armazena informações essenciais para o server
struct ServerInfo
{
    Socket socket;
    IPADDR4 address;

    int maxClients;
    bool running = false;

    std::vector<std::thread *> threadsVector = {};

    std::vector<Message> pendingMessages = {};

    ServerInfo(Socket serverSocket, IPADDR4 address, int maxClients) : socket(serverSocket), address(address), maxClients(maxClients) {}
};

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
            const Socket &clientSocket = messages[i].Dest; 
            std::cout << "Enviando mensagem: " << std::string((char *)messages[i].Data.buf) << std::endl;
            try
            {
                Socket::Send(clientSocket, messages[i].Data);
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
            MessageData data(Socket::Read(clientSocket));
            printf("Recebido dados: %s\n", data.buf);
            //TODO: change clientSocket for real destination
            Message message(data, clientSocket);
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

void acceptClients(ServerInfo *server_p)
{
    ServerInfo &server = *server_p;
    std::cout << "Aceitando clientes!" << std::endl;
    while (server.running)
    {
        Socket clientSocket = server.socket.Accept();
        printf("\a\n");

        printClientStats(clientSocket);

        std::thread *serverListenThread = new std::thread(listenMessages, &server, clientSocket);
        server.threadsVector.push_back(serverListenThread);
    }
}

void startServer(ServerInfo &server, bool waitThreads = true)
{
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

    shutdown(server.socket.getFD(), 2);
    close(server.socket.getFD());

    server.threadsVector.clear();
}

int main(int argc, char const *argv[])
{
    std::cout << "Criando servidor..." << std::endl;
    ServerInfo server = createServer();

    auto handleSocketDestruction = [&server](int sig)
    {
        std::cout << "To vindo com " << sig << std::endl;

        // server.socket.Close();

        // std::cout << "Servidor encerrado abruptamente (" << sig << ") received.\n";
        // server.running = false;
    };

    signal(SIGKILL, (void (*)(int)) & handleSocketDestruction);
    signal(SIGTERM, (void (*)(int)) & handleSocketDestruction);
    signal(SIGINT, (void (*)(int)) & handleSocketDestruction);
    signal(SIGQUIT, (void (*)(int)) & handleSocketDestruction);
    signal(SIGTSTP, (void (*)(int)) & handleSocketDestruction);
    signal(SIGSEGV, (void (*)(int)) & handleSocketDestruction);

    std::cout << "Iniciando servidor..." << std::endl;
    startServer(server, true);

    std::cout << "Fechando o servidor..." << std::endl;
    endServer(server);

    return 0;
}
