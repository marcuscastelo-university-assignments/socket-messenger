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

    struct
    {
        std::unordered_map<std::string, Socket> clientSockets = {};
        std::unordered_map<Socket, std::string> socketClients = {};

        std::unordered_map<std::string, Socket>::iterator find(const std::string &nick) { return clientSockets.find(nick); }
        std::unordered_map<Socket, std::string>::iterator find(const Socket &socket) { return socketClients.find(socket); }

        void insert(const std::string &nick, const Socket &socket)
        {
            clientSockets.insert({nick, socket});
            socketClients.insert({socket, nick});
        }

        bool erase(const std::string &nick)
        {
            auto it = find(nick);

            if (it == clientSockets.end()) return false;
            if (find(it->second) == socketClients.end()) throw std::runtime_error("Inconsistency in bimap! this IS a bug.");

            clientSockets.erase(it);
            socketClients.erase(it->second);

            return true;
        }

        bool erase(const Socket &socket)
        {
            return socketClients.erase(socket);
        }

    } biMapClientSocket;

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
                Socket::Send(destSocket->second, message.ToBuffer());
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
            if (it == server.biMapClientSocket.socketClients.end()) continue;
            
            SocketBuffer recBuf = Socket::Read(clientSocket);
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

void acceptClients(ServerInfo *server_p)
{
    ServerInfo &server = *server_p;
    std::cout << "Aceitando clientes!" << std::endl;
    while (server.running)
    {
        Socket clientSocket = server.socket.Accept();
        printf("\a\n");

        server.biMapClientSocket.insert("marucs", clientSocket);

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

    shutdown(server.socket.GetFD(), 2);
    close(server.socket.GetFD());

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
