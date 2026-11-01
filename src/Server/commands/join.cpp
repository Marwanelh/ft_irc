/* ========================================================================== */
/*  join.cpp - JOIN command handler                                           */
/*                                                                            */
/*  Syntax: JOIN <#channel> [<password>]                                      */
/*  Creates a channel if it doesn't exist (creator becomes operator).         */
/*  Otherwise, joins an existing channel after checking permissions:          */
/*    - invite-only (+i), password (+k), user limit (+l)                      */
/* ========================================================================== */

#include "../Server.hpp"

void Server::cmdJoin(Client &client, std::stringstream &ss)
{
	// Must be registered to join channels
	if (!client.getIsRegistered())
	{
		client.reply(ERR_NOTREGISTERED, ":You have not registered");
		return;
	}

	std::string channelName;
	if (!(ss >> channelName))
	{
		client.reply(ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
		return;
	}

	// Channel names must start with '#'
	if (channelName[0] != '#')
	{
		client.reply(ERR_NOSUCHCHANNEL, channelName + " :Channel names must start with '#'");
		return;
	}

	Channel *channel = getChannelByName(channelName);

	if (!channel)
	{
		// --- Create a new channel; the creator is automatically operator ---
		Channel *newChannel = new Channel(channelName, _debug);
		newChannel->addMember(&client);
		newChannel->setMemberAsOperator(&client);
		_channels.push_back(newChannel);
		channel = newChannel;

		if (_debug)
			std::cout << FGRN("[LOG] Created channel ") << channelName
					  << ", operator: " << client.getNickname() << std::endl;
	}
	else
	{
		// --- Join an existing channel: check restrictions ---

		// Already a member?
		if (channel->isMember(&client))
		{
			client.reply(ERR_USERONCHANNEL, channelName + " :You are already on that channel");
			return;
		}

		// Check invite-only mode (+i)
		if ((channel->getModes() & MD_INV) && !channel->isInInviteList(client.getNickname()))
		{
			client.reply(ERR_INVITEONLYCHAN, channelName + " :Cannot join channel (invite only)");
			return;
		}

		// Check password mode (+k)
		if (channel->getModes() & MD_PASSWORD)
		{
			std::string password;
			ss >> password;
			if (password.empty() || password != channel->getPassword())
			{
				client.reply(ERR_BADCHANNELKEY, channelName + " :Cannot join channel (incorrect password)");
				return;
			}
		}

		// Check user limit (+l)
		if ((channel->getModes() & MD_USR_LIM)
			&& channel->getUserLimit() > 0
			&& (int)channel->getMemberCount() >= channel->getUserLimit())
		{
			client.reply(ERR_CHANNELISFULL, channelName + " :Cannot join channel (channel is full)");
			return;
		}

		channel->addMember(&client);

		if (_debug)
			std::cout << FGRN("[LOG] ") << client.getNickname()
					  << " joined " << channelName << std::endl;
	}

	// Broadcast the JOIN message to all channel members (including the new member)
	std::string joinMsg = client.getPrefix() + " JOIN :" + channelName;
	channel->broadcast(joinMsg);

	// Send the topic (if one is set)
	if (!channel->getTopic().empty())
		client.reply(RPL_TOPIC, channelName + " :" + channel->getTopic());

	// Send the names list
	client.reply(RPL_NAMREPLY, "= " + channelName + " :" + channel->getNamesList());
	client.reply(RPL_ENDOFNAMES, channelName + " :End of /NAMES list.");
}
