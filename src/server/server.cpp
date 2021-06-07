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
    * Parâmetros: ServerInfo &server => possui as mensagens a serem reenviadas
    *
    * Retorno: void
*/
void resendmessage(ServerInfo &server)
{
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
    * Parâmetros: ServerInfo &server => armazena informações recebidas
    *             Socket clientSocket  => possui as informações a serem armazenadas
    *
    * Retorno: void
 */
void listenMessages(ServerInfo &server, Socket clientSocket)
{
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

/**
 * Função que faz o join das threads no servidor
 * 
 * Parâmetros: const ServerInfo &server	=>	Referência para o servidor em questão
 * 
 * Retorno: void
*/
void joinServerThreads(const ServerInfo &server)
{
    for (size_t i = 0; i < server.threadsVector.size(); i++)
    {
        server.threadsVector[i]->join();
    }
}

/**
 * Função que printa quando um cliente se conectou e qual seu ip:porta
 * 
 * Parâmetros: Socket clientSocket	=>	Socket do cliente conectado
 * 
 * Retorno: void
*/
void printClientStats(Socket clientSocket)
{
    std::cout << "Um cliente se conectou! IP = [";
    std::cout << clientSocket.GetAddress();
    std::cout << "]" << std::endl;
}

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

/**
 * Função que permite que o servidor aceite a entrada de clientes
 * e armazena sua thread no vector
 * 
 * Parâmetros: ServerInfor *server_p	=>	Servidor em questão
 * 
 * Return: void
*/
void acceptClients(ServerInfo *server_p)
{
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

        std::thread *serverListenThread = new std::thread(listenMessages, std::ref(server), clientSocket);
        server.threadsVector.push_back(serverListenThread);
    }
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
void startServer(ServerInfo *server_p, bool waitThreads = true)
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

    std::thread *serverResendMessagesThread = new std::thread(resendmessage, std::ref(server));
    server.threadsVector.push_back(serverResendMessagesThread);

    server.socket.Listen(server.maxClients);

    std::thread *acceptClientThread = new std::thread(acceptClients, std::ref(server));
    server.threadsVector.push_back(acceptClientThread);

    if (joinThreads)
        joinServerThreads(server);
}

/**
 * Função que encerra o server, fechando os sockets e
 * finalizando as suas threads
 * 
 * Parâmetros:	ServerInfor &server	=>	Servidor a ser fechado
 * 
 * Retorno: void
*/
void endServer(ServerInfo &server)
{
    server.running = false;

    for (size_t i = 0; i < server.threadsVector.size(); i++)
        delete (server.threadsVector[i]);

    shutdown(server.socket.GetFD(), 2);
    close(server.socket.GetFD());

    server.threadsVector.clear();
}

/**
 * Função main do servidor, que faz a chamada na ordem correta
 * dos métodos:
 * 				- Cria o servidor
 * 				- Iniciliza o servidor
 * 				- Executa a tui
 * 				- Gerencia todas as funções
 * 				- Encerra o servidor
*/
int main(int argc, char const *argv[])
{
    tui::clear();
    ServerInfo server = createServer();

    tui::printl("Inicializando o Zaplan Server..."_fgre);
    tui::down(2);

    std::thread serverThread(startServer, std::ref(server), true);
    server.serverThread = &serverThread;
    
    tui::ServerTUI serverTui(server);
    
    tui::printl("Mudando para o modo interativo - Terminal User Interface (TUI)"_fgre);
    std::this_thread::sleep_for(900ms);
    serverTui.Enter();

    // endServer(server);

    tui::printl("Todas as threads foram encerradas com sucesso!"_fblu);
    return 0;
}
