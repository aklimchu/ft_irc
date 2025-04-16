#pragma once

#include <iostream>
#include <vector>
//#include <Client.hpp>

class Channel {
	public:
		Channel(void);
		Channel(Channel const & src) = delete;
		~Channel(void) = default;

		Channel & operator=(Channel const & rhs) = delete;

		
		private:
			// std::vector<Client*>		operator;
			// std::vector<Client*>		users;
			// std::vector<Client*>		invite;
			// std::vector<Client*>		voice;
			size_t						_user_limit;
			std::string					_password;
};