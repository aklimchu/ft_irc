#include "Server.hpp"
#include "server_replies.hpp"

bool Server::_signal_received = false;

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
	// close client fds (i has to start from 1, because _sockfd is the first element in _pollFds!)
	for(size_t i = 1; i < this->_pollFds.size(); i++)
	{
		if (this->_pollFds[i].fd > -1)
			close(this->_pollFds[i].fd);
	}
}

void	Server::setNonBlock(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Error: fcntl() failed in setNonBlock()");
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

	//this->setNonBlock(this->_sockfd); // For MacOS
}

void	Server::handleNewClient()
{
	int	client_socket = accept(this->_sockfd, NULL, NULL);

	if (client_socket < 0)
		throw std::runtime_error("Error: accept() failed in handleNewClient()");

	//this->setNonBlock(client_socket);  // For MacOS

	pollfd	new_client;
	std::memset(&new_client, 0, sizeof(new_client));
	new_client.fd = client_socket;
	new_client.events = POLLIN;
	this->_pollFds.push_back(new_client);

	this->_clients[client_socket] = Client(client_socket);
	this->_clients[client_socket].setHostname(this->getIP(client_socket));

	std::cout << "New client connected" << std::endl;
}

/* Erasing from the pollFds can only be done here, and the index i has to be decremented,
so that the loop logic in startServer() stays consistent!
Also the hasQuit flag has to be checked here, so that already cleaned(in quit()) memory
will not be attempted to cleaned again!*/
void	Server::handleOldClient(size_t &i)
{
	char	buffer[1024];
	int		fd = this->_pollFds[i].fd;
	Client	&client = this->_clients[fd];

	std::memset(buffer, 0, sizeof(buffer));
	ssize_t	bytes_read = recv(this->_pollFds[i].fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_read <= 0) // Disconnet/Error
	{
		if (bytes_read == 0)
			std::cout << "Client disconnected" << std::endl;
		else if (bytes_read < 0)
			std::cerr << "Error: recv()" << std::endl;
		// Client didnt send quit, have to call quit manually
		if (this->_clients.find(fd) != _clients.end() && !client.getHasQuit())
		{
			Message	quitMsg("QUIT :Connection closed", this->_clients);
			this->quit(quitMsg, client);
		}
		close(fd);
		this->_clients.erase(fd); // Have to erase this first before below!!!
		this->_pollFds.erase(this->_pollFds.begin() + i);
		i--;
	}
	else // Incoming data
	{
		std::cout << "Data received:" << buffer << std::endl;

		client.appendBuffer(buffer);

		std::string	&buf = client.getBuffer();
		size_t		pos;

		// Added the 2cnd condition into the loop, to work with netcat, which only adds the '\n'
		while((pos = buf.find("\r\n")) != std::string::npos || (pos = buf.find("\n")) != std::string::npos)
		{
			std::string	line = buf.substr(0, pos);
			if (buf[pos] == '\r')
				buf.erase(0, pos + 2);
			else
				buf.erase(0, pos + 1); // Added to work with netcat

			std::cout << "Parsed line: " << line << std::endl;

			try {
				this->executeCommand(line, client);
			}
			catch (std::exception &) {
				throw;
			}
		}
	}
}

void Server::startServer(void)
{
	pollfd	server_pollfd;

	std::memset(&server_pollfd, 0, sizeof(server_pollfd));
	server_pollfd.fd = this->_sockfd;
	server_pollfd.events = POLLIN;
	this->_pollFds.push_back(server_pollfd);

	while (!this->_signal_received)
	{
		int	ready_fds = poll(this->_pollFds.data(), this->_pollFds.size(), -1);

		if (ready_fds < 0)
			throw std::runtime_error("Error: poll() failed in startServer()");

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
					catch (std::exception &) {
						throw;
					}
				}
			}
		}
	}
	this->closeFds();
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
		catch (std::exception &) {
			throw;
		}
    } else {
        std::cerr << "Unknown command: " << message_received.getSender() << std::endl << std::endl;
    }
}

