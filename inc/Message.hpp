#pragma once

#include <iostream>
#include <vector>
#include "Client.hpp"
#include "commands.hpp"

class Message {
	public:
		Message(void) = delete;
		Message(std::string buffer);
		Message(Message const & src) = delete;
		~Message(void) = default;

		Message & operator=(Message const & rhs) = delete;

		std::vector<std::string> ft_split(std::string & line, const char & sep);
		int parseBuffer(void);
		void callCommand(void);

	private:
		std::string _buffer;
		std::vector<std::string> _buffer_divided;
		std::string _sender;
		std::string _receiver;
		// payload
		std::string _command_called;
		const std::vector<void(*)(std::vector<std::string> &, std::string &)> _functions = \
			{pass, nick, user, join, part, topic, invite, kick, quit, mode, privmsg, leave};
		const std::vector<std::string> _function_names = \
			{"PASS", "NICK", "USER", "JOIN", "PART", "TOPIC", "INVITE", \
			"KICK", "QUIT", "MODE", "PRIVMSG", "LEAVE"};
};