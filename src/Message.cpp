#include "Message.hpp"

//--------------------------------Constructors--------------------------------//

Message::Message(std::string buffer, const std::map<int, Client>	&clients_map) : _buffer(buffer), _sender("SenderX"), \
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
	_buffer_divided = ft_split(_buffer, ' '); // do we need to explicitly delete?
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

Client & Message::getReceiverClient(void) {
	return(this->_receiver_client);
}

Channel & Message::getReceiverChannel(void) {
	return(this->_receiver_channel);
}

std::vector<std::string> & Message::getBufferDivided(void) {
	return(this->_buffer_divided);
}

void	Message::setSender(const std::string &sender)
{
	this->_sender = sender;
}

void	Message::setReceiverClient(void)
{
	std::pair<int, Client> found_pair;
	bool found = false;

	for (const auto& pair : _clients_map) {
		if (pair.second.getNickname() == _buffer_divided[1]) {
			found_pair = pair;
			found = true;
			break;
		}
	}

	if (found) {
		this->_receiver_client = found_pair.second;
    }
	else {
		throw Message::NoSuchNick();
	}

}

void	Message::setReceiverChannel(std::map<std::string, Channel>	& _channels) {
	std::pair<std::string, Channel> found_pair;
	bool found = false;
	
	for (auto& pair : _channels) {
		std::cout << "Channel name: "<< pair.first << std::endl;
	}

	for (auto& pair : _channels) {
		if (pair.first == _buffer_divided[1]) {
			found_pair = pair;
			found = true;
			break;
		}
	}

	if (found) {
		this->_receiver_channel = found_pair.second;
    }
	else {
		throw Message::NoSuchNick();
	}
}