void Server::pass(Message & message, Client &client) {
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

	// Check if nick already taken
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (it->second.getNickname() == newNick && it->first != client.getFd()) {
			sendToClient(fd, errNicknameInUse(SERVER_NAME, nick, newNick));
			return ;
		}
	}
	client.setNickname(newNick);
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
		std::string	user = client.getUsername().empty() ? "user" : client.getUsername();
		std::string	host = client.getHostname();
		std::string	str = ":" + nick + "!" + user + "@" + host + " NICK :" + newNick + "\r\n";

		// Check shared channels between clients, and only send to the ones that share a channel!
		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
		{
			if (it->first == fd || this->sharedChannel(client, it->second))
				sendToClient(it->first, str);
		}
	}
};

void Server::user(Message & message, Client &client) {
	std::cout << "USER command by " << message.getSender() << std::endl;

	int							fd = client.getFd();
	std::string					nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::vector<std::string>	&args = message.getBufferDivided();

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
	if (args.size() < 5)
	{
		sendToClient(fd, errNeedMoreParams(SERVER_NAME, nick, "USER"));
		return ;
	}

	std::string	realname = args[4];

	client.setUsername(args[1]);
	for (size_t i = 5; i < args.size(); i++) // Extract real name from the args
		realname += " " + args[i];
	if (!realname.empty() && realname[0] == ':')
		realname.erase(0, 1);
	client.setRealname(realname);
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
	std::cout << "JOIN command by " << message.getSender() << std::endl;

	int							fd = client.getFd();
	std::string					nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::vector<std::string>	&args = message.getBufferDivided();

	if (args.size() < 2)
	{
		sendToClient(fd, errNeedMoreParams(SERVER_NAME, nick, "JOIN"));
		return ;
	}

	std::vector<std::string>	channels = message.ft_split(args[1], ',');
	std::vector<std::string>	keys;

	if (args.size() > 2) // Store the channel keys, if any were given
		keys = message.ft_split(args[2], ',');

	for (size_t i = 0; i < channels.size(); i++)
	{
		// If it doesnt exist, create it (the first member should be set as operator)
		if (this->_channels.find(channels[i]) == this->_channels.end())
		{
			this->_channels[channels[i]] = Channel(channels[i]);
		}

		Channel	&channel = this->_channels[channels[i]];

		if (channel.isUser(&client)) // For joining a channel, where already a member
			continue;
		// Check if a 'i' channel mode is set
		if (channel.getChannelModes().find('i') != std::string::npos)
		{
			// Check if the client is in the invited list
			if (channel.getInvitedUsers().find(&client) == channel.getInvitedUsers().end())
			{
				sendToClient(fd, errInviteOnlyChan(SERVER_NAME, nick, channels[i]));
				continue ;
			}
		}
		// Check if a 'k' channel mode is set
		if (channel.getChannelModes().find('k') != std::string::npos)
		{
			// Check if key was even given, and compare it to the channel password
			if (i >= keys.size() || keys[i] != channel.getPassword())
			{
				sendToClient(fd, errBadChannelKey(SERVER_NAME, nick, channels[i]));
				continue ;
			}
		}
		// Check if a 'l' channel mode is set
		if (channel.getChannelModes().find('l') != std::string::npos)
		{
			// Compare current users to the user limit
			if (channel.getUsers().size() >= channel.getUserLimit())
			{
				sendToClient(fd, errChannelIsFull(SERVER_NAME, nick, channels[i]));
				continue ;
			}
		}
		channel.addUser(&client);
		client.joinChannel(channels[i]);
		channel.removeInvite(&client); // After succesful join, the invite has to be removed
		// If the client was the first channel member(creator), add it as an op
		if (channel.getUsers().size() == 1) 
			channel.setAsOperator(&client);
	
		// JOIN message to all channel members
		std::string	str = ":" + nick + "!" + client.getUsername() + "@" + client.getHostname()
			+ " JOIN :" + channels[i] + "\r\n";
		const std::set<Client *>	&users = channel.getUsers();

		for (std::set<Client *>::iterator it = users.begin(); it != users.end(); it++)
			sendToClient((*it)->getFd(), str);

		// If the channel topic is set, send RPL_TOPIC to the joiner
		if (!channel.getTopic().empty())
			sendToClient(fd, rplTopic(SERVER_NAME, nick, channels[i], channel.getTopic()));

		// Handle sending 353 RPL_NAMREPLY and 366 RPL_ENDOFNAMES
		// Add "@" prefix to operator names
		std::string	names;
		for (std::set<Client *>::const_iterator it = users.begin(); it != users.end(); it++)
		{
			if (channel.isOperator(*it))
				names += '@';
			names += (*it)->getNickname() + " ";
		}
		if (!names.empty())
			names.pop_back();
		sendToClient(fd, rplNamReply(SERVER_NAME, nick, channels[i], names));
		sendToClient(fd, rplEndOfNames(SERVER_NAME, nick, channels[i]));
	}
};

