#pragma once

#include <iostream>

// GENERAL

// 461 ERR_NEEDMOREPARAMS
inline std::string errNeedMoreParams(const std::string& server, \
	const std::string& nick, const std::string& command) {
		return ":" + server + " 461 " + nick + " " + command + " :Not enough parameters\r\n";
}


// PRIVMSG

//401    ERR_NOSUCHNICK
inline std::string errNoSuchNick(const std::string& server, \
	const std::string& nick, const std::string& receiver) {
		return ":" + server + " 401 " + nick + " " + receiver + " :No such nick/channel\r\n";
}

//411	ERR_NORECIPIENT
inline std::string errNoRecipient(const std::string& server, \
	const std::string& nick, const std::string& command) {
		return ":" + server + " 401 " + nick + " " + command + " :No recipient given\r\n";
}

//412 ERR_NOTEXTTOSEND
inline std::string errNoTextToSend(const std::string& server, \
	const std::string& nick) {
		return ":" + server + " 412 " + nick + " :No text to send\r\n";
}

//413 ERR_NOTOPLEVEL
inline std::string errNoTopLevel(const std::string& server, \
	const std::string& nick, const std::string& mask) {
		return ":" + server + " 413 " + nick + " " + mask + " :No toplevel domain specified\r\n";
}

//407 ERR_TOOMANYTARGETS
inline std::string errTooManyTargets(const std::string& server, \
	const std::string& nick, const std::string& message) {
		return ":" + server + " 407 " + nick + " :" + message + "\r\n";
}

//404 ERR_CANNOTSENDTOCHAN
inline std::string errCannotSendToChan(const std::string& server, \
	const std::string& nick, const std::string& channel) {
		return ":" + server + " 404 " + nick + " " + channel + " :Cannot send to channel\r\n";

}

//414 ERR_WILDTOPLEVEL
inline std::string errWildTopLevel(const std::string& server, \
	const std::string& nick, const std::string& mask) {
		return ":" + server + " 414 " + nick + " " + mask + " :Wildcard in toplevel domain\r\n";

}

//301 RPL_AWAY
inline std::string rplAway(const std::string& server, const std::string& nick, \
	const std::string& receiver, const std::string& away_autoreply) {
		return ":" + server + " 301 " + nick + " " + receiver + " :" + \
			away_autoreply + "\r\n";
}

//501 ERR_UMODEUNKNOWNFLAG
inline std::string errUModeUnknownFlag(const std::string& server, const std::string& nick) {
		return ":" + server + " 501 " + nick + " :Unknown MODE flag\r\n";
}

//502 ERR_USERSDONTMATCH
inline std::string errUsersDontMatch(const std::string& server, const std::string& nick) {
	return ":" + server + " 502 " + nick + " :Cannot change mode for other users\r\n";
}

//221 RPL_UMODEIS
inline std::string rplUModeIs(const std::string& server, const std::string& nick, \
	const std::string& userMode) {
	return ":" + server + " 221 " + nick + " " + userMode + "\r\n";
}