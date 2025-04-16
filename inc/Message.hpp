#pragma once

#include <iostream>
#include <vector>
#include "commands.cpp"

class Message {
	public:
		Message(void) = delete;
		Message(std::string sender, std::string receiver, std::string command_called);
		Message(Message const & src) = delete;
		~Message(void) = default;

		Message & operator=(Message const & rhs) = delete;

		void callCommand(void) const;

	private:
		const std::string _sender;
		const std::string _receiver;
		// payload
		const std::string _command_called;
		const std::vector<void(*)(void)> _functions = {join, leave, kick, invite, topic, mode};
		const std::vector<std::string> _function_names = {"join", "leave", "kick", "invite", "topic", "mode"};
};