void Server::part(Message & message, Client &client) {
	std::cout << "PART command by " << message.getSender() << std::endl;

	int							fd = client.getFd();
	std::string					nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::vector<std::string>	&args = message.getBufferDivided();
	std::string					partMsg = "";

	if (args.size() < 2)
	{
		sendToClient(fd, errNeedMoreParams(SERVER_NAME, nick, "PART"));
		return ;
	}

	std::vector<std::string>	channels = message.ft_split(args[1], ',');

	if (args.size() > 2) // Build the part msg, if it was included
	{
		partMsg = args[2];
		if (partMsg[0] == ':')
			partMsg.erase(0, 1);
		for (size_t i = 3; i < args.size(); i++)
			partMsg += " " + args[i];
	}
	// Loop through each channel given as argument
	for(size_t i = 0; i < channels.size(); i++)
	{
		std::string	&channelName = channels[i];

		// Check if the channel exists
		if (this->_channels.find(channelName) == this->_channels.end())
		{
			sendToClient(fd, errNoSuchChannel(SERVER_NAME, nick, channelName));
			continue;
		}

		Channel	&channel = this->_channels[channelName];

		// Check if the client is on that channel
		if (!channel.isUser(&client))
		{
			sendToClient(fd, errNotOnChannel(SERVER_NAME, nick, channelName));
			continue;
		}

		std::string					fullMsg = ":" + nick + "!"+ client.getUsername() + "@"
			+ client.getHostname() + " PART " + channelName;// + " :" + partMsg + "\r\n";
		if (!partMsg.empty())
			fullMsg += " :" + partMsg;
		fullMsg += "\r\n";
		const std::set<Client *>	&users = channel.getUsers();

		// Send the full part msg to all the clients on that channel
		for (std::set<Client *>::const_iterator it = users.begin(); it != users.end(); it++)
			sendToClient((*it)->getFd(), fullMsg);
		if (channel.isOperator(&client)) // Check if the client is an op and needs to be removed
			channel.removeOperator(&client);
		channel.removeUser(&client); // Remove the client from the channel
		client.leaveChannel(channelName); // Remove the channel from the clients list of channels
		if (channel.getUsers().empty()) // If channel is left empty, remove it
			this->_channels.erase(channelName);
	}
};

void Server::topic(Message & message, Client &client) {
	std::cout << "TOPIC command by " << message.getSender() << std::endl;

	int							fd = client.getFd();
	std::string					nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::vector<std::string>	&args = message.getBufferDivided();

	if (args.size() < 2)
	{
		sendToClient(fd, errNeedMoreParams(SERVER_NAME, nick, "TOPIC"));
		return ;
	}

	std::string	channelName = args[1];
	std::map<std::string, Channel>::iterator it = this->_channels.find(channelName);
	
	// Check if the channel exists
	if (it == this->_channels.end())
	{
		sendToClient(fd, errNoSuchChannel(SERVER_NAME, nick, channelName));
		return ;
	}

	Channel	&channel = it->second;

	// Check if the client is a member of the channel
	if (!channel.isUser(&client))
	{
		sendToClient(fd, errNotOnChannel(SERVER_NAME, nick, channelName));
		return ;
	}

	// Just show the current topic, if no args given after channel name
	if (args.size() == 2)
	{
		if (channel.getTopic().empty())
			sendToClient(fd, rplNoTopic(SERVER_NAME, nick, channelName));
		else
			sendToClient(fd, rplTopic(SERVER_NAME, nick, channelName, channel.getTopic()));
		return ;
	}

	// Check if the client has to be an op to set the topic
	if (channel.getChannelModes().find('t') != std::string::npos)
	{
		if (!channel.isOperator(&client)) // Check if the client is op on the channel
		{
			sendToClient(fd, errChanOPrivNeeded(SERVER_NAME, nick, channelName));
			return ;
		}
	}

	// Build and set the new topic
	std::string	newTopic = args[2];

	if (newTopic[0] == ':')
		newTopic.erase(0, 1);
	for (size_t i = 3; i < args.size(); i++)
		newTopic += " " + args[i];
	channel.setTopic(newTopic);

	std::string	topicChangeMsg = ":" + nick + "!" + client.getUsername() + "@"
		+ client.getHostname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
	
	// Send the full topic change message to all the clients in the channel
	for (std::set<Client *>::const_iterator it = channel.getUsers().begin();
		it != channel.getUsers().end(); it++)
		sendToClient((*it)->getFd(), topicChangeMsg);
};

