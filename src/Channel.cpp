#include "Channel.hpp"

//--------------------------------Constructors--------------------------------//

Channel::Channel(): _channelModes(""), _channelParams(""), _user_limit(0), \
	_password("") , _topic(""){}
Channel::Channel(const std::string &name) : _name(name), _channelModes(""), \
	_channelParams(""), _user_limit(0), _password(""), _topic("")
{
}

//-------------------------------Member functions------------------------------//

const std::string	&Channel::getName() const
{
	return (this->_name);
}

void	Channel::addUser(Client *client)
{
	this->_users.insert(client);
}

void	Channel::removeUser(Client *client)
{
	this->_users.erase(client);
}

bool	Channel::isUser(Client *client) const
{
	if (_users.find(client) != _users.end())
		return (true);
	else
		return (false);
}

bool	Channel::isOperator(Client *client) const
{
	if (this->_operators.find(client) != this->_operators.end())
		return (true);
	else
		return (false);
}

const	std::set<Client *>	&Channel::getUsers() const
{
	return (_users);
}

const	std::string Channel::getChannelModes() const {
	if (_channelParams.length())
		return _channelModes + " " + _channelParams;
	else
		return _channelModes;
}

const std::string	&Channel::getTopic(void) const
{
	return (this->_topic);
}

void	Channel::setTopic(const std::string &str)
{
	this->_topic = str;
}

void	Channel::setAsOperator(Client *client)
{
	this->_operators.insert(client);
}

std::string	Channel::addChannelModes(std::vector<std::string> &args, \
	std::map<int, Client> &serverUsers, Client &client) {
	
	// check if the user has operator privileges
	if (_operators.find(&client) == _operators.end()) {
    	channelSendToClient(client.getFd(), errChanOPrivNeeded(SERVER_NAME, \
			client.getNickname(), getName()));
    	return "";
	}
	
	size_t paramLimit = args.size() - 3;
	if (paramLimit > 3) 
		paramLimit = 3; // max 3 modes with optional parameters accepted
	std::string	mode = args[2];
	size_t paramCount = 0;
	std::string successfulChangesMode = "";
	std::string successfulChangesParam = "";

	// go symbol by symbol to add modes to the channel
	for (size_t i = 1; i < mode.length(); i++) {
		if (mode[i] != 'i' && mode[i] != 't' && mode[i] != 'k' && \
			mode[i] != 'l' && mode[i] != 'o') {
    		channelSendToClient(client.getFd(), \
       			errUnknownMode(SERVER_NAME, client.getNickname(), mode[i], this->getName()));
			continue;
		}

		// i, t
		if (mode[i] == 'i' || mode[i] == 't') {
			addITMode(mode[i], successfulChangesMode);
			continue;
		}

		// check if enough parameters
		if (args.size() < 3 + paramCount + 1) {
			channelSendToClient(client.getFd(), \
				errNeedMoreParams(SERVER_NAME, client.getNickname(), "MODE"));
			std::string returnStr = "";
			return(returnStr);
		}

		// moving to next parameter
		if (paramCount == paramLimit) {
			continue;
		}
		paramCount++;

		// k, l, o
		if (mode[i] == 'k' && addKeyToChannel(args, client, paramCount) == 0) {
			successfulChangesMode += 'k';
			if (!successfulChangesParam.empty()) {
				successfulChangesParam += " ";
			}
			successfulChangesParam += args[3 + paramCount - 1];
		}
		else if (mode[i] == 'l' && addLimitToChannel(args, client, paramCount) == 0) {
			successfulChangesMode += 'l';
			if (!successfulChangesParam.empty()) {
				successfulChangesParam += " ";
			}
			successfulChangesParam += args[3 + paramCount - 1];
		}
		else if (mode[i] == 'o' && \
			addOperatorToChannel(args, serverUsers, client, paramCount) == 0) {
			successfulChangesMode += 'o';
			if (!successfulChangesParam.empty()) {
				successfulChangesParam += " ";
			}
			successfulChangesParam += args[3 + paramCount - 1];
		}
	}
	if (!successfulChangesMode.empty()) {
		if (!successfulChangesParam.empty()) {
			return (successfulChangesMode + " " + successfulChangesParam);
		}
		else {
			return (successfulChangesMode);
		}
	}
	return("");
}

void Channel:: addITMode(const char & mode, std::string & successfulChangesMode) {
	if (_channelModes.find(mode) == std::string::npos) {
		_channelModes += mode;
	}
	successfulChangesMode += mode;
}

int Channel::addOperatorToChannel(std::vector<std::string> &args, \
	std::map<int, Client> &serverUsers, Client &client, size_t &paramCount) {
	// check if user exists on the server
	   auto it = std::find_if(serverUsers.begin(), serverUsers.end(),
	   [&args, paramCount](const std::pair<const int, Client> &pair) {
		   return pair.second.getNickname() == args[3 + paramCount - 1];
	   });
	if (it == serverUsers.end()) {
	   channelSendToClient(client.getFd(), \
		   errNoSuchNick(SERVER_NAME, client.getNickname(), args[3 + paramCount - 1]));
	   return -1;
   }

	// check if user exists in the channel and update privileges
	bool found = false;
	for (const auto& result : _users) {
		if (result && result->getNickname() == args[3 + paramCount - 1]) {
			Client & client = *result;
			client.setOperator(true); // do we need this?
			this->setAsOperator(&client);
			found = true;
			break;
		}
	}
	if (!found) {
		channelSendToClient(client.getFd(), \
			errUserNotInChannel(SERVER_NAME, client.getNickname(), \
			args[3 + paramCount - 1], this->getName()));
		return -1;
	}

	if (_channelModes.find('o') == std::string::npos) {
		_channelModes += 'o';
	}
	return 0;
}

