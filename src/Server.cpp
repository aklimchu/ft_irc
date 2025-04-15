#include "Server.hpp"

void Server::initServer(void) {
	struct sockaddr_in sockaddr;

	if (((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0))
		throw SocketError();

	std::memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(PORT); // can take from argv
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (((bind(this->sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr))) < 0)) {
		throw BindingError();
	}

	if ((listen(this->sockfd, BACKLOG)) < 0) {
		throw ListeningError();
	}

	//while (1)  {
		//accept or poll
		//or poll always
	//}

	close(this->sockfd);
}

void Server::startServer(void) {}

int main (int argc, char *argv[]) {
	Server irc_server;
		
	/* if (argc < 3 || argc > 3)
		std::cout << "Wrong number of arguments" << std::endl; */
	(void)argv;
	(void)argc;
	try {
		irc_server.initServer();
		irc_server.startServer();
	}
	catch (std::exception & e) {
		std::cerr << e.what() << std::endl;
	}
}
