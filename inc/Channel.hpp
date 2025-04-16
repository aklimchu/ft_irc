#pragma once

#include <iostream>
#include <vector>
#include <Client.hpp>

class Channel {
	public:
		Channel(void);
		Channel(Channel const & src) = delete;
		~Channel(void) = default;

		Channel & operator=(Channel const & rhs) = delete;

		
		private:
			/* std::vector<Client*>		_operator;
			std::vector<Client*>		_users;
			std::vector<Client*>		_invite;
			std::vector<Client*>		_voice;
			size_t						_user_limit;
			std::string					_password; */
};