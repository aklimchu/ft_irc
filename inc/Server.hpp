#pragma once

#include <iostream>
#include <poll.h>
#include <sys/socket.h>
#include <cstring> // for memset
#include <netinet/in.h> // for sockaddr_in, htons
// #include <errno.h>
#include <unistd.h> // for close

#define BACKLOG 10
#define PORT 8888

class Server {
	public:
		Server(void) = default;
		Server(Server & src) = delete;

		~Server(void) = default;

		Server & operator=(Server & rhs) = delete;

		void initServer(void);
		void startServer(void);

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
};