#pragma once

#include <vector>
#include <iostream>

void pass(std::vector<std::string> & _buffer_divided, std::string & _sender);

void nick(std::vector<std::string> & _buffer_divided, std::string & _sender);

void user(std::vector<std::string> & _buffer_divided, std::string & _sender);

void join(std::vector<std::string> & _buffer_divided, std::string & _sender);

void part(std::vector<std::string> & _buffer_divided, std::string & _sender);

void topic(std::vector<std::string> & _buffer_divided, std::string & _sender);

void invite(std::vector<std::string> & _buffer_divided, std::string & _sender);

void kick(std::vector<std::string> & _buffer_divided, std::string & _sender);

void quit(std::vector<std::string> & _buffer_divided, std::string & _sender);

void mode(std::vector<std::string> & _buffer_divided, std::string & _sender);

void privmsg(std::vector<std::string> & _buffer_divided, std::string & _sender);

void leave(std::vector<std::string> & _buffer_divided, std::string & _sender);