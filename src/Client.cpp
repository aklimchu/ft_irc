#include "Client.hpp"

Client::Client() : _fd(-1), _registered(false), _passwdOK(false)
{
}

Client::Client(int fd) : _fd(fd), _registered(false), _passwdOK(false)
{
}

Client::~Client()
{
}

//GETTERS:
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

const std::string	&Client::getRealname() const
{
	return (this->_realname);
}

const std::string	&Client::getBuffer() const
{
	return (this->_buffer);
}

bool	Client::isRegistered() const
{
	return (this->_registered);
}

bool	Client::isPasswdOK() const
{
	return (this->_passwdOK);
}

//SETTERS:
void	Client::setUsername(const std::string &str)
{
	this->_username = str;
}

void	Client::setNickname(const std::string &str)
{
	this->_nickname = str;
}

void	Client::setRealname(const std::string &str)
{
	this->_realname = str;
}

void	Client::setBuffer(const std::string &str)
{
	this->_buffer = str;
}

void	Client::appendBuffer(const std::string &str)
{
	this->_buffer += str;
}

void	Client::clearBuffer()
{
	this->_buffer.clear();
}

void	Client::setRegistered(bool value)
{
	this->_registered = value;
}

void	Client::setPasswdOK(bool value)
{
	this->_passwdOK = value;
}
