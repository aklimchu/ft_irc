#pragma once

#include <iostream>
#include <poll.h>
#include <sys/socket.h>
#include <cstring> // for memset
#include <netinet/in.h> // for sockaddr_in, htons
// #include <errno.h>
#include <unistd.h> // for close

class Server {
	public:
		Server(void) = delete;
		Server(Server & src) = delete;

		~Server(void) = default;

		Server & operator=(Server & rhs) = delete;

		static int initServer(void);
		static int startServer(void);

	private:
};