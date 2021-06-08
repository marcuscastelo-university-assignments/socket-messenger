#pragma once

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
