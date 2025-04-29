#include "Message.hpp"

//--------------------------------Constructors--------------------------------//

Message::Message(std::string buffer, std::map<int, Client>	&clients_map) : _buffer(buffer), _sender("SenderX"), \
	_command("Unknown"), _clients_map(clients_map) {
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

	if (std::size(_buffer) == 0)
		return -1;
	_buffer_divided = ft_split(_buffer, ' ');
	for (size_t i = 0; i < _function_names.size(); i++) {
		if (this->_buffer_divided[0] == this->_function_names[i])
			this->_command = this->_function_names[i];
	}
	return 0;
}

const std::string & Message::getCommand(void) const {
	return(this->_command);
}

const std::string & Message::getSender(void) const {
	return(this->_sender);
}

std::vector<std::string> & Message::getBufferDivided(void) {
	return(this->_buffer_divided);
}

void	Message::setSender(const std::string &sender)
{
	this->_sender = sender;
}

Client & Message::getReceiverClient(void)
{
	for (auto &pair : _clients_map) {
        if (pair.second.getNickname() == _buffer_divided[1]) {
            return pair.second;
        }
    }

    throw Message::NoSuchNick();
}

Channel & Message::getReceiverChannel(std::map<std::string, Channel> & _channels) {
	auto it = _channels.find(_buffer_divided[1]);
    if (it != _channels.end()) {
        return it->second;
    }
	throw Message::NoSuchChannel();
}