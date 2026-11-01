/* ========================================================================== */
/*  pass.cpp - PASS command handler                                           */
/*                                                                            */
/*  The PASS command must be sent before NICK and USER. If the password       */
/*  matches, the client's _hasPass flag is set. If it doesn't match,          */
/*  the client is disconnected.                                               */
/* ========================================================================== */

#include "../Server.hpp"

void Server::cmdPass(Client &client, std::stringstream &ss)
{
	// Already registered clients cannot re-authenticate
	if (client.getIsRegistered())
	{
		client.reply(ERR_ALREADYREGISTRED, ":You may not reregister");
		return;
	}

	std::string pass;
	if (!(ss >> pass))
	{
		client.reply(ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
		return;
	}

	if (pass == _password)
	{
		client.setHasPass(true);
		client.reply(RPL_NOTICE, "Step 1/3: Password accepted. Now send NICK <nickname>.");
	}
	else
	{
		client.reply(ERR_PASSWDMISMATCH, ":Password incorrect. Disconnecting.");
		client.setWasDisconnected(true);
	}
}
