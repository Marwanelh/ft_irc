/* ========================================================================== */
/*  topic.cpp - TOPIC command handler                                         */
/*                                                                            */
/*  Syntax: TOPIC <#channel> [:<new topic>]                                   */
/*  Without a topic argument: shows the current topic.                        */
/*  With a topic argument: sets a new topic.                                  */
/*  When +t mode is set, only operators can change the topic.                 */
/* ========================================================================== */

#include "../Server.hpp"

void Server::cmdTopic(Client &client, std::stringstream &ss)
{
	// Must be registered
	if (!client.getIsRegistered())
	{
		client.reply(ERR_NOTREGISTERED, ":You have not registered");
		return;
	}

	std::string channelName;
	if (!(ss >> channelName))
	{
		client.reply(ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");
		return;
	}

	// Verify channel exists
	Channel *channel = getChannelByName(channelName);
	if (!channel)
	{
		client.reply(ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return;
	}

	// Verify the client is a member
	if (!channel->isMember(&client))
	{
		client.reply(ERR_NOTONCHANNEL, channelName + " :You are not on that channel");
		return;
	}

	// Check if there's a topic argument (everything remaining in the stream)
	std::string remaining;
	std::getline(ss, remaining);

	// Trim leading whitespace
	while (!remaining.empty() && remaining[0] == ' ')
		remaining.erase(0, 1);

	// If no topic argument, just display the current topic
	if (remaining.empty())
	{
		if (channel->getTopic().empty())
			client.reply(RPL_NOTOPIC, channelName + " :No topic is set");
		else
			client.reply(RPL_TOPIC, channelName + " :" + channel->getTopic());
		return;
	}

	// Setting a new topic: check +t mode restriction
	if ((channel->getModes() & MD_TOPIC) && !channel->isMemberOperator(&client))
	{
		client.reply(ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return;
	}

	// Extract topic text (remove leading ':' if present)
	std::string newTopic = remaining;
	if (!newTopic.empty() && newTopic[0] == ':')
		newTopic.erase(0, 1);

	channel->setTopic(newTopic);

	// Broadcast the topic change to all channel members
	std::string topicMsg = client.getPrefix() + " TOPIC " + channelName
						   + " :" + newTopic;
	channel->broadcast(topicMsg);
}
