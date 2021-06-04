#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <vector>

struct IPADDR4 {
    std::vector<int> ipParts;
    int port;
};


sockaddr_in makeAddr(std::vector<int> ipParts, int port)
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;

    //TODO: fix
    addr.sin_addr.s_addr = INADDR_ANY;

    addr.sin_port = htons(port);
    memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));

    return addr;
}

class User
{
};

struct SocketData
{
    void *buf;
    size_t len;
    SocketData(const void *_buf, size_t _len) : len(_len) {
        buf = malloc(len);
        mempcpy(buf, _buf, len); 
    }

    SocketData(const SocketData& other) : SocketData(other.buf, other.len) {}

    ~SocketData() {
        free(buf);
    }
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

    IPADDR4 address;

public:
    Socket(SocketType type)
    {
        m_SocketFD = socket(AF_INET, type, 0);
        if (m_SocketFD == -1)
            throw 11;
        m_Type = type;
    }

    Socket(int socketFD)
    {
        m_SocketFD = socketFD;

        SocketType option_value;
        socklen_t option_len;
        getsockopt(m_SocketFD, SOL_SOCKET, SO_TYPE, &option_value, &option_len);

        m_Type = option_value;
    }

    void Bind(IPADDR4 addr)
    {
        //FIXME: ipParts
        sockaddr_in m_Address = makeAddr(addr.ipParts, addr.port);

        int errorCode = bind(m_SocketFD, (sockaddr *)&m_Address, sizeof(m_Address));
        if (errorCode == -1)
            throw 12;
    }

    void Connect(IPADDR4 addr) {
        sockaddr_in m_Address = makeAddr(addr.ipParts, addr.port);

        int errorCode = connect(m_SocketFD, (sockaddr *)&m_Address, sizeof(m_Address));
        if (errorCode == -1)
            throw 17;
    }

    void Listen(int maxConnections)
    {
        int errorCode = listen(m_SocketFD, maxConnections);
        if (errorCode == -1)
            throw 13;
    }

    Socket Accept()
    {
        int sockClientFD = accept(m_SocketFD, 0, 0);
        if (sockClientFD == -1)
            throw 14;

        return Socket(sockClientFD);
    }
    
  
    static SocketData Read(const Socket& clientSocket, int bufferMaxSize = 1024)
    {
        size_t readCount = 0;
        char readBuf[bufferMaxSize];

        readCount = recv(clientSocket.getFD(), readBuf, bufferMaxSize, 0); /* Recebe mensagem do cliente */
        readBuf[readCount] = '\0';

        return (SocketData) {
            readBuf,
            readCount,
        };
    }

    static void Send(const Socket &destination, const SocketData& data)
    {
        ssize_t sentByteCount = send(destination.getFD(), data.buf, data.len, 0);

        if (sentByteCount == -1) {
            throw 15;
        }

        if ((size_t) sentByteCount != data.len) {
            //TODO: verificar se isso é uma preoucupação
            throw 16;
        }
    }

    inline const int getFD() const { return m_SocketFD; }
};

struct MessageData : public SocketData
{
    MessageData(const std::string &text) : SocketData(text.c_str(), text.length()) {}
    MessageData(const SocketData &sd) : SocketData(sd.buf, sd.len) {}
};

struct Message
{
    MessageData Data;
    const Socket &Dest;
    Message(MessageData data, Socket dest) : Data(data), Dest(dest) {}
};