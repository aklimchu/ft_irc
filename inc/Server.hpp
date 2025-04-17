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
#define PORT 8888

class Server;

using CommandFunction = void(Server::*)(Message&);

class Server {
	public:
		Server(void);
		Server(Server & src) = delete;

		~Server(void) = default;

		Server & operator=(Server & rhs) = delete;

		void initServer(void);
		void startServer(void);
		void closeFds(void);
		static void handleSignals(int num);
		void	setNonBlock(int fd);

		//void	executeCommand(std::string buffer);
		void	executeCommand(const std::string &buffer, Client &client);
		void	handleNewClient();
		void	handleOldClient(size_t &i);
		void	sendToClient(int fd, const std::string &msg);
		//void	handleClientsLine(const std::string &line, Client &client);

		void	pass(Message & message);
		void	nick(Message & message);
		void	user(Message & message);
		void	join(Message & message);
		void	part(Message & message);
		void	topic(Message & message);
		void	invite(Message & message);
		void	kick(Message & message);
		void	quit(Message & message);
		void	mode(Message & message);
		void	privmsg(Message & message);
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

	private:
		int sockfd;
		static bool signal_received;

		std::vector<pollfd>		pollFds;
		std::map<int, Client>	clients;

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
