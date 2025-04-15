#include "Message.hpp"

//--------------------------------Constructors--------------------------------//

Message::Message(std::string sender, std::string receiver, std::string command_called) : \
	sender(sender), receiver(receiver), command_called(command_called) {}

//-------------------------------Member functions------------------------------//

void Message::callCommand(void) const {
	//checking user permissions before executing the command?
	for (int i = 0; i < sizeof(this->functions); i++) {
		if (this->command_called == this->function_names[i]) // how about MODE + flag?
			this->functions[i]();
	}
}