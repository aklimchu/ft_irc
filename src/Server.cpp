#include "Server.hpp"
#include "server_replies.hpp"

bool Server::_signal_received = false;

//Server::Server() : _sockfd(-1) {}

Server::Server(std::string passwd) : _sockfd(-1), _server_passwd(passwd) {};

void Server::handleSignals(int num) {
	(void)num;
	
	Server::_signal_received = true;
	std::cout << std::endl << "Signal received" << std::endl;
}

void Server::closeFds(void) {
	if (this->_sockfd != -1) {
		close(this->_sockfd);
	}
	// close client fds
	for(size_t i = 1; i < this->_pollFds.size(); i++) //i has to start from 1(_sockfd is the first element in _pollFds)!
	{
		if (this->_pollFds[i].fd > -1)
			close(this->_pollFds[i].fd);
	}
}

void	Server::setNonBlock(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		std::cerr << "Error: setNonBlock()" << std::endl; //
}

void Server::initServer(char *argv[]) {
	struct sockaddr_in sockaddr;

	if (((this->_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0))
		throw SocketError();

	std::memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(std::atoi(argv[1]));
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (((bind(this->_sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr))) < 0)) {
		throw BindingError();
	}

	if ((listen(this->_sockfd, BACKLOG)) < 0) {
		throw ListeningError();
	}

	this->setNonBlock(this->_sockfd);
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
	int	client_socket = accept(this->_sockfd, NULL, NULL);

	if (client_socket < 0)
		std::cerr << "Error: accept()" << std::endl; //

	this->setNonBlock(client_socket);

	pollfd	new_client;
	new_client.fd = client_socket;
	new_client.events = POLLIN;
	this->_pollFds.push_back(new_client);

	this->_clients[client_socket] = Client(client_socket);

	std::cout << "New client connected" << std::endl;
}

void	Server::handleOldClient(size_t &i)
{
	char	buffer[1024];

	std::memset(buffer, 0, sizeof(buffer));
	ssize_t	bytes_read = recv(this->_pollFds[i].fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_read <= 0) // Disconnet/Error
	{
		std::cout << "Client disconnected/Error happened" << std::endl;
		close(this->_pollFds[i].fd);
		this->_clients.erase(this->_pollFds[i].fd); // Have to erase this first before below!!!
		this->_pollFds.erase(this->_pollFds.begin() + i);
		i--;
	}
	else // Incoming data
	{
		std::cout << "Data received:" << buffer << std::endl;

		Client	&client = this->_clients[this->_pollFds[i].fd];
		client.appendBuffer(buffer);

		std::string	&buf = client.getBuffer();
		size_t		pos;

		while((pos = buf.find("\r\n")) != std::string::npos)
		{
			std::string	line = buf.substr(0, pos);
			buf.erase(0, pos + 2);

			std::cout << "Parsed line: " << line << std::endl;

			//this->handleClientsLine(line, client);
			try {
				this->executeCommand(line, client);
			}
			catch (std::exception &) { // Added the reference...
				throw;
			}
		}
	}
}

void Server::startServer(void)
{
	pollfd	server_pollfd;

	server_pollfd.fd = this->_sockfd;
	server_pollfd.events = POLLIN;
	this->_pollFds.push_back(server_pollfd);

	while (!this->_signal_received)
	{
		int	ready_fds = poll(this->_pollFds.data(), this->_pollFds.size(), -1);

		if (ready_fds < 0)
			std::cerr << "Error: poll()" << std::endl; //

		for (size_t i = 0; i < this->_pollFds.size(); i++)
		{
			if (this->_pollFds[i].revents & POLLIN)
			{
				if (this->_pollFds[i].fd == this->_sockfd)
					this->handleNewClient();
				else {
					try {
						this->handleOldClient(i);
					}
					catch (std::exception &) { // Added the reference...
						throw;
					}
				}
			}
		}
	}
}

void	Server::executeCommand(const std::string &buffer, Client &client)
{
	Message message_received = Message(buffer, getClients());

	std::string sender = client.getNickname();
	if (sender.empty()) {
		sender = ":" + std::to_string(client.getFd());
	}
	message_received.setSender(sender);

	if (message_received.parseBuffer() == -1)
		return;
	auto it = _command_map.find(message_received.getCommand());
    if (it != _command_map.end()) {
        try {
			(this->*(it->second))(message_received, client);
		}
		catch (std::exception &) { // Added the reference...
			throw;
		}
    } else {
        std::cerr << "Unknown command: " << message_received.getSender() << std::endl << std::endl;
    }
}

