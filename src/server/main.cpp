#include "socket.hpp"
#include "server.hpp"
#include "tui.hpp"
using namespace tui::text_literals;

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

    IPADDR4 serverIP{"0.0.0.0", 4545};
    Server server(serverIP, 10);

    server.Start();
    server.EnterTUI();

    tui::printl("Inicializando o Zaplan Server..."_fgre);
    tui::down(2);
    
    
    // endServer(server);

    tui::printl("Todas as threads foram encerradas com sucesso!"_fblu);
    return 0;
}
