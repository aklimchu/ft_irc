#include "Server.hpp"

int Server::initServer(void) {
	int sockfd;
	struct sockaddr_in sockaddr;

	if (((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)) {
		std::cout << "Error while creating the socket" << std::endl;
		return(1);
	}

	std::memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(8888);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (((bind(sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr))) < 0)) {
		std::cout << "Error in binding" << std::endl;
		return(1);
	}

	if ((listen(sockfd, 10)) < 0) {
		std::cout << "Error in listening" << std::endl;
		return(1);
	}

	while (1)  {
		//accept or poll
	}

	close(sockfd);

	return 0;
}

int Server::startServer(void) {
	return 0;
}

int main (int argc, char *argv[]) {
	/* if (argc < 3 || argc > 3)
		std::cout << "Wrong number of arguments" << std::endl; */
	(void)argv;
	(void)argc;
	if (Server::initServer() == 1) {
		return 1;
	};
	if (Server::startServer() == 1) {
		return 1;
	};
}