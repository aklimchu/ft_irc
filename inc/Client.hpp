#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>


class	Client
{
	private:
		int			_fd;
		std::string	_username;
		std::string	_nickname;
		std::string	_realname;
		std::string _buffer;
		bool		_registered;
		bool		_usernameOK;
		bool		_nicknameOK;
		bool		_passwdOK;

	
	public:
		Client();
		Client(int fd);
		~Client();

		int					getFd() const;
		const std::string	&getUsername() const;
		const std::string	&getNickname() const;
		const std::string	&getRealname() const;
		/*const*/ std::string	&getBuffer() /*const*/;
		bool				isRegistered() const;
		bool				isPasswdOK() const;
		bool				isUsernameOK(void);
		bool				isNicknameOK(void);

		void	setUsername(const std::string &str);
		void	setNickname(const std::string &str);
		void	setRealname(const std::string &str);
		void	setBuffer(const std::string &str);
		void	appendBuffer(const std::string &str);
		void	clearBuffer();
		void	setRegistered(bool value);
		void	setPasswdOK(bool value);
		void	setUsernameOK(bool value);
		void	setNicknameOK(bool value);
};

#endif
