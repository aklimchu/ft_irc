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
	}
	// close client fds
	for(size_t i = 1; i < this->pollFds.size(); i++) //i has to start from 1(sockfd is the first element in pollFds)!
	{
		if (this->pollFds[i].fd > -1)
			close(this->pollFds[i].fd);
	}
}

void	Server::setNonBlock(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		std::cerr << "Error: setNonBlock()" << std::endl; //
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

	this->setNonBlock(this->sockfd);

	/*while (!this->signal_received)  {
		//accept and poll
	}*/

}

void	Server::handleNewClient()
{
	int	client_socket = accept(this->sockfd, NULL, NULL);

	if (client_socket < 0)
		std::cerr << "Error: accept()" << std::endl; //

	this->setNonBlock(client_socket);

	pollfd	new_client;
	new_client.fd = client_socket;
	new_client.events = POLLIN;
	this->pollFds.push_back(new_client);

	this->clients[client_socket] = Client(client_socket);

	std::cout << "New client connected" << std::endl;
}

void	Server::handleOldClient(size_t &i)
{
	char	buffer[1024];

	std::memset(buffer, 0, sizeof(buffer));
	ssize_t	bytes_read = recv(this->pollFds[i].fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_read <= 0) // Disconnet/Error
	{
		std::cout << "Client disconnected/Error happened" << std::endl;
		close(this->pollFds[i].fd);
		this->clients.erase(this->pollFds[i].fd); // Have to erase this first before below!!!
		this->pollFds.erase(this->pollFds.begin() + i);
		i--;
	}
	else // Incoming data
	{
		std::cout << "Data received:" << buffer << std::endl;
	}
}

void Server::startServer(void)
{
	pollfd	server_pollfd;

	server_pollfd.fd = this->sockfd;
	server_pollfd.events = POLLIN;
	this->pollFds.push_back(server_pollfd);

	while (!this->signal_received)
	{
		int	ready_fds = poll(this->pollFds.data(), this->pollFds.size(), -1);

		if (ready_fds < 0)
			std::cerr << "Error: poll()" << std::endl; //

		for (size_t i = 0; i < this->pollFds.size(); i++)
		{
			if (this->pollFds[i].revents & POLLIN)
			{
				if (this->pollFds[i].fd == this->sockfd)
					this->handleNewClient();
				else
					this->handleOldClient(i);
			}
		}
	}
}