int Channel::addKeyToChannel(std::vector<std::string> &args, Client &client, \
	size_t &paramCount) {
	// check if password is already set
	if (_channelModes.find('k') < _channelModes.length()) {
		channelSendToClient(client.getFd(), \
			errKeySet(SERVER_NAME, client.getNickname(), this->getName()));
		return -1;
	}

	// adding new password
	_channelModes += 'k';
	
	_password = args[3 + paramCount - 1];

	if (_channelParams.length() != 0) {
		_channelParams += " ";
	}
	_channelParams += args[3 + paramCount - 1];
	return 0;
}

int Channel::addLimitToChannel(std::vector<std::string> &args, Client &client, \
	size_t &paramCount) {
	//check if limit is positive integer
	int new_limit;
	try {
		new_limit = std::stoi(args[3 + paramCount - 1]);
		if (new_limit <= 0)
			throw std::exception();
	}
	catch (...) {
		channelSendToClient(client.getFd(), \
			errUnknownMode(SERVER_NAME, client.getNickname(), 'l', this->getName()));
		return -1;
	}

	// adding new user limit
	if (_channelModes.find('l') == std::string::npos) {
		_channelModes += 'l';
	}
	
	_user_limit = new_limit;

	if (_channelParams.length() != 0) {
		_channelParams += " ";
	}
	_channelParams += args[3 + paramCount - 1];
	return 0;
}

std::string Channel::removeChannelModes(std::vector<std::string> &args, \
	std::map<int, Client> &serverUsers, Client &client) {
	// check if the user has operator privileges
	if (_operators.find(&client) == _operators.end()) {
    	channelSendToClient(client.getFd(), errChanOPrivNeeded(SERVER_NAME, \
			client.getNickname(), getName()));
    	return "";
	}
	
	size_t paramLimit = args.size() - 3;
	if (paramLimit > 3) 
		paramLimit = 3; // max 3 modes with optional parameters accepted
	std::string	mode = args[2];
	size_t paramCount = 0;
	std::string successfulChangesMode = "";
	std::string successfulChangesParam = "";

	// go symbol by symbol to add modes to the channel
	for (size_t i = 1; i < mode.length(); i++) {
		if (mode[i] != 'i' && mode[i] != 't' && mode[i] != 'k' && \
			mode[i] != 'l' && mode[i] != 'o') {
    		channelSendToClient(client.getFd(), \
       			errUnknownMode(SERVER_NAME, client.getNickname(), mode[i], this->getName()));
			continue;
		}

		// i, t
		if (mode[i] == 'i' || mode[i] == 't') {
			removeITMode(mode[i], successfulChangesMode);
			continue;
		}

		// k, l
		if (mode[i] == 'k' && removeKeyFromChannel(args, client, paramCount) == 0) {
			successfulChangesMode += 'k';
			continue;
		}
		if (mode[i] == 'l' && removeLimitFromChannel(args, client, paramCount) == 0) {
			successfulChangesMode += 'l';
			continue;
		}

		// check if enough parameters
		if (args.size() < 3 + paramCount + 1) {
			channelSendToClient(client.getFd(), \
				errNeedMoreParams(SERVER_NAME, client.getNickname(), "MODE"));
			std::string returnStr = "";
			return(returnStr);
		}

		// moving to next parameter
		if (paramCount == paramLimit) {
			continue;
		}
		paramCount++;

		// o
		if (mode[i] == 'o' && \
			removeOperatorFromChannel(args, serverUsers, client, paramCount) == 0) {
			successfulChangesMode += 'o';
			if (!successfulChangesParam.empty()) {
				successfulChangesParam += " ";
			}
			successfulChangesParam += args[3 + paramCount - 1];
		}
	}
	if (!successfulChangesMode.empty()) {
		if (!successfulChangesParam.empty()) {
			return (successfulChangesMode + " " + successfulChangesParam);
		}
		else {
			return (successfulChangesMode);
		}
	}
	return("");
}

void Channel::removeITMode(const char & mode, std::string & successfulChangesMode) {
	auto it = _channelModes.find(mode);
	if (it != std::string::npos) {
		_channelModes.erase(it);
	}
	successfulChangesMode += mode;
}


int Channel::removeKeyFromChannel(std::vector<std::string> &args, Client &client, \
	size_t &paramCount) {
	auto it = _channelModes.find('k');
	if (it == std::string::npos) {
		return 0;
	}
	else {
		_channelModes.erase(it);
	}

	size_t paramIndex = findParamIndex('k');
	removeFromChannelParams(paramIndex);

	return 0;
}
	
int Channel::removeLimitFromChannel(std::vector<std::string> &args, Client &client, \
	size_t &paramCount) {
	auto it = _channelModes.find('l');
	if (it == std::string::npos) {
		return 0;
	}
	else {
		_channelModes.erase(it);
	}

	//erase from Params

	return 0;
}
		
int Channel::removeOperatorFromChannel(std::vector<std::string> &args, \
	std::map<int, Client> &serverUsers, Client &client, size_t &paramCount) {

}

size_t Channel::findParamIndex(char mode) {

}

void Channel::removeFromChannelParams(size_t paramIndex) {

}

int	Channel::channelSendToClient(int fd, const std::string &msg)
{
	int bytesSent = send(fd, msg.c_str(), msg.length(), 0);
	//std::cout << "Sent to fd: " << fd << " message: " << msg; // Debug
	return bytesSent; // returning the number of bytes sent - for easier debugging
}

void Channel::removeOperator(Client* client) {
    _operators.erase(client);
}
