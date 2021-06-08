/**
 * Header criado para gerenciar as funções do cliente
 */
#pragma once

#include <vector>
#include <thread>
#include "socket.hpp"

//Para uso do ponteiro opaco ClientTUI* (dependência cíclica)
#include "types.fwd.hpp"

//Classe cliente
class Client
{
    //Variáveis que armazenam o nickname, socket mensagens recebidas e as threads de servidor e TUI (text user interface)
    Socket m_Socket;
    std::string m_Nickname = "guest";

    bool m_Exiting = false;

    mutable std::vector<Message> m_Messages = {};

    std::thread *m_ServerSlaveThread = nullptr;
    tui::ClientTUI *m_CurrentTUI = nullptr;

    inline void CloseSockets() { m_Socket.Shutdown(); }

    /**
     * Função que continuamente lê os dados recebidos do servidor enquanto funcionando
     *
     * Parâmetros: void
     * Retorno: void
     */
    void ServerSlaveLoop();

public:
    //Define o socket para o tipo TCP
    Client() : m_Socket(SocketType::TCP) {}

    //Função auxiliar que retorna se o cliente existe
    inline bool IsExiting() const { return m_Exiting; }
    //Função auxiliar que retorna o nickname do cliente
    inline const std::string& GetNickname() const { return m_Nickname; }
    //Função auxiliar que retorna vector com as mensagens recebidas
    inline std::vector<Message> GetReceivedMessages() const { return m_Messages; }

    /**
     * Função que conecta e loga o usuario no servidor
     *
     * Parâmetros:  const IPADDR4 &serverAddr   =>  IP e porta do servidor
                    const std::string &nickname =>  Nickname do cliente a conectar
                    size_t maxTrials = 3        =>  Número máximo de tentativas de conexão

     * Retorno: void
     */
    void ConnectAndLogin(const IPADDR4 &serverAddr, const std::string &nickname, size_t maxTrials = 3);

    /**
     * Função que envia a mensagem do cliente para o servidor
     *
     * Parâmetros:  const Message &message  =>  Mensagem a ser enviada
                    size_t maxTries = 3     =>  Número máximo de tentativas de conexão

     * Retorno: void
     */
    void SendMessage(const Message &message, size_t maxTries = 3);

    /**
     * Função que inicializa o cliente para o servidor, criando sua thread e a thread da tui
     *
     * Parâmetros: void
     * Retorno: void
     */
    void Start();

    /**
     * Função que faz o pedido de saída, fechando os  sockets
     *
     * Parâmetros: void
     * Retorno: void
     */
    void RequestExit();

    /**
     * Função que encerra as threads do cliente
     *
     * Parâmetros: void
     * Retorno: void
     */
    virtual ~Client();
};
