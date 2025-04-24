#pragma once

#include <iostream>
#include <vector>
#include <Client.hpp>
#include <server_replies.hpp>

#include <string>
#include <set>
#include <map>

#define SERVER_NAME "ircserv"

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
		const std::string			getChannelModes() const;

		void addChannelModes(std::vector<std::string> &args, \
			std::map<int, Client> &serverUsers, Client &client);
		void addOperatorToChannel(std::vector<std::string> &args, \
			std::map<int, Client> &serverUsers, Client &client, size_t &paramCount);
		void addKeyToChannel(std::vector<std::string> &args, Client &client, \
			size_t &paramCount);
		void addLimitToChannel(std::vector<std::string> &args, Client &client, \
			size_t &paramCount);
		void						removeChannelModes(std::vector<std::string> &args);
		
		private:
			/*std::set<Client*>			_invite;
			std::set<Client*>			_voice; */
			std::string					_name;
			std::set<Client *>			_users;
			std::string					_channelModes;
			std::string					_channelParams;
			std::set<Client*>			_operators;
			size_t						_user_limit;
			std::string					_password;
};