void Server::invite(Message & message, Client &client) {
	std::cout << "INVITE command by " << message.getSender() << std::endl;
	int							fd = client.getFd();
	std::string					nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::vector<std::string>	&args = message.getBufferDivided();

	if (args.size() < 3)
	{
		sendToClient(fd, errNeedMoreParams(SERVER_NAME, nick, "INVITE"));
		return;
	}

	// check if channel exists
	auto it = _channels.find(args[2]);
    if (it == _channels.end()) {
		sendToClient(fd, errNoSuchChannel(SERVER_NAME, nick, args[2]));
		return;
	}
	Channel &targetChannel = it->second;

	// check if caller is in the channel
    if (!targetChannel.isUser(&client)) {
		sendToClient(fd, errNotOnChannel(SERVER_NAME, nick, args[2]));
		return;
	}

	// check if caller is op
	if (!targetChannel.isOperator(&client)) {
        sendToClient(fd, errChanOPrivNeeded(SERVER_NAME, nick, args[2]));
		return;
	}

	// check if target user exists in network
	auto it4 = std::find_if(this->_clients.begin(), _clients.end(),
	[&args](const std::pair<const int, Client> &pair) {
		return pair.second.getNickname() == args[1];
	});
	if (it4 == _clients.end()) {
		sendToClient(fd, errNoSuchNick(SERVER_NAME, nick, args[1]));
		return;
	}
	Client &targetUser = it4->second;

	// check if target user is already on channel 
	if (targetChannel.isUser(&targetUser)) {
        sendToClient(fd, errUserOnChannel(SERVER_NAME, nick, targetUser.getNickname(), args[2]));
		return;
	}

	// check if target user is already in the list of invited users
	auto it5 = targetChannel.getInvitedUsers().find(&targetUser);
    if (it5 != targetChannel.getInvitedUsers().end()) {
		return;
	}

	// add target user to list on invited users in the channel
	targetChannel.addInvite(&targetUser);

	// send an invitation to target user to join the channel
	sendToClient(targetUser.getFd(), rplInviting(SERVER_NAME, nick, targetUser.getNickname(), args[2]));
};

