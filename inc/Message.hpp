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
		const std::string sender;
		const std::string receiver;
		// payload
		const std::string command_called;
		const std::vector<void(*)(void)> functions = {join, leave, kick, invite, topic, mode};
		const std::vector<std::string> function_names = {"join", "leave", "kick", "invite", "topic", "mode"};
};