void Server::pass(Message & message, Client &client) {
	// Numeric Replies:

	// ✓ERR_NEEDMOREPARAMS              ERR_ALREADYREGISTRED
	int			fd = client.getFd();
	std::string	nick = client.getNickname().empty() ? "*" : client.getNickname();

	std::cout << "PASS command by " << message.getSender() << std::endl;

	if (client.isRegistered())
	{
		sendToClient(fd, errAlreadyRegistered(SERVER_NAME, nick));
		return ;
	}

	if (message.getBufferDivided().size() < 2)
	{
		sendToClient(client.getFd(), errNeedMoreParams(SERVER_NAME, nick, "PASS"));
		return ;
	}

	if (message.getBufferDivided()[1] == _server_passwd)
		client.setPasswdOK(true);
	else
		sendToClient(client.getFd(), errPasswdMismatch(SERVER_NAME));
};

void Server::nick(Message & message, Client &client) {
	// Numeric Replies:

    //        ERR_NONICKNAMEGIVEN             ERR_ERRONEUSNICKNAME
    //        ERR_NICKNAMEINUSE               ERR_NICKCOLLISION
    //        ERR_UNAVAILRESOURCE             ERR_RESTRICTED

	int			fd = client.getFd();
	std::string	nick = client.getNickname().empty() ? "*" : client.getNickname();

	if (!client.isPasswdOK())
	{
		sendToClient(fd, errPasswdMismatch(SERVER_NAME));
		return ;
	}

	std::cout << "NICK command by " << message.getSender() << std::endl;

	if (message.getBufferDivided().size() < 2)
	{
		sendToClient(fd, errNoNicknameGiven(SERVER_NAME, nick));
		return ;
	}

	std::string	newNick = message.getBufferDivided()[1];

	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.getNickname() == newNick && it->first != client.getFd()) {
			sendToClient(fd, errNicknameInUse(SERVER_NAME, nick, newNick));
			return ;
		}
	}
	client.setNickname(newNick);
	//client.setNickname(message.getBufferDivided()[1]);
	client.setNicknameOK(true);

	if (!client.isRegistered()) // 1st time registeration
	{
		if (client.isUsernameOK() == true) {
			if (client.isPasswdOK() == true)
			{
				client.setRegistered(true);
				this->welcomeMessages(client);
			}
			else
				throw Server::BadPassword();
		}
	}
	else // Nick reset
	{
		std::string	str = ":" + nick + " NICK :" + newNick + "\r\n";
		sendToClient(fd, str);
	}
};

void Server::user(Message & message, Client &client) {
	// Numeric Replies:

    //        ✓ERR_NEEDMOREPARAMS              ERR_ALREADYREGISTRED

	int			fd = client.getFd();
	std::string	nick = client.getNickname().empty() ? "*" : client.getNickname();

	if (!client.isPasswdOK())
	{
		sendToClient(fd, errPasswdMismatch(SERVER_NAME));
		return ;
	}

	if (client.isRegistered())
	{
		sendToClient(fd, errAlreadyRegistered(SERVER_NAME, nick));
		return ;
	}

	std::cout << "USER command by " << message.getSender() << std::endl;

	if (message.getBufferDivided().size() < 2)
	{
		sendToClient(fd, errNeedMoreParams(SERVER_NAME, nick, "USER"));
		return ;
	}

	client.setUsername(message.getBufferDivided()[1]);
	client.setUsernameOK(true);
	
	if (client.isNicknameOK() == true) {
		if (client.isPasswdOK() == true)
		{
			client.setRegistered(true);
			this->welcomeMessages(client);
		}
		else
			throw Server::BadPassword();
	}
};

void Server::join(Message & message, Client &client) {
	// Numeric Replies:

    //        ✓ERR_NEEDMOREPARAMS              ERR_BANNEDFROMCHAN
    //        ERR_INVITEONLYCHAN              ERR_BADCHANNELKEY
    //        ERR_CHANNELISFULL               ERR_BADCHANMASK
    //        ERR_NOSUCHCHANNEL               ERR_TOOMANYCHANNELS
    //        ERR_TOOMANYTARGETS              ERR_UNAVAILRESOURCE
    //        RPL_TOPIC
	std::cout << "JOIN command by " << message.getSender() << std::endl;
	(void)client;
};

