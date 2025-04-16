#include "Message.hpp"

//--------------------------------Constructors--------------------------------//

Message::Message(std::string sender, std::string receiver, std::string command_called) : \
	_sender(sender), _receiver(receiver), _command_called(command_called) {}

//-------------------------------Member functions------------------------------//

void Message::callCommand(void) const {
	//checking user permissions before executing the command?
	for (int i = 0; i < sizeof(this->_functions); i++) {
		if (this->_command_called == this->_function_names[i]) // how about MODE + flag?
			this->_functions[i]();
	}
}