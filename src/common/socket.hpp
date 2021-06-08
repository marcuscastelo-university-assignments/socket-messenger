/**
 * Header criado para organizar todas as funções do Socket
*/
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
    DEF_WHAT("Error while binding server.")
};

struct SocketAcceptException : public std::exception
{
    DEF_WHAT("Accept failed.")
};

//Estrutura auxiliar para armazenar o IP e a porta utilizadas
struct IPADDR4
{
    std::string ip;
    int port;
    std::string ToString() const;
};

//Estrutura auxiliar para armazenar as informações do Socket
struct SocketBuffer
{
    char *buf;
    size_t len;
    SocketBuffer(const void *_buf, size_t _len);
    SocketBuffer(const SocketBuffer &other);

    ~SocketBuffer();
};

//Estrutura auxiliar que armazena os dados da informação contida no socket
struct Message
{
    std::string FromUser, ToUser, Content;
    Message(const std::string &fromUser, const std::string &toUser, const std::string &content);
    Message(const SocketBuffer &buf);
    Message(const std::string &fromUser, const SocketBuffer &buf);
    SocketBuffer ToBuffer() const;
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
enum SocketType : int
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
    inline const IPADDR4 &GetAddress() const { return m_Address; }
    inline const int GetFD() const { return m_SocketFD; }
    inline const SocketType GetType() const { return m_Type; }

    //Construtor da classe Socket, que retorna um socket do tipo passado como argumento
    Socket(SocketType type);

    //Construtor do tipo copy-constructor, permite clonar um Socket (só clona o objeto, o FD (File Descriptor) é o mesmo)
    Socket(const Socket& other) = default;
    Socket(const Socket&& other) {
        m_SocketFD = other.m_SocketFD;
        m_Type = other.m_Type;
        m_Address = other.m_Address;
        m_NativeAddress = other.m_NativeAddress;
        m_AcceptedSockets = std::move(other.m_AcceptedSockets);
    }
    
    //TODO: comentar (usado no UserSockets)
    Socket &operator=(const Socket& other) {
        m_SocketFD = other.m_SocketFD;
        m_Type = other.m_Type;
        m_Address = other.m_Address;
        m_NativeAddress = other.m_NativeAddress;
        m_AcceptedSockets = other.m_AcceptedSockets;
        return *this;
    }

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
    void Shutdown() const;

    /**
     * Função responsável por aceitar uma nova conexão de socket
     *
     * Parâmetros: void
     *
     * Retorno: Socket => Socket aceito e adicionado ao vetor de conexões
    */
    Socket Accept();

    /**
     * Função que lê o pacote de dados recebido por meio do socket atual
     *
     * Parâmetros: int bufferMaxSize            =>  Inteiro que contém o tamanho máximo do buffer da mensagem
     *
     * Retorno: SocketData  =>  Informações adquiridas pelo socket cliente
    */
    SocketBuffer Read(int bufferMaxSize = 1024) const;

    /**
     * Função que envia um pacote de dados por meio do socket atual
     *
     * Parâmetros: const SocketBuffer &data       =>  Informações tratadas pelo socket a serem transmitidas
     *
     * Retorno: void
    */
    void Send(const SocketBuffer &data) const;


    //Operador funcional para comparação entre dois sockets
    bool operator==(const Socket& other) const;
};

template <>
struct std::hash<Socket>
{
    std::size_t operator()(const Socket &sock) const
    {
        return (
            std::hash<int>()(sock.GetFD()) ^
            std::hash<int>()(sock.GetType()) ^
            std::hash<std::string>()(sock.GetAddress().ip) ^
            std::hash<int>()(sock.GetAddress().port));
    }
};