#pragma once

#include <iostream>
#include <vector>
#include "Client.hpp"

class Message {
	public:
		Message(void) = delete;
		Message(std::string buffer);
		Message(Message const & src) = delete;
		~Message(void) = default;

		Message & operator=(Message const & rhs) = delete;

		std::vector<std::string> ft_split(std::string & line, const char & sep);
		int parseBuffer(void);
		std::string& getCommand(void);
		std::string& getSender(void);
		std::vector<std::string>& getBufferDivided(void);

	private:
		std::string _buffer;
		std::vector<std::string> _buffer_divided;
		std::string _sender;
		std::string _receiver;
		// payload
		std::string _command;
		const std::vector<std::string> _function_names = \
			{"PASS", "NICK", "USER", "JOIN", "PART", "TOPIC", "INVITE", \
			"KICK", "QUIT", "MODE", "PRIVMSG"};
};