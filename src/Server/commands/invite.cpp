/* ========================================================================== */
/*  invite.cpp - INVITE command handler                                       */
/*                                                                            */
/*  Syntax: INVITE <nickname> <#channel>                                      */
/*  Invites a user to a channel. If the channel is invite-only (+i),          */
/*  only operators can invite. The invited user receives a notification.      */
/* ========================================================================== */

#include "../Server.hpp"

void Server::cmdInvite(Client &client, std::stringstream &ss)
{
	// Must be registered
	if (!client.getIsRegistered())
	{
		client.reply(ERR_NOTREGISTERED, ":You have not registered");
		return;
	}

	std::string targetNick, channelName;
	if (!(ss >> targetNick >> channelName))
	{
		client.reply(ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters");
		return;
	}

	// Verify channel exists
	Channel *channel = getChannelByName(channelName);
	if (!channel)
	{
		client.reply(ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return;
	}

	// Verify the target user exists on the server
	Client *targetClient = getClientByNickname(targetNick);
	if (!targetClient)
	{
		client.reply(ERR_NOSUCHNICK, targetNick + " :No such nick");
		return;
	}

	// Verify the inviter is a member of the channel
	if (!channel->isMember(&client))
	{
		client.reply(ERR_NOTONCHANNEL, channelName + " :You are not on that channel");
		return;
	}

	// If channel is invite-only, only operators can invite
	if ((channel->getModes() & MD_INV) && !channel->isMemberOperator(&client))
	{
		client.reply(ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return;
	}

	// Check if target is already in the channel
	if (channel->isMember(targetClient))
	{
		client.reply(ERR_USERONCHANNEL, targetNick + " " + channelName + " :is already on channel");
		return;
	}

	// Add to invite list and notify both parties
	channel->addToInviteList(targetNick);
	client.reply(RPL_INVITING, targetNick + " " + channelName);

	// Notify the invited user
	std::string inviteMsg = client.getPrefix() + " INVITE " + targetNick
							+ " :" + channelName + "\r\n";
	send(targetClient->getFd(), inviteMsg.c_str(), inviteMsg.length(), 0);
}
