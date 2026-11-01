/* ========================================================================== */
/*  privmsg.cpp - PRIVMSG command handler                                     */
/*                                                                            */
/*  Syntax: PRIVMSG <target> :<message>                                       */
/*  Sends a message to a user or channel. The target can be a nickname        */
/*  (direct message) or a channel name starting with '#'.                     */
/* ========================================================================== */

#include "../Server.hpp"

void Server::cmdPrivmsg(Client &client, const std::string &line, std::stringstream &ss)
{
	// Must be registered to send messages
	if (!client.getIsRegistered())
	{
		client.reply(ERR_NOTREGISTERED, ":You have not registered");
		return;
	}

	// Parse the target (nick or #channel)
	std::string target;
	if (!(ss >> target))
	{
		client.reply(ERR_NORECIPIENT, ":No recipient given (PRIVMSG)");
		return;
	}

	// Extract the message text (everything after the first ':')
	size_t colonPos = line.find(":");
	if (colonPos == std::string::npos)
	{
		client.reply(ERR_NOTEXTTOSEND, ":No text to send");
		return;
	}
	std::string message = line.substr(colonPos);   // Keeps the ':' for protocol

	// Try to find the target as a client or a channel
	Client  *targetClient  = getClientByNickname(target);
	Channel *targetChannel = getChannelByName(target);

	if (targetClient)
	{
		// Direct message: relay with the sender's prefix
		std::string fullMsg = client.getPrefix() + " PRIVMSG "
							  + target + " " + message + "\r\n";
		send(targetClient->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
	}
	else if (targetChannel)
	{
		// Channel message: sender must be a member
		if (!targetChannel->isMember(&client))
		{
			client.reply(ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
			return;
		}
		// Relay to all channel members except the sender
		targetChannel->broadcastMessage(message, &client);
	}
	else
	{
		client.reply(ERR_NOSUCHNICK, target + " :No such nick/channel");
	}
}
