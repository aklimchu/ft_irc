#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>

class	Client
{
	private:
		int			_fd;
		std::string	_username;
		std::string	_nickname;
	
	public:
		Client();
		Client(int fd);
		~Client();

};

#endif
