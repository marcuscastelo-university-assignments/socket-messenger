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
#include "tui.hpp"
using namespace tui::text_literals;

Socket serverSocket(SocketType::TCP);

bool isTuiRunning = false;
void startTUI()
{
    isTuiRunning = true;

    tui::clear();
    tui::printl("Bem vindo ao Zaplan"_fgre);
    tui::printl();
    tui::print("Digite seu " + "nick"_fwhi + ": ");
    tui::text::Text nick = tui::readline();

    tui::printl("Bem vindo, "_fred + nick.Bold());

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
            Socket::Send(serverSocket, {mensagem, strlen(mensagem) + 1});
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
            SocketBuffer data = Socket::Read(serverSocket);
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

    signal(SIGKILL, handleSocketDestruction);
    signal(SIGTERM, handleSocketDestruction);
    signal(SIGINT, handleSocketDestruction);
    signal(SIGQUIT, handleSocketDestruction);
    signal(SIGTSTP, handleSocketDestruction);
    signal(SIGSEGV, handleSocketDestruction);

    std::cout << "Criando um cliente!!" << std::endl;

    IPADDR4 serverAddress{"127.0.0.1", 4545};

    try
    {
        serverSocket.Connect(serverAddress);
    }
    catch (ConnectionFailedException &e)
    {
        std::cerr << "Unable to connect to " << serverAddress.ToString() << ". \nReason:\n\t ";
        std::cerr << e.what() << std::endl;
        return -1;
    }

    std::this_thread::sleep_for(1s);

    std::thread receiveServerMessages(receiveMessages);
    startTUI();

    close(serverSocket.GetFD());
    receiveServerMessages.join();

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
