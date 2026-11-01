/* ========================================================================== */
/*  kick.cpp - KICK command handler                                           */
/*                                                                            */
/*  Syntax: KICK <#channel> <nickname> [:<reason>]                            */
/*  Only channel operators can kick. The kicked user is removed from the      */
/*  channel and notified. The reason is optional (defaults to "Kicked").      */
/* ========================================================================== */

#include "../Server.hpp"

void Server::cmdKick(Client &client, std::stringstream &ss)
{
	// Must be registered
	if (!client.getIsRegistered())
	{
		client.reply(ERR_NOTREGISTERED, ":You have not registered");
		return;
	}

	// Parse channel name and target nickname
	std::string channelName, targetNick;
	if (!(ss >> channelName >> targetNick))
	{
		client.reply(ERR_NEEDMOREPARAMS, "KICK :Not enough parameters (usage: KICK #channel nickname [reason])");
		return;
	}

	// Parse optional reason (everything after ':' or the next word)
	std::string reason;
	std::getline(ss, reason);
	// Trim leading whitespace and colon
	if (!reason.empty() && reason[0] == ' ')
		reason.erase(0, 1);
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	if (reason.empty())
		reason = "Kicked";

	// Verify channel exists
	Channel *channel = getChannelByName(channelName);
	if (!channel)
	{
		client.reply(ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return;
	}

	// Verify the kicker is a member of the channel
	if (!channel->isMember(&client))
	{
		client.reply(ERR_NOTONCHANNEL, channelName + " :You are not on that channel");
		return;
	}

	// Verify the kicker is an operator
	if (!channel->isMemberOperator(&client))
	{
		client.reply(ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return;
	}

	// Verify the target user exists on the server
	Client *targetClient = getClientByNickname(targetNick);
	if (!targetClient)
	{
		client.reply(ERR_NOSUCHNICK, targetNick + " :No such nick");
		return;
	}

	// Verify the target is in the channel
	if (!channel->isMember(targetClient))
	{
		client.reply(ERR_USERNOTINCHANNEL, targetNick + " " + channelName + " :They aren't on that channel");
		return;
	}

	// Broadcast the KICK to all channel members
	std::string kickMsg = client.getPrefix() + " KICK " + channelName
						  + " " + targetNick + " :" + reason;
	channel->broadcast(kickMsg);

	// Remove the kicked user from the channel and invite list
	channel->removeMember(targetClient);
	channel->removeFromInviteList(targetNick);
}