void Server::kick(Message & message, Client &client) {
	std::cout << "KICK command by " << message.getSender() << std::endl;

	int							fd = client.getFd();
	std::string					nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::vector<std::string>	&args = message.getBufferDivided();

	if (args.size() < 3)
	{
		sendToClient(fd, errNeedMoreParams(SERVER_NAME, nick, "KICK"));
		return ;
	}

	std::string	channelName = args[1];
	std::string	targetNick = args[2];
	std::string	kickMsg = "Kicked";

	if (args.size() > 3) // Build a kick msg if it was sent
	{
		kickMsg = args[3];
		if (kickMsg[0] == ':')
			kickMsg.erase(0, 1);
		for (size_t i = 4; i < args.size(); i++)
			kickMsg += " " + args[i];
	}

	// Check if the channel exists
	std::map<std::string, Channel>::iterator it = this->_channels.find(channelName);
	if (it == this->_channels.end())
	{
		sendToClient(fd, errNoSuchChannel(SERVER_NAME, nick, channelName));
		return ;
	}

	Channel	&channel = it->second;

	// Check if the kicker is on the channel
	if (!channel.isUser(&client))
	{
		sendToClient(fd, errNotOnChannel(SERVER_NAME, nick, channelName));
		return ;
	}
	// Check if the kicker is an op
	if (!channel.isOperator(&client))
	{
		sendToClient(fd, errChanOPrivNeeded(SERVER_NAME, nick, channelName));
		return ;
	}

	Client	*targetClient = NULL;

	// Search for the target client
	for (std::map<int, Client>::iterator it_2 = this->_clients.begin(); it_2 != this->_clients.end(); it_2++)
	{
		if (it_2->second.getNickname() == targetNick)
		{
			targetClient = &(it_2->second);
			break;
		}
	}
	if (!targetClient)
	{
		sendToClient(fd, errNoSuchNick(SERVER_NAME, nick, targetNick));
		return ;
	}
	// Check if the target is in the channel
	if (!channel.isUser(targetClient))
	{
		sendToClient(fd, errUserNotInChannel(SERVER_NAME, nick, targetNick, channelName));
		return ;
	}

	// Full msg
	std::string	fullMsg = ":" + nick + "!" + client.getUsername() + "@" + client.getHostname()
		+ " KICK " + channelName + " " + targetNick + " :" + kickMsg + "\r\n";

	const std::set<Client *>	&users = channel.getUsers();

	// Send the full kick msg to all users in the channel
	for (std::set<Client *>::const_iterator it_3 = users.begin(); it_3 != users.end(); it_3++)
		sendToClient((*it_3)->getFd(), fullMsg);
	// Remove the target client from the channel
	channel.removeUser(targetClient);
	// Remove the channel from the target clients own list
	targetClient->leaveChannel(channelName);
	// If target was an op in the channel, remove it
	if (channel.isOperator(targetClient))
		channel.removeOperator(targetClient);
	// If the channel is left empty, delete it (only possible if the kicker kicked itself)
	if (channel.getUsers().empty())
		this->_channels.erase(channelName);
};

void Server::quit(Message & message, Client &client) {
	std::cout << "QUIT command by " << message.getSender() << std::endl;

	std::string					nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::string					quitMsg = "Client Quit"; // default quit msg
	std::vector<std::string>	&args = message.getBufferDivided();

	if (args.size() > 1) // Build the quit msg, if client sent something specific
	{
		quitMsg = args[1];
		if (quitMsg[0] == ':')
			quitMsg.erase(0, 1);
		for (size_t i = 2; i < args.size(); i++)
			quitMsg += " " + args[i];
	}

	std::string	fullMsg = ":" + nick + "!" + client.getUsername() + "@" + client.getHostname() + " QUIT :" + quitMsg + "\r\n";

	// Check shared channels between clients, and only send to the ones that share a channel!
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (it->first != client.getFd() && this->sharedChannel(client, it->second))
			sendToClient(it->first, fullMsg);
	}

	// Remove the quitting client from the channels it belongs to
	for (std::set<std::string>::const_iterator it = client.getChannels().begin(); it != client.getChannels().end(); it++)
	{
		std::string	channelName = *it;
		std::map<std::string, Channel>::iterator	it_2 = this->_channels.find(channelName);
		if (it_2 != this->_channels.end())
		{
			if (it_2->second.isOperator(&client)) // If the client is an op on the channel, client gets removed
				it_2->second.removeOperator(&client);
			it_2->second.removeUser(&client);
			if (it_2->second.getUsers().empty()) // If the channel is left empty, it gets removed
				this->_channels.erase(it_2);
		}
	}
	// Remove any invitations to any channel, since the client wont exist anymore
	for (std::map<std::string, Channel>::iterator it_3 = this->_channels.begin(); it_3 != this->_channels.end(); it_3++)
		it_3->second.removeInvite(&client);
	client.setHasQuit(true);
};

