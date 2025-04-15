#include "Server.hpp"

int main (int argc, char *argv[]) {
	Server irc_server;
		
	/* if (argc < 3 || argc > 3)
		std::cout << "Wrong number of arguments" << std::endl; */
	(void)argv;
	(void)argc;
	try {
		signal(SIGINT, Server::handleSignals);
		signal(SIGQUIT, Server::handleSignals);
		irc_server.initServer();
		irc_server.startServer();
	}
	catch (std::exception & e) {
		irc_server.closeFds(); // do we need to close fds if no exception?
		std::cerr << e.what() << std::endl;
	}
}