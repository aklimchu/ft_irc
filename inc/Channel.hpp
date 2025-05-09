#pragma once

#include <iostream>
#include <vector>
#include "Client.hpp"
#include "server_replies.hpp"

#include <string>
#include <set>
#include <map>
#include <algorithm> // for find_if
#include <sys/socket.h> // send()

#define SERVER_NAME "ircserv"

class Channel {
	public:
		Channel(void);
		Channel(const std::string &name);
		Channel(Channel const & src) = delete;
		~Channel(void) = default;

		Channel & operator=(Channel const & rhs);

		const std::string			&getName() const;
		void						addUser(Client *client);
		void						removeUser(Client *client);
		void						removeOperator(Client* client);
		bool						isUser(Client *client) const;
		bool						isOperator(Client *client) const;
		const std::set<Client *>	&getUsers() const;
		const std::string			getChannelModes() const;
		const std::string			getChannelParams() const;
		const std::set<Client *>	&getInvitedUsers() const;
		const std::set<Client *>	&getOperators() const;
		const std::string			&getTopic() const;
		const std::string			&getPassword() const;
		size_t						getUserLimit() const;
		void						setTopic(const std::string &str);
		void						setAsOperator(Client *client);
		void						addInvite(Client *client);
		void						removeInvite(Client *client);
		std::string					addChannelModes(std::vector<std::string> &args, \
										std::map<int, Client> &serverUsers, Client &client);
		std::string					removeChannelModes(std::vector<std::string> &args, \
										std::map<int, Client> &serverUsers, Client &client);
		void						addITMode(const char & mode, std::string & successfulChangesMode);
		int							addOperatorToChannel(std::vector<std::string> &args, \
										std::map<int, Client> &serverUsers, \
										Client &client, size_t &paramCount);
		int							addKeyToChannel(std::vector<std::string> &args, Client &client, \
										size_t &paramCount);
		int							addLimitToChannel(std::vector<std::string> &args, Client &client, \
										size_t &paramCount);
		void						removeITMode(const char & mode, std::string & successfulChangesMode);
		int							removeOperatorFromChannel(std::vector<std::string> &args, \
										std::map<int, Client> &serverUsers, Client &client, size_t &paramCount);
		void						removeKeyFromChannel(void);
		void						removeLimitFromChannel(void);
		int							channelSendToClient(int fd, const std::string &msg);
		int							findParamIndex(char mode);
		void						removeFromChannelParams(int paramIndex);
		std::vector<std::string>	ft_split(std::string & line, const char & sep);
		
		private:
			std::set<Client*>			_invitedUsers;
			std::string					_name;
			std::set<Client *>			_users;
			std::string					_channelModes;
			std::string					_channelParams;
			std::set<Client*>			_operators;
			size_t						_user_limit;
			std::string					_password;
			std::string					_topic;
};
