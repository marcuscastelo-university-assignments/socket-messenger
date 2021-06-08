#include "socket.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <errno.h>

#include "socket_exceptions.hpp"

std::string IPADDR4::ToString() const
{
    std::stringstream ss;
    ss << ip << ":" << port;
    return ss.str();
};

std::ostream &operator<<(std::ostream &o, const IPADDR4 &ip)
{
    return o << ip.ToString();
}

sockaddr_in makeAddr(std::string ip, int port)
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));

    return addr;
}


Socket::Socket(SocketType type)
{
    m_SocketFD = socket(AF_INET, type, 0);
    if (m_SocketFD == -1)
        throw 11;
    m_Type = type;
}

Socket::Socket(int socketFD, SocketType type, IPADDR4 address)
{
    m_SocketFD = socketFD;
    m_Type = type;
    m_Address = address;
}

void Socket::Bind(IPADDR4 addr)
{
    //FIXME: ipParts
    m_NativeAddress = makeAddr(addr.ip, addr.port);

    m_Address = addr;

    int errorCode = bind(m_SocketFD, (sockaddr *)&m_NativeAddress, sizeof(m_NativeAddress));
    if (errorCode == -1)
        throw SocketBindException();
}

void Socket::Connect(IPADDR4 addr)
{
    m_NativeAddress = makeAddr(addr.ip, addr.port);
    m_Address = addr;

    int errorCode = connect(m_SocketFD, (sockaddr *)&m_NativeAddress, sizeof(m_NativeAddress));
    if (errorCode == -1)
        throw ConnectionFailedException();
}

void Socket::Listen(int maxConnections)
{
    int errorCode = listen(m_SocketFD, maxConnections);
    if (errorCode == -1)
        throw 13;
}

void Socket::Shutdown() const
{
    int true_ = 1;

    //     for (int acceptedFD : m_AcceptedSockets)
    //     {
    //         true_ = 1;
    //         setsockopt(acceptedFD, SOL_SOCKET, SO_REUSEADDR, &true_, sizeof(int));
    // #ifdef SO_REUSEPORT
    //         setsockopt(acceptedFD, SOL_SOCKET, SO_REUSEPORT, &true_, sizeof(int));
    // #endif
    //         shutdown(acceptedFD, SHUT_RDWR);
    //     }

    //     true_ = 1;
    //     setsockopt(m_SocketFD, SOL_SOCKET, SO_REUSEADDR, &true_, sizeof(int));
    // #ifdef SO_REUSEPORT
    //     setsockopt(m_SocketFD, SOL_SOCKET, SO_REUSEPORT, &true_, sizeof(int));
    // #endif
    shutdown(m_SocketFD, SHUT_RDWR);
}

Socket Socket::Accept()
{
    struct sockaddr_in remoteaddr;
    socklen_t remoteaddr_len = sizeof(remoteaddr);
    int sockClientFD = accept(m_SocketFD, (sockaddr *)&remoteaddr, &remoteaddr_len);
    if (sockClientFD == -1)
    {
        throw SocketAcceptException();
    }

    IPADDR4 clientAddress{inet_ntoa(remoteaddr.sin_addr), ntohs(remoteaddr.sin_port)};

    m_AcceptedSockets.push_back(sockClientFD);

    return Socket(sockClientFD, m_Type, clientAddress);
}

SocketBuffer Socket::Read(int bufferMaxSize) const
{
    size_t readCount = 0;
    char readBuf[bufferMaxSize];

    readCount = recv(m_SocketFD, readBuf, bufferMaxSize, 0); /* Recebe mensagem do cliente */

    if (readCount == 0)
    {
        throw ConnectionClosedException();
    }

    readBuf[readCount] = '\0';

    return (SocketBuffer){
        readBuf,
        readCount,
    };
}

void Socket::Send(const SocketBuffer &data) const
{
    try
    {
        ssize_t sentByteCount = send(m_SocketFD, data.buf, data.len, 0);
        if (sentByteCount <= -1)
        {
            throw ConnectionClosedException();
        }

        if ((size_t)sentByteCount != data.len)
        {
            
            throw std::runtime_error("sent != data.len");
        }
    }
    catch (std::exception &e)
    {
    }
}

bool Socket::operator==(const Socket &other) const
{
    return m_Type == other.m_Type && m_SocketFD == other.m_SocketFD && m_Address.ip == other.m_Address.ip && m_Address.port == other.m_Address.port;
}
