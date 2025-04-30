#pragma once

#include <iostream>
#include <vector>
#include <map>
#include "Client.hpp"
#include "Channel.hpp"
#include "server_replies.hpp"

#define SERVER_NAME "ircserv"

class Message {
	public:
		Message(void) = delete;
		Message(std::string buffer, std::map<int, Client> &clients_map); // Changed to take a const reference
		Message(Message const & src) = delete;
		~Message(void) = default;

		Message & operator=(Message const & rhs) = delete;
		
		class NoSuchNick : public std::exception {
			public:
			virtual const char* what() const throw() {
				return ("");
			}
		};
		
		class NoSuchChannel : public std::exception {
			public:
			virtual const char* what() const throw() {
				return ("");
			}
		};

		std::vector<std::string>	ft_split(std::string & line, const char & sep);
		int							parseBuffer(void);
		const std::string			&getCommand(void) const;
		const std::string			&getSender(void) const;
		Client						&getReceiverClient(void);
		Channel						&getReceiverChannel(std::map<std::string, Channel> &_channels);
		std::vector<std::string>	&getBufferDivided(void);
		void						setSender(const std::string &sender);

	private:
		std::string						_buffer;
		std::vector<std::string>		_buffer_divided;
		std::string						_sender;
		std::string						_command;
		std::map<int, Client> 			&_clients_map;
		const std::vector<std::string>	_function_names = \
			{"PASS", "NICK", "USER", "JOIN", "PART", "TOPIC", "INVITE", \
			"KICK", "QUIT", "MODE", "PRIVMSG", "PING", "CAP", "WHOIS", "WHO"};

};
