#if defined(WIN32) //Winows

#error Windows is not Supported

#else //Unix

//Usado para fechamento de sockets
#include <unistd.h>

#endif

//Usando operadores ms, ns, etc..
#include <chrono>
using namespace std::chrono_literals;

#include <thread>
#include <random>

#include "socket.hpp"
#include "client_tui.hpp"
#include "tui.hpp"

using namespace tui::text_literals;

int main(int argc, char const *argv[])
{
    std::srand(time(NULL));
    tui::clear();
    tui::printl("Inicializando Zaplan v0.1 - Cliente"_fgre);

    //Caso nenhum ip de servidor seja fornecido
    const std::string defaultServerIP("127.0.0.1");
    const std::string *serverIp = &defaultServerIP;
    tui::print("Digite o IP do servidor com o qual deseja se conectar (Enter para localhost): ");
    std::string input = tui::readline();
    if (!input.empty())
        serverIp = &input;

    //Configura o endereço do servidor para a porta 4545
    IPADDR4 serverAddress{*serverIp, 4545};

    tui::print("Digite seu " + "nick"_fwhi + ": ");
    std::string nick = tui::readline();
    
    Client client;
    try {
        client.ConnectAndLogin(serverAddress, nick);
    } catch(ConnectionFailedException &e) {
        return -1;
    }
    
    client.Start();

    tui::printl("Zaplan (Cliente) v0.1 encerrado com sucesso."_fyel);
    return 0;
}