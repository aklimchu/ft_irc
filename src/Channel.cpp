#include "Channel.hpp"

//--------------------------------Constructors--------------------------------//

Channel::Channel(): _channelModes(""), _channelParams("") {}
Channel::Channel(const std::string &name) : _name(name), _channelModes(""), \
	_channelParams("")
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

const	std::set<Client *>	&Channel::getUsers() const
{
	return (_users);
}

const	std::string	Channel::getChannelModes() const {
	if (_channelParams.size())
		return _channelModes + " " + _channelParams;
	else
		return _channelModes;
}

void	Channel::addChannelModes(std::vector<std::string> &args, \
	std::map<int, Client> &serverUsers, Client &client) {
	
	//checking the privileges?
	// ERR_CHANOPRIVSNEEDED
	
	size_t paramLimit = args.size() - 3;
	if (paramLimit > 3) 
		paramLimit = 3; // max 3 modes with optional parameters accepted
	std::string	&mode = args[2];
	size_t paramCount = 0;

	// go symbol by symbol to add modes to the channel
	for (size_t i = 1; i < mode.size(); i++)
	{
		//if mode[i] != ........
		//ERR_UNKNOWNMODE

		// i, t
		if (mode[i] == 'i' || mode[i] == 't') {
			_channelModes += mode[i];
			continue;
		}

		// check if enough parameters
		if (args.size() < 3 + paramCount + 1) {
			errNeedMoreParams(SERVER_NAME, client.getNickname(), "MODE");
			return;
		}

		// moving to next parameter
		if (paramCount == paramLimit) {
			continue;
		}
		paramCount++;

		// k, l, o
		if (mode[i] == 'k') {
			addKeyToChannel(args, client, paramCount);
		}
		else if (mode[i] == 'l') {
			addLimitToChannel(args, client, paramCount);
		}
		else if (mode[i] == 'o') {
			addOperatorToChannel(args, serverUsers, client, paramCount);
		}
	}
}

void	Channel::addOperatorToChannel(std::vector<std::string> &args, \
	std::map<int, Client> &serverUsers, Client &client, size_t &paramCount) {
	// check if user exists on the server
	bool found = false;
	for (const auto& pair : serverUsers) {
		if (pair.second.getNickname() == args[3 + paramCount - 1]) {
			found = true;
			break;
		}
	}
	if (!found) {
		errNoSuchNick(SERVER_NAME, client.getNickname(), args[3 + paramCount - 1]);
		return;
   	}

	// check if user exists in the channel and update privileges
	found = false;
	for (const auto& result : _users) {
		if (result->getNickname() == args[3 + paramCount - 1]) {
			Client & client = *result;
			_operators.insert(&client); // should we also set privileges to client?
			found = true;
			break;
		}
	}
	if (!found) {
		errUserNotInChannel(SERVER_NAME, client.getNickname(), \
			args[3 + paramCount - 1], this->getName());
		return;
	}
}

void Channel::addKeyToChannel(std::vector<std::string> &args, Client &client, \
	size_t &paramCount) {
	// check if password is already set
	if (_channelModes.find('k')) {
		errKeySet(SERVER_NAME, client.getNickname(), this->getName());
		return;
	}

	_channelModes += 'k';
	
	_password = args[3 + paramCount - 1];

	if (_channelParams.length() != 0) {
		_channelParams += " ";
	}
	_channelParams += args[3 + paramCount - 1];
}

void Channel::addLimitToChannel(std::vector<std::string> &args, Client &client, \
	size_t &paramCount) {
	//check if limit is positive integer
	int new_limit = std::stoi(args[3 + paramCount - 1]);
	if (new_limit <= 0) {
		//ERR_UNKNOWNMODE // is it specific enough?
		errUnknownMode(SERVER_NAME, client.getNickname(), 'l', this->getName());
		return;
	}

	_channelModes += 'l';
	
	_user_limit = new_limit;

	if (_channelParams.length() != 0) {
		_channelParams += " ";
	}
	_channelParams += args[3 + paramCount - 1];
}

void	Channel::removeChannelModes(std::vector<std::string> &args) {
	(void)args;
}