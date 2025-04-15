#include "Server.hpp"

bool Server::signal_received = false;

Server::Server() : sockfd(-1) {}

void Server::handleSignals(int num) {
	(void)num;
	
	Server::signal_received = true;
	std::cout << std::endl << "Signal received" << std::endl;
}

void Server::closeFds(void) {
	if (this->sockfd != -1) {
		close(this->sockfd);
	// close client fds
	}
}

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

	while (!this->signal_received)  {
		//accept and poll
	}

}

void Server::startServer(void) {}
