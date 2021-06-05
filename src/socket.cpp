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

SocketData::SocketData(const void *_buf, size_t _len) : len(_len)
{
    buf = (char *)malloc(len);
    mempcpy(buf, _buf, len);
}

SocketData::SocketData(const SocketData &other) : SocketData(other.buf, other.len) {}

SocketData::~SocketData()
{
    free(buf);
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
    m_NativeAddress = makeAddr(addr.ip, addr.port);;
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

void Socket::Close()
{
    int true_ = 1;

    for (int acceptedFD : m_AcceptedSockets)
    {
        setsockopt(acceptedFD, SOL_SOCKET, SO_REUSEADDR, &true_, sizeof(int));
        shutdown(acceptedFD, SHUT_RDWR);
        close(acceptedFD);
    }

    setsockopt(m_SocketFD, SOL_SOCKET, SO_REUSEADDR, &true_, sizeof(int));
    shutdown(m_SocketFD, SHUT_RDWR);
    close(m_SocketFD);
}

Socket Socket::Accept()
{
    // int sockClientFD = accept(m_SocketFD, 0, 0);
    struct sockaddr_in remoteaddr;
    socklen_t remoteaddr_len = sizeof(remoteaddr);
    int sockClientFD = accept(m_SocketFD, (sockaddr*)&remoteaddr, &remoteaddr_len);
    if (sockClientFD == -1) {
        throw std::runtime_error("Error on Socket::Accept! errno = " + errno);
    }
        
    IPADDR4 clientAddress{inet_ntoa(remoteaddr.sin_addr), ntohs(remoteaddr.sin_port)};

    m_AcceptedSockets.push_back(sockClientFD);

    return Socket(sockClientFD, m_Type, clientAddress);
}

//Static funcs

SocketData Socket::Read(const Socket &clientSocket, int bufferMaxSize)
{
    size_t readCount = 0;
    char readBuf[bufferMaxSize];

    readCount = recv(clientSocket.getFD(), readBuf, bufferMaxSize, 0); /* Recebe mensagem do cliente */

    if (readCount == 0)
    {
        throw ConnectionClosedException();
    }

    if (readCount == -1)
    {
        throw std::runtime_error("Unexpect error, readCount == -1");
    }

    readBuf[readCount] = '\0';

    return (SocketData){
        readBuf,
        readCount,
    };
}

void Socket::Send(const Socket &destination, const SocketData &data)
{
    ssize_t sentByteCount = send(destination.getFD(), data.buf, data.len, 0);

    if (sentByteCount == -1)
    {
        throw ConnectionClosedException();
    }

    if ((size_t)sentByteCount != data.len)
    {
        //TODO: verificar se isso é uma preoucupação
        throw std::logic_error("sent != data.len");
    }
}

MessageData::MessageData(const std::string &text) : SocketData(text.c_str(), text.length()) {}
MessageData::MessageData(const SocketData &sd) : SocketData(sd.buf, sd.len) {}

Message::Message(MessageData data, Socket dest) : Data(data), Dest(dest) {}