#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <exception>

#define DEF_WHAT(msg) \
    const char *what() const throw() { return #msg; }

struct ConnectionFailedException : public std::exception
{
    DEF_WHAT("Connection Failed")
};

struct ConnectionClosedException : public std::exception
{
    DEF_WHAT("Connection Ended.")
};

struct SocketBindException : public std::exception
{
    DEF_WHAT("Error on binding server.")
};

struct IPADDR4
{
    std::vector<int> ipParts = {-1, -1, -1, -1};
    int port;
    std::string ToString() const;
};

struct SocketData
{
    char *buf;
    size_t len;
    SocketData(const void *_buf, size_t _len);
    SocketData(const SocketData &other);

    ~SocketData();
};

struct MessageData : public SocketData
{
    MessageData(const std::string &text);
    MessageData(const SocketData &sd);
};

std::ostream &operator<<(std::ostream &o, const IPADDR4 &ip);

sockaddr_in makeAddr(std::vector<int> ipParts, int port);

class User
{
};

enum SocketType
{
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM
};

class Socket
{
    int m_SocketFD;
    int socketOpt = 0;
    SocketType m_Type;
    
    std::vector<int> acceptedSockets = {};

    IPADDR4 address;

public:
    const inline IPADDR4 &GetAddress() { return address; }
    inline const int getFD() const { return m_SocketFD; }

    Socket(SocketType type);

    Socket(int socketFD);

    void Bind(IPADDR4 addr);

    void Connect(IPADDR4 addr);

    void Listen(int maxConnections);

    void Close();

    Socket Accept();

    static SocketData Read(const Socket &clientSocket, int bufferMaxSize = 1024);

    static void Send(const Socket &destination, const SocketData &data);
};

struct Message
{
    MessageData Data;
    const Socket &Dest;
    Message(MessageData data, Socket dest);
};
