#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <thread>
#include <iostream>
#include <vector>

//Using operators ms, ns, etc..
#include <chrono>
using namespace std::chrono_literals;

#include "socket.hpp"
Socket serverSocket(SocketType::TCP);


bool tui = false;
void startTUI()
{
    tui = true;
    int enviados;
    char mensagem[1024];
    do
    {
        printf(">> Digite uma mensagem: ");
		memset(mensagem, '\0', strlen(mensagem));
        fgets(mensagem, 1024, stdin);
        mensagem[strlen(mensagem) - 1] = '\0';
        Socket::Send(serverSocket, { mensagem, strlen(mensagem) });
    } while (strcmp(mensagem, "exit") != 0);

    tui = false;
}

void receiveMessages()
{
	while (tui)
    {
        SocketData data = Socket::Read(serverSocket);
        printf("<< Mensagem recebida: \"%s\"\n", (char*)data.buf);
    }
}

int main(int argc, char const *argv[])
{

    std::cout << "Criando um cliente!!" << std::endl;
    serverSocket.Connect({{127, 0, 0, 1}, 4545});

    std::this_thread::sleep_for(1s);

    Socket::Send(serverSocket, SocketData("Oi", 3));
 
 
    std::this_thread::sleep_for(100s);


	// std::thread receiveServerMessages(receiveMessages);
    // startTUI();

    return 0;
}