#include "Client.hpp"

Client::Client() : _fd(-1), _hostname("localhost"), _registered(false), 
	_usernameOK(false), _nicknameOK(false), _passwdOK(false), _hasQuit(false)
{
}

Client::Client(int fd) : _fd(fd), _hostname("localhost"), _registered(false), \
	_usernameOK(false), _nicknameOK(false), _passwdOK(false), _hasQuit(false)
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

std::string	&Client::getBuffer()
{
	return (this->_buffer);
}

const std::string	&Client::getUsermodes() const
{
	return (this->_usermodes);
}

const std::string	&Client::getHostname() const
{
	return (this->_hostname);
}

bool	Client::isRegistered() const
{
	return (this->_registered);
}

bool	Client::isPasswdOK() const
{
	return (this->_passwdOK);
}

bool	Client::isUsernameOK(void)
{
	return (this->_usernameOK);
}

bool	Client::isNicknameOK(void)
{
	return (this->_nicknameOK);
}

bool	Client::getHasQuit(void)
{
	return (this->_hasQuit);
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

void	Client::setHostname(const std::string &str)
{
	this->_hostname = str;
}

void	Client::appendBuffer(const std::string &str)
{
	this->_buffer += str;
}

void	Client::addUsermode(char mode)
{
	if (this->_usermodes.find(mode) == std::string::npos)
		this->_usermodes += mode;
}

void	Client::removeUsermode(char mode)
{
	size_t	pos = this->_usermodes.find(mode);

	if (pos != std::string::npos)
		this->_usermodes.erase(pos, 1);
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

void	Client::setUsernameOK(bool value)
{
	this->_usernameOK = value;
}

void	Client::setNicknameOK(bool value)
{
	this->_nicknameOK = value;
}

void	Client::setHasQuit(bool value)
{
	this->_hasQuit = value;
}

void	Client::joinChannel(const std::string &channelName)
{
	this->_channels.insert(channelName);
}

void	Client::leaveChannel(const std::string &channelName)
{
	this->_channels.erase(channelName);
}

const	std::set<std::string>	&Client::getChannels() const
{
	return (this->_channels);
}
