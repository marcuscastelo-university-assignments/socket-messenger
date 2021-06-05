//ifndef para o header socket.hpp
#pragma once

//Includes utlizados para os sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <exception>

//Define para facilitar print das mensagens de exception
#define DEF_WHAT(msg) \
    const char *what() const throw() { return #msg; }

//Exceção caso a conexão falhe
struct ConnectionFailedException : public std::exception
{
    DEF_WHAT("Connection Failed")
};

//Exceção caso a conexão seja encerrada abruptamente
struct ConnectionClosedException : public std::exception
{
    DEF_WHAT("Connection Ended.")
};

//Exceção caso o Socket a ser utilizado esteja indisponível
struct SocketBindException : public std::exception
{
    DEF_WHAT("Error on binding server.")
};

//Estrutura auxiliar para armazenar o IP e a porta utilizadas
struct IPADDR4
{
    std::string ip;
    int port;
    std::string ToString() const;
};

//Estrutura auxiliar para armazenar as informações do Socket
struct SocketData
{
    char *buf;
    size_t len;
    SocketData(const void *_buf, size_t _len);
    SocketData(const SocketData &other);

    ~SocketData();
};

//Estrutura auxiliar que armazena os dados da informação contida no socket
struct MessageData : public SocketData
{
    MessageData(const std::string &text);
    MessageData(const SocketData &sd);
};

//Operador que auxilia no cout(impressão) do IP
std::ostream &operator<<(std::ostream &o, const IPADDR4 &ip);

/**
 * Assinatura do método que retorna uma estrutura que armazena informações do socket(endereço e porta)
 *
 * Parâmetros: std::vector<int> ipParts => Vetor de ints que contém o IP
               int                      => Inteiro que armazena a porta
 *
 * Return: sockaddr_in => estrutura com ip e a porta
 */
sockaddr_in makeAddr(std::vector<int> ipParts, int port);

class User
{
};

//Enum que define se o protocolo é TCP ou UDP
enum SocketType
{
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM
};

// Classe do Socket, contendo o seu endereço, sockets aceitos, tipo e seus dados.
class Socket
{
    int m_SocketFD;
    SocketType m_Type;
    IPADDR4 m_Address;
    
    sockaddr_in m_NativeAddress;

    std::vector<int> m_AcceptedSockets = {};


public:
    //Funções que retornam o endereço e o socket utilizado
    const inline IPADDR4 &GetAddress() { return m_Address; }
    inline const int getFD() const { return m_SocketFD; }

    //Construtor da classe Socket, que retorna um socket do tipo passado como argumento
    Socket(SocketType type);

    //Construtor da classe Socket, que retorna um socket com o valor passado
    Socket(int socketFD, SocketType type, IPADDR4 address);

    /**
     * Função responsável pelo bind do Socket com uma porta e um IP
     *
     * Parâmetros: IPADDR4 addr => IP e a porta a serem reservados pelo socket
     *
     * Retorno: void
    */
    void Bind(IPADDR4 addr);

    /**
     * Função responsável pela conexão de um socket a uma porta
     *
     * Parâmetros: IPADDR4 addr => IP e a porta a serem conectados ao socket
     *
     * Retorno: void
    */
    void Connect(IPADDR4 addr);

    /**
     * Função responsável por ouvir uma porta
     *
     * Parâmetros: IPADDR4 addr => IP e a porta a serem reservados pelo socket
     *
     * Retorno: void
    */
    void Listen(int maxConnections);

    /**
     * Função responsável por fechar e encerrar o socket e as demais conexões ligadas a ele
     *
     * Parâmetros: void
     *
     * Retorno: void
    */
    void Close();

    /**
     * Função responsável por aceitar uma nova conexão de socket
     *
     * Parâmetros: void
     *
     * Retorno: Socket => Socket aceito e adicionado ao vetor de conexões
    */
    Socket Accept();

    /**
     * Função estática que lê o pacote de dados recebido por um dado socket cliente
     *
     * Parâmetros: const Socket &clientSocket   =>  Socket do cliente a ser lido as informações
                   int bufferMaxSize            =>  Inteiro que contém o tamanho máximo do buffer da mensagem
     *
     * Retorno: SocketData  =>  Informações adquiridas pelo socket cliente
    */
    static SocketData Read(const Socket &clientSocket, int bufferMaxSize = 1024);

    /**
     * Função estática que envia um pacote de dados para um socket destino
     *
     * Parâmetros: const Socket &destination    =>  Socket destino
                   const SocketData &data       =>  Informações tratadas pelo socket a serem transmitidas
     *
     * Retorno: void
    */
    static void Send(const Socket &destination, const SocketData &data);
};

//Estrutura auxiliar de mensagem, que armazena as informações e o destino
struct Message
{
    MessageData Data;
    const Socket &Dest;
    Message(MessageData data, Socket dest);
};
