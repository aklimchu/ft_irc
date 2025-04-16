#include "Message.hpp"

//--------------------------------Constructors--------------------------------//

Message::Message(std::string buffer) : _buffer(buffer) {}

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
	std::string nptr = NULL;

	if (_buffer == nptr || std::size(_buffer) == 0) // \r, \n symbols?
		return -1;
	_buffer_divided = ft_split(_buffer, ' '); // do we need to explicitly delete?
	int i;
	for (i = 0; i < _functions.size(); i++) {
		if (this->_buffer_divided[0] == this->_function_names[i]) // how about MODE + flag?
			this->_command_called = this->_function_names[i];
	}
	if (i == _functions.size())
		return -1;
	return 0;
}

void Message::callCommand(void) {
	//checking user permissions before executing the command?
	std::string nptr = NULL;
	
	if (this->_command_called == nptr)
		return;
	for (int i = 0; i < _functions.size(); i++) {
		if (this->_command_called == this->_function_names[i]) // how about MODE + flag?
			this->_functions[i](_buffer_divided, _sender);
	}
}