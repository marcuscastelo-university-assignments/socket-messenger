/**
 * Header criado para facilitar manuseio dos clientes dentro do servidor (armazenar nomes e sockets enquanto mapeia)
*/
#pragma once

#include <unordered_map>
#include <string>
#include "socket.hpp"

/**
	 * Estrutura criada para facilitar gerenciamento dos clientes
	 * conectados em tempo de execução. Nela são contidas dois mapas
	 * não ordenados que armazenam os clientes e seus sockets respectivos,
	 * facilitando a organização e o fluxo de dados
	 * */

class UserSockets
{
	//Mapeia nome do cliente pelo Socket
    std::unordered_map<std::string, Socket> m_NickToSocket = {};
	//Mapeia socket pelo nome do cliente
    std::unordered_map<Socket, std::string> m_SocketToNick = {};

    inline std::unordered_map<std::string, Socket>::iterator FindByNick(const std::string &nick) { return m_NickToSocket.find(nick); }
    inline std::unordered_map<Socket, std::string>::iterator FindBySocket(const Socket &socket) { return m_SocketToNick.find(socket); }

public:
	/**
	 * Função que registra um usuário nos mapas
	 * 
	 * Parâmetros:	const std::string &nickname	=>	String com o nome do cliente
	 * 				const Socket &socket		=>	Socket do cliente
	 * 
	 * Retorno: void
	*/
    void RegisterUser(const std::string &nickname, const Socket &socket);

	/**
	 * Funções que apagam um usuário dos mapas
	 * 
	 * Parâmetros:	const std::string &nickname	=>	String com o nome do cliente
	 * 				const Socket &userSocket	=>	Socket do cliente
	 * 
	 * Retorno: void
	*/
    void UnregisterUser(const Socket &userSocket);
    void UnregisterUser(const std::string &nickname);

	/**
	 * Funções que verificam se um dado cliente (nickname ou socket) está registrado
	 * 
	 * Parâmetros:	const std::string &nickname	=>	String com o nome do cliente
	 * 				const Socket &userSocket	=>	Socket do cliente
	 * 
	 * Retorno: bool	=>	true se está registrado
	 * 						Se não, false
	*/
    bool IsUserRegistered(const Socket &userSocket);
    bool IsUserRegistered(const std::string &nickname);

	/**
	 * Funções que retornam o nome ao buscar pelo socket, ou vice versa
	 * 
	 * Parâmetros:	const std::string &nickname	=>	String com o nome do cliente
	 * 				const Socket &userSocket	=>	Socket do cliente
	 * 
	 * Retorno: const std::string	=>	Nome do usuário (busca pelo socket)
	 * 			const Socket		=>	Socket do usuário (busca pelo nome)
	*/
    const std::string &GetUserNick(const Socket &userSocket) const;
    const Socket &GetUserSocket(const std::string &nickname) const;
};