void Server::mode(Message & message, Client &client) {
	std::cout << "MODE command by " << message.getSender() << std::endl;

	int							fd = client.getFd();
	std::string					nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::vector<std::string>	&args = message.getBufferDivided();

	if (args.size() < 2)
	{
		sendToClient(fd, errNeedMoreParams(SERVER_NAME, nick, "MODE"));
		return ;
	}
	// User modes (only +i, because irssi sends it?)
	if (args[1][0] != '#')
	{
		if (args[1] != nick) // Check if its their own nick
		{
			sendToClient(fd, errUsersDontMatch(SERVER_NAME, nick));
			return ;
		}
		if (args.size() == 2) // Only shows usermodes
		{
			sendToClient(fd, rplUModeIs(SERVER_NAME, nick, "+" + client.getUsermodes()));
			return ;
		}
		// Add/remove mode(s)
		std::string	mode = args[2];
		for (size_t i = 1; i < mode.size(); i++)
		{
			if (mode[0] == '+')
				client.addUsermode(mode[i]);
			else if (mode[0] == '-')
				client.removeUsermode(mode[i]);
		}
		sendToClient(fd, rplUModeIs(SERVER_NAME, nick, "+" + client.getUsermodes()));
	}
	else // Channel modes
	{
		// find needed channel instance
		try {
			Channel & channel = message.getReceiverChannel(this->_channels);

			// MODE with no parameters
			if (args.size() == 2) // Only shows channel modes
			{
				sendToClient(fd, rplChannelModeIs(SERVER_NAME, nick, channel.getName(), \
					"+" + channel.getChannelModes()));
				return;
			}
			// Add/remove mode(s)
			std::string	&mode = args[2];
			std::string successfulChanges = "";
			if (mode[0] == '+') {
				successfulChanges = channel.addChannelModes(args, _clients, client);
			}
			else if (mode[0] == '-') {
				successfulChanges = channel.removeChannelModes(args, _clients, client);
			}
			// send information about changes to channel members
			if (!successfulChanges.empty()) {
				std::string senderPrefix = ":" + client.getNickname() + "!" + \
					client.getUsername() + "@" + client.getHostname();
				std::string toSend = senderPrefix + " MODE " + channel.getName() + \
					' ' + mode[0] + successfulChanges + "\r\n"; 
				this->sendToChannel(toSend, channel);
			}
		}
		catch (Message::NoSuchChannel & e) {
			sendToClient(client.getFd(), errNoSuchChannel(SERVER_NAME, message.getSender(), \
				args[1]));
			return;
		}
	}
	
};

void Server::privmsg(Message & message, Client &client) {
	std::cout << "PRIVMSG command by " << message.getSender() << std::endl;
	std::vector<std::string>	&args = message.getBufferDivided();

	if (args.size() == 1 || args[1].empty()) {
		this->sendToClient(client.getFd(), errNoRecipient(SERVER_NAME, client.getNickname(), \
			"PRIVMSG"));
		return;
	}
	if (args.size() < 3 || args[2].empty()) {
		this->sendToClient(client.getFd(), errNoTextToSend(SERVER_NAME, client.getNickname()));
		return;
	}

	// check if user is sending a message to another client or to a channel
	try {
		if (args[1][0] == '#') {
			broadcastMessageToChannel(args, message, client);
		}
		else {
			sendMessageToClient(args, message, client);
		}
	}
	catch (Message::NoSuchNick & e) {
		sendToClient(client.getFd(), errNoSuchNick(SERVER_NAME, message.getSender(), \
			args[1]));
	}
	catch (Message::NoSuchChannel & e) {
		sendToClient(client.getFd(), errNoSuchChannel(SERVER_NAME, message.getSender(), \
			args[1]));
	}
};

void Server::sendMessageToClient(std::vector<std::string> & args, Message & message, Client &client) {
	std::string messageText = args[2];
	for (size_t i = 3; i < args.size(); i++) {
		messageText += ' ' + args[i];
	}
	if (messageText[0] == ':') {
    	messageText = messageText.substr(1);
	}

	// find needed client instance
	Client & receiver = message.getReceiverClient();

	// build a message
	std::cout << "Receiver of PRIVMSG " + receiver.getNickname() << std::endl;
	std::string senderPrefix = ":" + client.getNickname() + "!" + client.getUsername() \
		+ "@" + client.getHostname();
    std::string privmsg = senderPrefix + " PRIVMSG " + receiver.getNickname() + " :" + messageText + "\r\n";
	
	// send a message
	int sendResult = this->sendToClient(receiver.getFd(), privmsg);
    if (sendResult == -1) {
        std::cerr << "Failed to send message to fd: " << receiver.getFd() << ", errno: " << errno << std::endl;
    } else {
        std::cout << "Successfully sent " << sendResult << " bytes to fd: " << receiver.getFd() << std::endl;
    }
}

