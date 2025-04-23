#pragma once

#include <iostream>

// GENERAL

// 001 RPL_WELCOME
inline std::string	rplWelcome(const std::string &server, const std::string &nick)
{
	return (":" + server + " 001 " + nick + " :Welcome to the IRC Network, " + nick + "\r\n");
}

// 002 RPL_YOURHOST
inline std::string	rplYourHost(const std::string &server, const std::string &nick)
{
	return (":" + server + " 002 " + nick + " :Your host is " + server + ", running version 1.0\r\n");
}

// 003 RPL_CREATED
inline std::string	rplCreated(const std::string &server, const std::string &nick)
{
	return (":" + server + " 003 " + nick + " :This server was created 1.1.2000\r\n");
}

// 004 RPL_MYINFO
inline std::string	rplMyInfo(const std::string &server, const std::string &nick,
	const std::string &userModes, const std::string &channelModes)
{
	return (":" + server + " 004 " + nick + " " + server + " 1.0 " + userModes + " " + channelModes + "\r\n");
}

// 311 RPL_WHOISUSER
inline std::string	rplWhoisUser(const std::string &server, const std::string &requester,
	const std::string &nick, const std::string &user, const std::string &host, const std::string &realname)
{
	return (":" + server + " 311 " + requester + " " + nick + " " + user + " " + host + " * :" + realname + "\r\n");
}

// 312 RPL_WHOISSERVER
inline std::string	rplWhoisServer(const std::string &server, const std::string &requester,
	const std::string &nick)
{
	return (":" + server + " 312 " + requester + " " + nick + " " + server + " :ft_irc server\r\n");
}

// 318 RPL_ENDOFWHOIS
inline std::string	rplEndOfWhois(const std::string &server, const std::string &requester,
	const std::string &nick)
{
	return (":" + server + " 318 " + requester + " " + nick + " :End of WHOIS list\r\n");
}

// 319 RPL_WHOISCHANNELS
inline std::string	rplWhoisChannels(const std::string &server, const std::string &requester,
	const std::string &nick, const std::string &channels)
{
	return (":" + server + " 319 " + requester + " " + nick + " :" + channels + "\r\n");
}

// 353 RPL_NAMREPLY
inline std::string	rplNamReply(const std::string &server, const std::string &nick,
	const std::string &channel, const std::string &namesList)
{
	return (":" + server + " 353 " + nick + " = " + channel + " :" + namesList + "\r\n");
}

// 366 RPL_ENDOFNAMES
inline std::string	rplEndOfNames(const std::string &server, const std::string &nick,
	const std::string &channel)
{
	return (":" + server + " 366 " + nick + " " + channel + " :End of /NAMES list.\r\n");
}

// 431 ERR_NONICKNAMEGIVEN
inline std::string	errNoNicknameGiven(const std::string &server, const std::string &nick)
{
	return (":" + server + " 431 " + nick + " :No nickname given\r\n");
}

// 433 ERR_NICKNAMEINUSE
inline std::string	errNicknameInUse(const std::string &server,
	const std::string &senderNick, const std::string &attemptedNick)
{
	return (":" + server + " 433 " + senderNick + " " + attemptedNick + " :Nickname is already in use\r\n");
}


// 461 ERR_NEEDMOREPARAMS
inline std::string errNeedMoreParams(const std::string& server, \
	const std::string& nick, const std::string& command) {
		return ":" + server + " 461 " + nick + " " + command + " :Not enough parameters\r\n";
}

// 462 ERR_ALREADYREGISTRED
inline std::string	errAlreadyRegistered(const std::string &server, const std::string &nick)
{
	return (":" + server + " 462 " + nick + " :You may not reregister\r\n");
}

// 464 ERR_PASSWDMISMATCH
inline std::string	errPasswdMismatch(const std::string &server)
{
	return (":" + server + " 464 * :Password incorrect\r\n");
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
