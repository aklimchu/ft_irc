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

void	Server::handleNewClient()
{
	int	client_socket = accept(this->_sockfd, NULL, NULL);

	if (client_socket < 0)
		std::cerr << "Error: accept()" << std::endl; //

	this->setNonBlock(client_socket);

	pollfd	new_client;
	std::memset(&new_client, 0, sizeof(new_client));
	new_client.fd = client_socket;
	new_client.events = POLLIN;
	this->_pollFds.push_back(new_client);

	this->_clients[client_socket] = Client(client_socket);
	this->_clients[client_socket].setHostname(this->getIP(client_socket));

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

	std::memset(&server_pollfd, 0, sizeof(server_pollfd));
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
		std::string	host = client.getHostname();//"localhost"; // Placeholder?
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
	// Numeric Replies:

    //        ✓ERR_NEEDMOREPARAMS              ERR_ALREADYREGISTRED
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
	if (args.size() < 2)
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
	// Numeric Replies:

    //        ✓ERR_NEEDMOREPARAMS              ERR_BANNEDFROMCHAN
    //        ERR_INVITEONLYCHAN              ERR_BADCHANNELKEY
    //        ERR_CHANNELISFULL               ERR_BADCHANMASK
    //        ERR_NOSUCHCHANNEL               ERR_TOOMANYCHANNELS
    //        ERR_TOOMANYTARGETS              ERR_UNAVAILRESOURCE
    //        RPL_TOPIC
	std::cout << "JOIN command by " << message.getSender() << std::endl;

	int							fd = client.getFd();
	std::string					nick = client.getNickname().empty() ? "*" : client.getNickname();

	if (message.getBufferDivided().size() < 2)
	{
		sendToClient(fd, errNeedMoreParams(SERVER_NAME, nick, "JOIN"));
		return ;
	}
	std::string	channelName = message.getBufferDivided()[1];

	// If it doesnt exist, create it (the first member should be set as operator?)
	if (this->_channels.find(channelName) == this->_channels.end())
	{
		this->_channels[channelName] = Channel(channelName);
	}
	this->_channels[channelName].addUser(&client);
	client.joinChannel(channelName);
	
	// JOIN message to all channel members
	std::string	str = ":" + nick + " JOIN :" + channelName + "\r\n";
	const std::set<Client *>	&users = this->_channels[channelName].getUsers();

	for (std::set<Client *>::iterator it = users.begin(); it != users.end(); it++)
		sendToClient((*it)->getFd(), str);

	// Handle sending 353 RPL_NAMREPLY and 366 RPL_ENDOFNAMES
	// Add "@" prefix to operator names?
	std::string	names;
	for (std::set<Client *>::const_iterator it = users.begin(); it != users.end(); it++)
		names += (*it)->getNickname() + " ";
	if (!names.empty())
		names.pop_back();
	sendToClient(fd, rplNamReply(SERVER_NAME, nick, channelName, names));
	sendToClient(fd, rplEndOfNames(SERVER_NAME, nick, channelName));
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
		// 	  ERR_KEYSET
        //    ERR_NOCHANMODES                 ERR_CHANOPRIVSNEEDED
        //    ERR_USERNOTINCHANNEL            ERR_UNKNOWNMODE
        //    ✓RPL_CHANNELMODEIS
        //    RPL_BANLIST                     RPL_ENDOFBANLIST
        //    RPL_EXCEPTLIST                  RPL_ENDOFEXCEPTLIST
        //    RPL_INVITELIST                  RPL_ENDOFINVITELIST
        //    RPL_UNIQOPIS
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
			message.setReceiverChannel(this->_channels);
		}
		catch (Message::NoSuchChannel & e) {
			sendToClient(client.getFd(), errNoSuchChannel(SERVER_NAME, message.getSender(), \
				args[1]));
			return;
		}
		Channel & channel = message.getReceiverChannel();

		// MODE with no parameters
		if (args.size() == 2) // Only shows channel modes
		{
			sendToClient(fd, rplChannelModeIs(SERVER_NAME, nick, channel.getName(), \
				"+" + channel.getChannelModes()));
			return ;
		}
		// Add/remove mode(s)
		std::string	&mode = args[2];
		/* for (size_t i = 1; i < mode.size(); i++)
		{ */
			if (mode[0] == '+')
				channel.addChannelModes(args, _clients);
			else if (mode[0] == '-')
				channel.removeChannelModes(args);
		/* } */
		//sendToClient(fd, rplUModeIs(SERVER_NAME, nick, "+" + client.getUsermodes()));
	}
};

