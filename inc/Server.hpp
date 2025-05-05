#pragma once

#include <iostream>
#include <iomanip>

#include <poll.h>
#include <sys/socket.h>
#include <cstring> // for memset
#include <netinet/in.h> // for htons
#include <unistd.h> // for close
#include <signal.h>
#include <algorithm> // for std::find_if

#include <vector>
#include <fcntl.h>
#include <map>
#include <unordered_map>
#include <arpa/inet.h>
#include "Client.hpp"
#include "Message.hpp"
#include "Channel.hpp"

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

		void	executeCommand(const std::string &buffer, Client &client);
		void	handleNewClient();
		void	handleOldClient(size_t &i);
		int		sendToClient(int fd, const std::string &msg);
		void	sendToChannel(const std::string &message, Channel & channel);
		void	welcomeMessages(Client &client);
		/* const */ std::map<int, Client> &getClients(void) /* const */;
		bool	sharedChannel(const Client &a, const Client &b) const;
		std::string	getIP(int fd);
		void 	sendMessageToClient(std::vector<std::string> & args, Message & message, Client &client);
		void 	broadcastMessageToChannel(std::vector<std::string> & args, Message & message, Client &client);

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
		void	ping(Message &message, Client &client);
		void	cap(Message &message, Client &client);
		void	whois(Message &message, Client &client);
		void	who(Message &message, Client &client);

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

		std::vector<pollfd>				_pollFds;
		std::map<int, Client>			_clients;
		std::map<std::string, Channel>	_channels;

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
			{"PRIVMSG", &Server::privmsg},
			{"PING", &Server::ping},
			{"CAP", &Server::cap},
			{"WHOIS", &Server::whois},
			{"WHO", &Server::who}
		};
};
