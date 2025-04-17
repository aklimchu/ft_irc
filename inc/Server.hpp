#pragma once

#include <iostream>
#include <poll.h>
#include <sys/socket.h>
#include <cstring> // for memset
#include <netinet/in.h> // for sockaddr_in, htons
#include <unistd.h> // for close
#include <signal.h>

#include <vector>
#include <fcntl.h>
#include <map>
#include <unordered_map>
#include "Client.hpp"
#include "Message.hpp"

#define BACKLOG 10
#define SERVER_NAME "ircserv"

class Server;

using CommandFunction = void(Server::*)(Message&, Client &);

class Server {
	public:
		Server(void) = delete;
		Server(std::string passwd);
		Server(Server & src) = delete;

		~Server(void) = default;

		Server & operator=(Server & rhs) = delete;

		void initServer(char *argv[]);
		void startServer(void);
		void closeFds(void);
		static void handleSignals(int num);
		void	setNonBlock(int fd);

		//void	executeCommand(std::string buffer);
		void	executeCommand(const std::string &buffer, Client &client);
		void	handleNewClient();
		void	handleOldClient(size_t &i);
		void	sendToClient(int fd, const std::string &msg);
		void	welcomeMessages(Client &client);
		//void	handleClientsLine(const std::string &line, Client &client);
		std::map<int, Client> getClients(void) const;

		void	pass(Message & message, Client &client);
		void	nick(Message & message, Client &client);
		void	user(Message & message, Client &client);
		void	join(Message & message, Client &client);
		void	part(Message & message, Client &client);
		void	topic(Message & message, Client &client);
		void	invite(Message & message, Client &client);
		void	kick(Message & message, Client &client);
		void	quit(Message & message, Client &client);
		void	mode(Message & message, Client &client);
		void	privmsg(Message & message, Client &client);
		//do we need oper function?

		class SocketError : public std::exception {
			public:
				virtual const char* what() const throw() {
					return ("Error while creating a socket");
				}
		};

		class BindingError : public std::exception {
			public:
				virtual const char* what() const throw() {
					return ("Error in binding");
				}
		};

		class ListeningError : public std::exception {
			public:
				virtual const char* what() const throw() {
					return ("Error in listening");
				}
		};

		class BadPassword : public std::exception {
			public:
				virtual const char* what() const throw() {
					return ("ERROR :Closing Link: localhost (Bad Password)");
				}
		};

	private:
		int					_sockfd;
		static bool			_signal_received;
		const std::string	_server_passwd;

		std::vector<pollfd>		_pollFds;
		std::map<int, Client>	_clients;

		const std::unordered_map<std::string, CommandFunction> _command_map = {
			{"PASS", &Server::pass},
			{"NICK", &Server::nick},
			{"USER", &Server::user},
			{"JOIN", &Server::join},
			{"PART", &Server::part},
			{"TOPIC", &Server::topic},
			{"INVITE", &Server::invite},
			{"KICK", &Server::kick},
			{"QUIT", &Server::quit},
			{"MODE", &Server::mode},
			{"PRIVMSG", &Server::privmsg}
		};
};
