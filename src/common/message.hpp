//Estrutura auxiliar para armazenar as informações do Socket

#include <string>

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
