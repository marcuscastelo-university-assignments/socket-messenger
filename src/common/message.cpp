#include "message.hpp"

#include <string.h>
#include <iostream>
#include <sstream>

Message::Message(const std::string &fromUser, const std::string &toUser, const std::string &content)
    : FromUser(fromUser), ToUser(toUser), Content(content) {}

Message::Message(const SocketBuffer &sd)
{
    char *buf = strdup(sd.buf);

    size_t i = 0;
    char *commandPart = buf;

    char *parts[3] = {buf, buf, buf};
    int partIdx = 0;

    for (; i < sd.len; i++)
    {
        if (buf[i] == '=')
        {
            buf[i] = '\0';
            parts[partIdx++] = buf + i + 1;
            break;
        }
    }

    for (; i < sd.len && partIdx < 3; i++)
    {
        if (sd.buf[i] == ':')
        {
            buf[i] = '\0';
            parts[partIdx++] = buf + i + 1;
        }
    }

    if (partIdx < 3)
        std::cerr << "Malformed buffer (unknown reason)" << std::endl;

    std::string command(commandPart);
    FromUser = parts[0];
    ToUser = parts[1];
    Content = parts[2];
    free(buf);
}

Message::Message(const std::string &fromUser, const SocketBuffer &sd) : Message::Message(sd)
{
    
    if (FromUser != fromUser)
    {
        std::cerr << "Impersonation detected!" << std::endl;
        ToUser = FromUser;
        FromUser = "System";
        Content = "Nice try!";
    }
}

SocketBuffer Message::ToBuffer() const
{
    std::stringstream ss;
    ss << "msg=" << FromUser << ":" << ToUser << ":" << Content;
    auto bufstr = ss.str();
    char *buf = strdup(bufstr.c_str());
    SocketBuffer sb = {
        buf,
        bufstr.length() + 1};

    return sb;
}

SocketBuffer::SocketBuffer(const void *_buf, size_t _len) : len(_len)
{
    buf = (char *)malloc(len);
    mempcpy(buf, _buf, len);
}

SocketBuffer::SocketBuffer(const SocketBuffer &other) : SocketBuffer(other.buf, other.len) {}

SocketBuffer::~SocketBuffer()
{
    free(buf);
}