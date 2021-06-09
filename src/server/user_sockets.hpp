/**
 * Header criado para facilitar manuseio dos clientes dentro do servidor (armazenar nomes e sockets enquanto mapeia)
*/
#pragma once

#include <unordered_map>
#include <string>
#include "socket.hpp"

#include <mutex>
#include <memory>

using guard = std::lock_guard<std::mutex>;

struct User
{
	SocketRef m_Socket;
	std::string m_Nick;

	User() {}
	User(const User &other) : m_Socket(other.m_Socket), m_Nick(other.m_Nick) {}
	User(User &&moveUser) : m_Socket(std::move(moveUser.m_Socket)), m_Nick(std::move(moveUser.m_Nick)) {}
	User &operator=(const User& other) {
		m_Socket = other.m_Socket;
		m_Nick = other.m_Nick;
		return *this;
	}

	bool operator==(const User& other) const {
		return other.m_Nick == m_Nick && other.m_Socket.get() == m_Socket.get();
	}
};

namespace std
{
	template <>
	struct hash<struct User>
	{
		size_t operator()(const User &user) const
		{
			return std::hash<Socket>()(*user.m_Socket.get()) ^ std::hash<std::string>()(user.m_Nick);
		}
	};
}

using SocketFD = int;

/**
	 * Estrutura criada para facilitar gerenciamento dos clientes
	 * conectados em tempo de execução. Nela são contidas dois mapas
	 * não ordenados que armazenam os clientes e seus sockets respectivos,
	 * facilitando a organização e o fluxo de dados
	 * */

class UserSockets
{
	std::vector<User> m_Users;

	//Mapeia nome do cliente pelo Socket
	std::unordered_map<std::string, User> m_NickToUser = {};
	//Mapeia socket pelo nome do cliente
	std::unordered_map<SocketFD, User> m_SocketToUser = {};

	mutable std::mutex m_Mutex;

public:
	/**
	 * Função que registra um usuário nos mapas
	 * 
	 * Parâmetros:	const User& user => usuário a ser registrado
	 * 
	 * Retorno: void
	*/
	void RegisterUser(const User &user);

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

	User *FindByNick(const std::string &nick);
	User *FindBySocket(const Socket &socket);

	//FIXME: não está sendo lockada pelo mutex
	inline const std::vector<User> &ListUsers() { return m_Users; }

	inline void UnregisterAllUsers()
	{
		guard g_(m_Mutex);
		m_NickToUser.clear();
		m_SocketToUser.clear();
		m_Users.clear();
	}
};