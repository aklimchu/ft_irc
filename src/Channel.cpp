#include "Channel.hpp"

//--------------------------------Constructors--------------------------------//

Channel::Channel(): _channelModes(""), _channelParams("") {}
Channel::Channel(const std::string &name) : _name(name), _channelModes(""), _channelParams("")
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
	std::map<int, Client> &serverUsers) {
	size_t paramLimit = args.size() - 3;
	if (paramLimit > 3) 
		paramLimit = 3; // max 3 modes with optional parameters accepted
	std::string	&mode = args[2];
	size_t paramCount = 0;

	for (size_t i = 1; i < mode.size(); i++)
	{
		if (mode[i] == 'i' || mode[i] == 't') {
			_channelModes += mode[i];
		}

		if (mode[i] == 'k' || mode[i] == 'l') {
		// consider ERR_NEEDMOREPARAMS
			_channelModes += mode[i];
		}

		// +o is user-specific and is not part of the channel’s mode string
		if (mode[i] == 'o') {
			// check if enough parameters
			if (args.size() < 3 + paramCount + 1) {
				// ERR_NEEDMOREPARAMS
				return;
			}
			
			// check if user exists on the server
			bool found = false;
			for (const auto& pair : serverUsers) {
				if (pair.second.getNickname() == args[3 + paramCount - 1]) {
					found = true;
					break;
				}
			}
			if (!found) {
				// ERR_NOSUCHNICK
				return;
   			}

			paramCount++;
			found = false;
			if (paramCount <= paramLimit) {
				for (const auto& result : _users) {
					if (result->getNickname() == args[3 + paramCount - 1]) {
						Client & client = *result;
						_operators.insert(&client); // should we also set privileges to client?
						found = true;
						break;
					}
				}
				if (!found) {
					// ERR_USERNOTINCHANNEL
					return;
				}
			}
		}	
	}
}

void	Channel::removeChannelModes(std::vector<std::string> &args) {
	(void)args;
}