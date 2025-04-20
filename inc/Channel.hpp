#pragma once

#include <iostream>
#include <vector>
#include <Client.hpp>

#include <string>
#include <set>

class Channel {
	public:
		Channel(void);
		Channel(const std::string &name);
		Channel(Channel const & src) = delete;
		~Channel(void) = default;

		//Channel & operator=(Channel const & rhs) = delete;

		const std::string			&getName() const;
		void						addUser(Client *client);
		void						removeUser(Client *client);
		bool						isUser(Client *client) const;
		const std::set<Client *>	&getUsers() const;

		
		private:
			/* std::vector<Client*>		_operator;
			std::vector<Client*>		_users;
			std::vector<Client*>		_invite;
			std::vector<Client*>		_voice;
			size_t						_user_limit;
			std::string					_password; */
			std::string					_name;
			std::set<Client *>			_users;
};
