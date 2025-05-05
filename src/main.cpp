#include "Server.hpp"

int main (int argc, char *argv[]) {
	
	if (argc < 3 || argc > 3) {
		std::cout << "Wrong number of arguments (correct format: ./ircserv <port> <password>)" \
		<< std::endl;
		return -1;
	}

	Server irc_server = Server(argv[2]);

	try {
		signal(SIGINT, Server::handleSignals);
		signal(SIGQUIT, Server::handleSignals);
		irc_server.initServer(argv);
		irc_server.startServer();
	}
	catch (std::exception & e) {
		irc_server.closeFds();
		std::cerr << e.what() << std::endl;
	}
}