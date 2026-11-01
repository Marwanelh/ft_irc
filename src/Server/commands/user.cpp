/* ========================================================================== */
/*  user.cpp - USER command handler                                           */
/*                                                                            */
/*  Syntax: USER <username> <hostname> <servername> :<realname>                */
/*  Sets the client's username and realname. PASS must be sent first.         */
/*  Cannot be used after registration is complete.                            */
/* ========================================================================== */

#include "../Server.hpp"

void Server::cmdUser(Client &client, std::stringstream &ss)
{
	// PASS must be sent before USER
	if (!client.getHasPass())
	{
		client.reply(ERR_NOTREGISTERED, ":You must send PASS first");
		return;
	}

	// Already registered clients cannot re-register
	if (client.getIsRegistered())
	{
		client.reply(ERR_ALREADYREGISTRED, ":You may not reregister");
		return;
	}

	// Parse the four parameters
	std::string user, host, server;
	if (!(ss >> user >> host >> server))
	{
		client.reply(ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
		return;
	}

	// Extract realname: everything after the first ':'
	size_t colonPos = ss.str().find(":");
	if (colonPos == std::string::npos)
	{
		client.reply(ERR_NEEDMOREPARAMS, "USER :Real name must be prefixed with ':'");
		return;
	}

	std::string realname = ss.str().substr(colonPos + 1);
	client.setUsername(user);
	client.setRealname(realname);
	client.reply(RPL_NOTICE, "Step 3/3: User info saved.");
}
