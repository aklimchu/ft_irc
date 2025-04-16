#include "Client.hpp"

Client::Client()
{
}

Client::Client(int fd) : _fd(fd)
{
}

Client::~Client()
{
}

int	Client::getFd() const
{
	return (this->_fd);
}

const std::string	&Client::getUsername() const
{
	return (this->_username);
}

const std::string	&Client::getNickname() const
{
	return (this->_nickname);
}
