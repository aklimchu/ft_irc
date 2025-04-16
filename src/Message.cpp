#include "Message.hpp"

//--------------------------------Constructors--------------------------------//

Message::Message(std::string buffer) : _buffer(buffer), _sender("SenderX"), \
	_command("Unknown") {
}

//-------------------------------Member functions------------------------------//

std::vector<std::string> Message::ft_split(std::string & line, const char & sep)
{
    std::vector<std::string> v;
    size_t start;
    size_t end = 0;

    while ((start = line.find_first_not_of(sep, end)) != std::string::npos)
    {
        end = line.find(sep, start);
        v.push_back(line.substr(start, end - start));
    }
    return v;
}

int Message::parseBuffer(void) {

	if (std::size(_buffer) == 0) // \r, \n symbols?
		return -1;
	_buffer_divided = ft_split(_buffer, ' '); // do we need to explicitly delete?
	for (size_t i = 0; i < _function_names.size(); i++) {
		if (this->_buffer_divided[0] == this->_function_names[i]) // how about MODE + flag?
			this->_command = this->_function_names[i];
	}
	return 0;
}

std::string & Message::getCommand(void) {
	return(this->_command);
}

std::string & Message::getSender(void) {
	return(this->_sender);
}

std::vector<std::string> & Message::getBufferDivided(void) {
	return(this->_buffer_divided);
}