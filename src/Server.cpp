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

}

/*void	Server::handleClientsLine(const std::string &line, Client &client)
{
	std::string	sender = client.getNickname();
	if (sender.empty())
		sender = ":" + std::to_string(client.getFd());

	Message msg(line);
	msg.setSender(sender);

	if (msg.parseBuffer() == 0)
		msg.callCommand();
	else
		std::cerr << "Invalid command: " << line << std::endl;
}*/


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

		Client	&client = this->clients[this->pollFds[i].fd];
		client.appendBuffer(buffer);

		std::string	&buf = client.getBuffer();
		size_t		pos;

		while((pos = buf.find("\r\n")) != std::string::npos)
		{
			std::string	line = buf.substr(0, pos);
			buf.erase(0, pos + 2);

			std::cout << "Parsed line: " << line << std::endl;

			//this->handleClientsLine(line, client);
			this->executeCommand(line, client);
		}
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

void	Server::executeCommand(const std::string &buffer, Client &client)
{
	Message message_received = Message(buffer);

	std::string sender = client.getNickname();
	if (sender.empty()) {
		sender = ":" + std::to_string(client.getFd());
	}
	message_received.setSender(sender);

	if (message_received.parseBuffer() == -1)
		return;
	auto it = _command_map.find(message_received.getCommand());
    if (it != _command_map.end()) {
        (this->*(it->second))(message_received);
    } else {
        std::cerr << "Unknown command: " << message_received.getSender() << std::endl << std::endl;
    }
}

/*void	Server::executeCommand(std::string buffer) {
	Message message_received = Message(buffer);
	
	if (message_received.parseBuffer() == -1)
		return;
	auto it = _command_map.find(message_received.getCommand());
    if (it != _command_map.end()) {
        (this->*(it->second))(message_received);
    } else {
        std::cerr << "Unknown command: " << message_received.getSender() << std::endl << std::endl;
    }
}*/

void Server::pass(Message & message) {
	// Numeric Replies:

	// ERR_NEEDMOREPARAMS              ERR_ALREADYREGISTRED
	std::cout << "PASS command by " << message.getSender() << std::endl;
};

void Server::nick(Message & message) {
	// Numeric Replies:

    //        ERR_NONICKNAMEGIVEN             ERR_ERRONEUSNICKNAME
    //        ERR_NICKNAMEINUSE               ERR_NICKCOLLISION
    //        ERR_UNAVAILRESOURCE             ERR_RESTRICTED
	std::cout << "NICK command by " << message.getSender() << std::endl;
};

void Server::user(Message & message) {
	// Numeric Replies:

    //        ERR_NEEDMOREPARAMS              ERR_ALREADYREGISTRED
	std::cout << "USER command by " << message.getSender() << std::endl;
};

void Server::join(Message & message) {
	// Numeric Replies:

    //        ERR_NEEDMOREPARAMS              ERR_BANNEDFROMCHAN
    //        ERR_INVITEONLYCHAN              ERR_BADCHANNELKEY
    //        ERR_CHANNELISFULL               ERR_BADCHANMASK
    //        ERR_NOSUCHCHANNEL               ERR_TOOMANYCHANNELS
    //        ERR_TOOMANYTARGETS              ERR_UNAVAILRESOURCE
    //        RPL_TOPIC
	std::cout << "JOIN command by " << message.getSender() << std::endl;
};

void Server::part(Message & message) {
	// Numeric Replies:

    //        ERR_NEEDMOREPARAMS              ERR_NOSUCHCHANNEL
    //        ERR_NOTONCHANNEL
	std::cout << "PART command by " << message.getSender() << std::endl;
};

void Server::topic(Message & message) {
	// Numeric Replies:

	// ERR_NEEDMOREPARAMS              ERR_NOTONCHANNEL
	// RPL_NOTOPIC                     RPL_TOPIC
	// ERR_CHANOPRIVSNEEDED            ERR_NOCHANMODES
	std::cout << "TOPIC command by " << message.getSender() << std::endl;
};

void Server::invite(Message & message) {
	// Numeric Replies:

    //        ERR_NEEDMOREPARAMS              ERR_NOSUCHNICK
    //        ERR_NOTONCHANNEL                ERR_USERONCHANNEL
    //        ERR_CHANOPRIVSNEEDED
    //        RPL_INVITING                    RPL_AWAY
	std::cout << "INVITE command by " << message.getSender() << std::endl;
};

void Server::kick(Message & message) {
	// Numeric Replies:

    //        ERR_NEEDMOREPARAMS              ERR_NOSUCHCHANNEL
    //        ERR_BADCHANMASK                 ERR_CHANOPRIVSNEEDED
    //        ERR_USERNOTINCHANNEL            ERR_NOTONCHANNEL
	std::cout << "KICK command by " << message.getSender() << std::endl;
};

void Server::quit(Message & message) {
	std::cout << "QUIT command by " << message.getSender() << std::endl;
};

void Server::mode(Message & message) {
	// Numeric Replies:

    //        ERR_NEEDMOREPARAMS              ERR_USERSDONTMATCH
    //        ERR_UMODEUNKNOWNFLAG            RPL_UMODEIS
	std::cout << "MODE command by " << message.getSender() << std::endl;
};

void Server::privmsg(Message & message) {
	// Numeric Replies:

    //        ERR_NORECIPIENT                 ERR_NOTEXTTOSEND
    //        ERR_CANNOTSENDTOCHAN            ERR_NOTOPLEVEL
    //        ERR_WILDTOPLEVEL                ERR_TOOMANYTARGETS
    //        ERR_NOSUCHNICK
    //        RPL_AWAY
	std::cout << "PRIVMSG command by " << message.getSender() << std::endl;
};

void	sendToClient(int fd, const std::string &msg)
{
	send(fd, msg.c_str(), msg.length(), 0);
}
