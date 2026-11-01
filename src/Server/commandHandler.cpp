/* ========================================================================== */
/*  commandHandler.cpp - Command parsing and dispatch                         */
/*                                                                            */
/*  Parses the first word of each line as a command, converts to uppercase,   */
/*  then dispatches to the appropriate handler via a switch statement.         */
/*  Also performs the final registration check after each command.             */
/* ========================================================================== */

#include "Server.hpp"

/* -------------------------------------------------------------------------- */
/*  Command String → Enum Mapping                                             */
/* -------------------------------------------------------------------------- */

int Server::parseCommand(const std::string &cmd) const
{
	if (cmd == "CAP")     return CMD_CAP;
	if (cmd == "PASS")    return CMD_PASS;
	if (cmd == "NICK")    return CMD_NICK;
	if (cmd == "USER")    return CMD_USER;
	if (cmd == "PRIVMSG") return CMD_PRIVMSG;
	if (cmd == "PING")    return CMD_PING;
	if (cmd == "MODE")    return CMD_MODE;
	if (cmd == "JOIN")    return CMD_JOIN;
	if (cmd == "INVITE")  return CMD_INVITE;
	if (cmd == "KICK")    return CMD_KICK;
	if (cmd == "TOPIC")   return CMD_TOPIC;
	if (cmd == "QUIT")    return CMD_QUIT;
	return CMD_UNKNOWN;
}

/* -------------------------------------------------------------------------- */
/*  Command Dispatch                                                          */
/* -------------------------------------------------------------------------- */

void Server::processCommand(Client &client, const std::string &line)
{
	std::stringstream ss(line);
	std::string command;
	ss >> command;

	// Convert command to uppercase for case-insensitive matching
	for (size_t i = 0; i < command.length(); i++)
		command[i] = toupper(command[i]);

	if (command.empty())
		return;

	switch (parseCommand(command))
	{
		case CMD_CAP:
			cmdCap(client);
			break;
		case CMD_PASS:
			cmdPass(client, ss);
			break;
		case CMD_NICK:
			cmdNick(client, ss);
			break;
		case CMD_USER:
			cmdUser(client, ss);
			break;
		case CMD_PRIVMSG:
			cmdPrivmsg(client, line, ss);
			break;
		case CMD_PING:
			client.reply("PONG", ":pong");
			break;
		case CMD_MODE:
			cmdMode(client, ss);
			break;
		case CMD_JOIN:
			cmdJoin(client, ss);
			break;
		case CMD_INVITE:
			cmdInvite(client, ss);
			break;
		case CMD_KICK:
			cmdKick(client, ss);
			break;
		case CMD_TOPIC:
			cmdTopic(client, ss);
			break;
		case CMD_QUIT:
			cmdQuit(client, ss);
			break;
		default:
			client.reply(ERR_UNKNOWNCOMMAND, command + " :Unknown command");
			break;
	}

	// Registration check: a client is fully registered once PASS + NICK + USER are done
	if (!client.getIsRegistered()
		&& client.getHasPass()
		&& !client.getNickname().empty()
		&& !client.getUsername().empty())
	{
		client.setIsRegistered(true);
		client.reply(RPL_WELCOME, ":Welcome to the Internet Relay Network " + client.getNickname());
		std::cout << "[LOG] Client '" << client.getNickname()
				  << "' (FD " << client.getFd() << ") registered successfully." << std::endl;
	}
}
