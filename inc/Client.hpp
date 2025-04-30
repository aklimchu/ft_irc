#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <set>


class	Client
{
	private:
		int			_fd;
		std::string	_username;
		std::string	_nickname;
		std::string	_realname;
		std::string _buffer;
		std::string	_usermodes;
		std::string	_hostname;
		bool		_registered;
		bool		_usernameOK;
		bool		_nicknameOK;
		bool		_passwdOK;
		bool		_hasQuit;

		std::set<std::string>	_channels;

	
	public:
		Client();
		Client(int fd);
		~Client();

		int					getFd() const;
		const std::string	&getUsername() const;
		const std::string	&getNickname() const;
		const std::string	&getRealname() const;
		std::string			&getBuffer();
		const std::string	&getUsermodes() const;
		const std::string	&getHostname() const;
		bool				isRegistered() const;
		bool				isPasswdOK() const;
		bool				isUsernameOK(void);
		bool				isNicknameOK(void);
		bool				getHasQuit(void);
		void				setUsername(const std::string &str);
		void				setNickname(const std::string &str);
		void				setRealname(const std::string &str);
		void				setBuffer(const std::string &str);
		void				addUsermode(char mode);
		void				removeUsermode(char mode);
		void				setHostname(const std::string &str);
		void				appendBuffer(const std::string &str);
		void				clearBuffer();
		void				setRegistered(bool value);
		void				setPasswdOK(bool value);
		void				setUsernameOK(bool value);
		void				setNicknameOK(bool value);
		void				setHasQuit(bool value);
		void				joinChannel(const std::string &channelName);
		void				leaveChannel(const std::string &channelName);

		const	std::set<std::string>	&getChannels() const;
};

#endif
