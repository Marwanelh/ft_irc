/* ========================================================================== */
/*  nick.cpp - NICK command handler                                           */
/*                                                                            */
/*  Sets or changes the client's nickname. Requires PASS to be sent first.    */
/*  Validates that the nickname is not empty and not already in use.           */
/* ========================================================================== */

#include "../Server.hpp"

void Server::cmdNick(Client &client, std::stringstream &ss)
{
	// PASS must be sent before NICK
	if (!client.getHasPass())
	{
		client.reply(ERR_NOTREGISTERED, ":You must send PASS first");
		return;
	}

	std::string nick;
	if (!(ss >> nick))
	{
		client.reply(ERR_NONICKNAMEGIVEN, ":No nickname given");
		return;
	}

	// Validate: nickname must start with a letter or special char, not a digit
	if (nick.empty() || nick[0] == '#' || nick[0] == ':')
	{
		client.reply(ERR_ERRONEUSNICKNAME, nick + " :Erroneous nickname");
		return;
	}

	// Check for uniqueness
	if (hasClient(nick))
	{
		client.reply(ERR_NICKNAMEINUSE, nick + " :Nickname is already in use");
		return;
	}

	client.setNickname(nick);
	client.reply(RPL_NEWNICK, "Nickname successfully changed to " + nick);
}
