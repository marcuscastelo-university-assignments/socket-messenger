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

Socket serverSocket(SocketType::TCP);

bool isTuiRunning = false;

void startTUI()
{
    isTuiRunning = true;

    int enviados;
    char mensagem[1024];
    do
    {
        printf(">> Digite uma mensagem: ");
        memset(mensagem, '\0', strlen(mensagem));
        fgets(mensagem, 1024, stdin);

        //Se a stdin foi fechada, nem tenta enviar para o servidor
        if (strlen(mensagem) == 0)
            return;

        mensagem[strlen(mensagem) - 1] = '\0';
        try
        {
            serverSocket.Send({mensagem, strlen(mensagem) + 1});
        }
        catch (ConnectionClosedException &e)
        {
            std::cout << "Unable to send message. Is server closed?" << std::endl;
            std::cout << "Reason:\t" << e.what() << std::endl;
        }
    } while (isTuiRunning && strcmp(mensagem, "exit") != 0);

    isTuiRunning = false;
}

void receiveMessages()
{
    std::this_thread::sleep_for(1s);
    while (isTuiRunning)
    {
        try
        {
            SocketBuffer data = serverSocket.Read();
            printf("<< Mensagem recebida: \"%s\"\n", (char *)data.buf);
        }
        catch (ConnectionClosedException &e)
        {
            std::cout << "Server offline." << std::endl;
            isTuiRunning = false;
            fclose(stdin);
            return;
        }
    }
}

void handleSocketDestruction(int sig)
{
    std::cout << "To vindo com " << sig << std::endl;

    serverSocket.Close();

    // std::cout << "Cliente encerrado abruptamente (" << sig << ") received.\n";
    isTuiRunning = false;
}

int main(int argc, char const *argv[])
{
    tui::clear();
    tui::printl("Inicializando Zaplan v0.1 - Cliente"_fgre);

    const std::string defaultServerIP("127.0.0.1");
    const std::string *serverIp = &defaultServerIP;
    tui::print("Digite o IP do servidor com o qual deseja se conectar (Enter para localhost): ");
    std::string input = tui::readline();
    if (!input.empty())
        serverIp = &input;

    IPADDR4 serverAddress{*serverIp, 4545};
    Socket serverSocket(SocketType::TCP);

    try
    {
        serverSocket.Connect(serverAddress);
    }
    catch (ConnectionFailedException &e)
    {
        tui::print(tui::text::Text("Unable to connect to " + serverAddress.ToString() + ". \nReason:\t ").FRed());
        std::cerr << e.what() << std::endl;
        return -1;
    }

    tui::print("Digite seu " + "nick"_fwhi + ": ");
    std::string nick = tui::readline();

    try
    {
        std::string bufContet = "nick=" + nick;
        SocketBuffer data{bufContet.c_str(), bufContet.length() + 1};
        serverSocket.Send(data);
    }
    catch (ConnectionFailedException &e)
    {
        std::cerr << "Unable to send username to server"
                  << ".\nReason:\n\t ";
        std::cerr << e.what() << std::endl;
        throw e;
    }

    Client client(serverSocket, nick);

    std::thread receiveServerMessages(receiveMessages);
    auto clientTUI = tui::ClientTUI(client);
    clientTUI.Enter();

    tui::printl("Encerrando Zaplan v0.1 - Cliente"_fyel);

    close(serverSocket.GetFD());
    receiveServerMessages.join();

    tui::printl("Todas as threads foram encerradas com sucesso"_fyel);
    return 0;
}

// int main(int argc, char const *argv[])
// {

//     tui::printl("Contatos online: " + "10"_fwhi.Bold());
//     tui::printl();
//     tui::printl();

//     tui::printl("Digite " + "/help"_fcya + " para obter uma lista de comandos disponíveis");

//     tui::printl();
//     tui::printl();

//     tui::print("> "_fgre);
//     tui::text::Text test = tui::readline();

//     tui::printl();
//     tui::printl("[ERRO]"_bmag.WithColor(tui::text::TextColorF::White).Bold() + " O programa ainda não faz nada!!"_fred);
//     return 0;
// }