void Server::privmsg(Message & message, Client &client) {
	// Numeric Replies:

    //        ✓ERR_NORECIPIENT                 ✓ERR_NOTEXTTOSEND
    //        ✓ERR_CANNOTSENDTOCHAN            ✓ERR_NOTOPLEVEL
    //        ✓ERR_WILDTOPLEVEL                ✓ERR_TOOMANYTARGETS
    //        ✓ERR_NOSUCHNICK
    //        ✓RPL_AWAY
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
	if (messageText[0] == ':') {
    	messageText = messageText.substr(1);
	}

	// find needed client instance
	message.setReceiverClient();
	Client & receiver = message.getReceiverClient();

	// build a message
	std::cout << "Receiver of PRIVMSG " + receiver.getNickname() << std::endl;
	std::string senderPrefix = ":" + client.getNickname() + "!" + client.getUsername() \
		+ "@" + client.getHostname();
	// do we need to handle other hostnames?
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
	if (messageText[0] == ':') {
    	messageText = messageText.substr(1);
	}

	// find the needed channel instance
	message.setReceiverChannel(this->_channels);
	Channel & targetChannel = message.getReceiverChannel();

	//Check for modes like +n (no external messages), +m (only voiced/operator users can speak), or +b (bans).

	/* if (!targetChannel.isUserInChannel(&client)) {
    sendToClient(client.getFd(), errCannotSendToChan(SERVER_NAME, client.getNickname(), targetChannel.getName()));
    return;
	}
	if (targetChannel.hasMode('m') && !targetChannel.isVoiced(&client) && !targetChannel.isOperator(&client)) {
	    sendToClient(client.getFd(), errCannotSendToChan(SERVER_NAME, client.getNickname(), targetChannel.getName()));
	    return;
	} */

	const std::set<Client *> & targetUsers = targetChannel.getUsers();
	
	std::set<Client *>::iterator itr;

	// send message to the clients who joined the channel
	for (itr = targetUsers.begin(); itr != targetUsers.end(); itr++) {
		if (*itr != &client) {
			std::string senderPrefix = ":" + client.getNickname() + "!" + client.getUsername() \
			+ "@" + client.getHostname();
			// do we need to handle other hostnames?
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
		//sendToClient(fd, errNoNicknameGiven(SERVER_NAME, nick));
		// ERR_NEEDMOREPARAMS ???
		sendToClient(fd, errNeedMoreParams(SERVER_NAME, nick, "WHOIS")); // Correct?
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
	std::string	host = client.getHostname();//"localhost"; // Placeholder?
	targetNick = target.getNickname();

	sendToClient(fd, rplWhoisUser(SERVER_NAME, nick, targetNick, target.getUsername(), host, target.getRealname()));
	sendToClient(fd, rplWhoisServer(SERVER_NAME, nick, targetNick));
	if (!channels.empty())
		sendToClient(fd, rplWhoisChannels(SERVER_NAME, nick, targetNick, channels));
	sendToClient(fd, rplEndOfWhois(SERVER_NAME, nick, targetNick));
}

int	Server::sendToClient(int fd, const std::string &msg)
{
	int bytesSent = send(fd, msg.c_str(), msg.length(), 0);
	std::cout << "Sent to fd: " << fd << " message: " << msg; // Debug
	return bytesSent; // returning the number of bytes sent - for easier debugging
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

// Added to return a const reference
const std::map<int, Client> &Server::getClients(void) const {
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