void Server::broadcastMessageToChannel(std::vector<std::string> & args, Message & message, Client &client) {
	std::string messageText = args[2];
	for (size_t i = 3; i < args.size(); i++) {
		messageText += ' ' + args[i];
	}
	if (messageText[0] == ':') {
    	messageText = messageText.substr(1);
	}

	// find the needed channel instance
	Channel & targetChannel = message.getReceiverChannel(this->_channels);
	const std::set<Client *> & targetUsers = targetChannel.getUsers();
	std::set<Client *>::iterator itr;

	// send message to the clients who joined the channel
	for (itr = targetUsers.begin(); itr != targetUsers.end(); itr++) {
		if (*itr != &client) {
			std::string senderPrefix = ":" + client.getNickname() + "!" + client.getUsername() \
			+ "@" + client.getHostname();
    		std::string privmsg = senderPrefix + " PRIVMSG " + targetChannel.getName() + " :" + messageText + "\r\n";

			int sendResult = this->sendToClient((**itr).getFd(), privmsg);
    		if (sendResult == -1) {
    		    std::cerr << "Failed to send message to fd: " << (**itr).getFd() << ", errno: " << errno << std::endl;
    		} else {
    		    std::cout << "Successfully sent " << sendResult << " bytes to fd: " << (**itr).getFd() << std::endl;
    		}
		}
	}
}

void	Server::ping(Message &message, Client &client)
{
	std::vector<std::string> &args = message.getBufferDivided();

	if (args.size() < 2)
	{
		std::cerr << "PING command received with parameter missing" << std::endl;
		return ;
	}

	sendToClient(client.getFd(), "PONG :" + args[1] + "\r\n");
}

void	Server::cap(Message &message, Client &client)
{
	int							fd = client.getFd();
	std::vector<std::string>	&args = message.getBufferDivided();
	std::string					nick = client.getNickname().empty() ? "*" : client.getNickname();

	if (args.size() < 2)
		return ;
	if (args[1] == "LS")
		sendToClient(fd, "CAP " + nick + " LS :\r\n");
	else if (args[1] == "REQ")
	{
		if (args.size() < 3 || args[2].empty())
		{
			sendToClient(fd, "CAP " + nick + " NAK :\r\n");
			return ;
		}
		sendToClient(fd, "CAP " + nick + " NAK :" + args[2] + "\r\n");
	}
}

void	Server::whois(Message &message, Client &client)
{
	int							fd = client.getFd();
	std::string					nick = client.getNickname();
	std::vector<std::string>	&args = message.getBufferDivided();

	if (args.size() < 2)
	{
		sendToClient(fd, errNeedMoreParams(SERVER_NAME, nick, "WHOIS"));
		return ;
	}

	std::string								targetNick = args[1];
	std::map<int, Client>::const_iterator	it;
	
	// Search for the target client
	for (it = this->_clients.begin(); it != this->_clients.end(); it++)
	{
		if (it->second.getNickname() == targetNick)
			break;
	}
	if (it == _clients.end()) // Target client not found
	{
		sendToClient(fd, errNoSuchNick(SERVER_NAME, nick, targetNick));
		sendToClient(fd, rplEndOfWhois(SERVER_NAME, nick, targetNick));
		return ;
	}

	// Build a list of channels
	const Client	&target = it->second;
	std::string		channels;
	const std::set<std::string>	&channelsSet = target.getChannels();

	for (std::set<std::string>::const_iterator it = channelsSet.begin(); it != channelsSet.end(); it++)
		channels += *it + " ";
	if (!channels.empty())
		channels.pop_back();

	// Send replies back to requesting client
	std::string	host = client.getHostname();
	targetNick = target.getNickname();

	sendToClient(fd, rplWhoisUser(SERVER_NAME, nick, targetNick, target.getUsername(), host, target.getRealname()));
	sendToClient(fd, rplWhoisServer(SERVER_NAME, nick, targetNick));
	if (!channels.empty())
		sendToClient(fd, rplWhoisChannels(SERVER_NAME, nick, targetNick, channels));
	sendToClient(fd, rplEndOfWhois(SERVER_NAME, nick, targetNick));
}

