#include "Channel.hpp"

//--------------------------------Constructors--------------------------------//

Channel::Channel() {}
Channel::Channel(const std::string &name) : _name(name)
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

const std::set<Client *>	&Channel::getUsers() const
{
	return (_users);
}