void Server::part(Message & message, Client &client) {
	// Numeric Replies:

    //        ✓ERR_NEEDMOREPARAMS              ERR_NOSUCHCHANNEL
    //        ERR_NOTONCHANNEL
	std::cout << "PART command by " << message.getSender() << std::endl;
	(void)client;
};

void Server::topic(Message & message, Client &client) {
	// Numeric Replies:

	// ✓ERR_NEEDMOREPARAMS              ERR_NOTONCHANNEL
	// RPL_NOTOPIC                     RPL_TOPIC
	// ERR_CHANOPRIVSNEEDED            ERR_NOCHANMODES
	std::cout << "TOPIC command by " << message.getSender() << std::endl;
	(void)client;
};

void Server::invite(Message & message, Client &client) {
	// Numeric Replies:

    //        ✓ERR_NEEDMOREPARAMS              ERR_NOSUCHNICK
    //        ERR_NOTONCHANNEL                ERR_USERONCHANNEL
    //        ERR_CHANOPRIVSNEEDED
    //        RPL_INVITING                    RPL_AWAY
	std::cout << "INVITE command by " << message.getSender() << std::endl;
	(void)client;
};

void Server::kick(Message & message, Client &client) {
	// Numeric Replies:

    //        ✓ERR_NEEDMOREPARAMS              ERR_NOSUCHCHANNEL
    //        ERR_BADCHANMASK                 ERR_CHANOPRIVSNEEDED
    //        ERR_USERNOTINCHANNEL            ERR_NOTONCHANNEL
	std::cout << "KICK command by " << message.getSender() << std::endl;
	(void)client;
};

void Server::quit(Message & message, Client &client) {
	std::cout << "QUIT command by " << message.getSender() << std::endl;
	(void)client;
};

void Server::mode(Message & message, Client &client) {
	// Numeric Replies:

    //        ✓ERR_NEEDMOREPARAMS              ✓ERR_USERSDONTMATCH
    //        ✓ERR_UMODEUNKNOWNFLAG            ✓RPL_UMODEIS
	std::cout << "MODE command by " << message.getSender() << std::endl;
	(void)client;
};

void Server::privmsg(Message & message, Client &client) {
	// Numeric Replies:

    //        ✓ERR_NORECIPIENT                 ✓ERR_NOTEXTTOSEND
    //        ✓ERR_CANNOTSENDTOCHAN            ✓ERR_NOTOPLEVEL
    //        ✓ERR_WILDTOPLEVEL                ✓ERR_TOOMANYTARGETS
    //        ✓ERR_NOSUCHNICK
    //        ✓RPL_AWAY
	std::cout << "PRIVMSG command by " << message.getSender() << std::endl;
	if (message.getBufferDivided()[1].empty()) {
		this->sendToClient(client.getFd(), errNoRecipient(SERVER_NAME, client.getNickname(), \
			"PRIVMSG"));
		return;
	}
	if (message.getBufferDivided()[2].empty()) {
		this->sendToClient(client.getFd(), errNoTextToSend(SERVER_NAME, client.getNickname()));
		return;
	}
	try {
		message.setReceiver();
		Client receiver = message.getReceiver();
		std::cout << "Receiver of PRIVMSG" + receiver.getNickname() << std::endl;
		this->sendToClient(receiver.getFd(), message.getBufferDivided()[2]);
	}
	catch (Message::NoSuchNick & e) {
		sendToClient(client.getFd(), errNoSuchNick(SERVER_NAME, message.getSender(), \
			message.getBufferDivided()[1]));
	}
};

void	Server::sendToClient(int fd, const std::string &msg)
{
	send(fd, msg.c_str(), msg.length(), 0);
}

void	Server::welcomeMessages(Client &client)
{
	int			fd = client.getFd();
	std::string	nick = client.getNickname();
	std::string	userModes = "o";
	std::string	channelModes = "itkol";

	sendToClient(fd, rplWelcome(SERVER_NAME, nick));
	sendToClient(fd, rplYourHost(SERVER_NAME, nick));
	sendToClient(fd, rplCreated(SERVER_NAME, nick));
	sendToClient(fd, rplMyInfo(SERVER_NAME, nick, userModes, channelModes));
}

std::map<int, Client> Server::getClients(void) const {
	return this->_clients;
}