void	Server::who(Message &message, Client &client)
{
	std::cout << "WHO command by " << message.getSender() << std::endl;

	int							fd = client.getFd();
	std::string					nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::vector<std::string>	&args = message.getBufferDivided();

	if (args.size() < 2)
	{
		sendToClient(fd, rplEndOfWho(SERVER_NAME, nick, "*"));
		return ;
	}

	std::string	channelName = args[1];

	// Check if channel exists
	if (this->_channels.find(channelName) == this->_channels.end())
	{
		sendToClient(fd, rplEndOfWho(SERVER_NAME, nick, channelName));
		return ;
	}

	Channel						&channel = this->_channels[channelName];
	const std::set<Client *>	&users = channel.getUsers();

	// Loop through every client on that channel and send the rplWhoReply to the requester
	for (std::set<Client *>::const_iterator it = users.begin(); it != users.end(); it++)
	{
		Client		*user = *it;
		std::string	flags = "H"; // Placeholder? (H is "Here", G is "Gone"). Add "@" to mark operator status?
		std::string	hopcountAndRealname = "0 " + user->getRealname();

		if (channel.isOperator(user)) // Adds the "@" prefix, if operator
			flags += "@";
		sendToClient(fd, rplWhoReply(SERVER_NAME, nick, channelName, user->getUsername(),
			user->getHostname(), SERVER_NAME, user->getNickname(), flags, hopcountAndRealname));
	}
	// Marks the end
	sendToClient(fd, rplEndOfWho(SERVER_NAME, nick, channelName));
}

int	Server::sendToClient(int fd, const std::string &msg)
{
	int bytesSent = send(fd, msg.c_str(), msg.length(), 0);
	if (bytesSent < 0) // in error case, recv() returning 0 will clean up the client
		std::cerr << "send() failed to send to fd: " << fd << std::endl;
	else
		std::cout << "Sent to fd: " << fd << " message: " << msg; // Debug
	return bytesSent;
}

void	Server::welcomeMessages(Client &client)
{
	int			fd = client.getFd();
	std::string	nick = client.getNickname();
	std::string	userModes = "io";
	std::string	channelModes = "itkol";

	sendToClient(fd, rplWelcome(SERVER_NAME, nick));
	sendToClient(fd, rplYourHost(SERVER_NAME, nick));
	sendToClient(fd, rplCreated(SERVER_NAME, nick));
	sendToClient(fd, rplMyInfo(SERVER_NAME, nick, userModes, channelModes));
}

std::map<int, Client> &Server::getClients(void) {
	return this->_clients;
}

bool	Server::sharedChannel(const Client &a, const Client &b) const
{
	const std::set<std::string>	&aChannels = a.getChannels();
	const std::set<std::string>	&bChannels = b.getChannels();

	for (std::set<std::string>::const_iterator it = aChannels.begin(); it != aChannels.end(); it++)
	{
		if (bChannels.find(*it) != bChannels.end())
			return (true);
	}
	return (false);
}

std::string	Server::getIP(int fd)
{
	struct sockaddr_in	addr;
	socklen_t			len = sizeof(addr);

	// Extract data from the fd(socket) into the addr struct
	if (getpeername(fd, (struct sockaddr *)&addr, &len) == -1)
		return ("unknown");

	char	ipStr[INET_ADDRSTRLEN];

	// Convert from binary to string
	if (!inet_ntop(AF_INET, &addr.sin_addr, ipStr, sizeof(ipStr)))
		return ("unknown");
	return (std::string(ipStr));
}

void Server::sendToChannel(const std::string &message, Channel &channel) {
    for (const auto &member : channel.getUsers()) {
        if (member) {
            auto it = std::find_if(_clients.begin(), _clients.end(),
                                   [&member](const auto &pair) {
                                       return pair.second.getNickname() == member->getNickname();
                                   });
            if (it != _clients.end()) {
                sendToClient(it->first, message);
            }
        }
    }
}
