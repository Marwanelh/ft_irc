/* ========================================================================== */
/*  mode.cpp - MODE command handler                                           */
/*                                                                            */
/*  Supports channel modes:                                                   */
/*    i: Set/remove Invite-only channel                                       */
/*    t: Set/remove TOPIC restriction to channel operators                     */
/*    k: Set/remove channel key (password)                                    */
/*    o: Give/take channel operator privilege                                 */
/*    l: Set/remove user limit to channel                                     */
/* ========================================================================== */

#include "../Server.hpp"

void Server::cmdMode(Client &client, std::stringstream &ss)
{
	if (!client.getIsRegistered())
	{
		client.reply(ERR_NOTREGISTERED, ":You have not registered");
		return;
	}

	std::string target, modeString;
	if (!(ss >> target))
	{
		client.reply(ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
		return;
	}

	// User mode query / setting (e.g., MODE nick)
	if (target[0] != '#')
	{
		if (target != client.getNickname())
		{
			client.reply(ERR_USERSDONTMATCH, ":Cannot change mode for other users");
			return;
		}
		if (!(ss >> modeString))
		{
			client.reply(RPL_UMODEIS, "+i");
			return;
		}
		return;
	}

	// Channel mode handling
	Channel *channel = getChannelByName(target);
	if (!channel)
	{
		client.reply(ERR_NOSUCHCHANNEL, target + " :No such channel");
		return;
	}

	// Query active channel modes: MODE #channel (no mode string)
	if (!(ss >> modeString))
	{
		std::string currentModes = "+";
		if (channel->getModes() & MD_INV)      currentModes += "i";
		if (channel->getModes() & MD_TOPIC)    currentModes += "t";
		if (channel->getModes() & MD_PASSWORD) currentModes += "k";
		if (channel->getModes() & MD_USR_LIM)  currentModes += "l";

		std::string modeArgs = "";
		if (channel->getModes() & MD_PASSWORD)
			modeArgs += " " + channel->getPassword();
		if (channel->getModes() & MD_USR_LIM)
		{
			std::stringstream oss;
			oss << channel->getUserLimit();
			modeArgs += " " + oss.str();
		}
		client.reply(RPL_CHANNELMODEIS, channel->getName() + " " + currentModes + modeArgs);
		return;
	}

	// Changing channel modes requires channel operator privileges
	if (!channel->isMemberOperator(&client))
	{
		client.reply(ERR_CHANOPRIVSNEEDED, target + " :You're not channel operator");
		return;
	}

	bool adding = true;
	std::string appliedModes = "";
	std::string appliedArgs = "";
	std::string modeArg;

	for (size_t i = 0; i < modeString.length(); ++i)
	{
		char c = modeString[i];
		if (c == '+')
		{
			adding = true;
			continue;
		}
		if (c == '-')
		{
			adding = false;
			continue;
		}

		switch (c)
		{
			case 'i': // Invite-only
				adding ? channel->addMode(MD_INV) : channel->removeMode(MD_INV);
				appliedModes += (adding ? "+i" : "-i");
				break;

			case 't': // Topic restriction
				adding ? channel->addMode(MD_TOPIC) : channel->removeMode(MD_TOPIC);
				appliedModes += (adding ? "+t" : "-t");
				break;

			case 'k': // Channel key (password)
				if (adding)
				{
					if (ss >> modeArg)
					{
						channel->setPassword(modeArg);
						channel->addMode(MD_PASSWORD);
						appliedModes += "+k";
						appliedArgs += " " + modeArg;
					}
					else
					{
						client.reply(ERR_NEEDMOREPARAMS, "MODE +k :Not enough parameters");
					}
				}
				else
				{
					channel->setPassword("");
					channel->removeMode(MD_PASSWORD);
					appliedModes += "-k";
				}
				break;

			case 'l': // User limit
				if (adding)
				{
					if (ss >> modeArg)
					{
						int limit = std::atoi(modeArg.c_str());
						if (limit > 0)
						{
							channel->setUserLimit(limit);
							channel->addMode(MD_USR_LIM);
							appliedModes += "+l";
							appliedArgs += " " + modeArg;
						}
					}
					else
					{
						client.reply(ERR_NEEDMOREPARAMS, "MODE +l :Not enough parameters");
					}
				}
				else
				{
					channel->setUserLimit(0);
					channel->removeMode(MD_USR_LIM);
					appliedModes += "-l";
				}
				break;

			case 'o': // Channel operator privilege
				if (ss >> modeArg)
				{
					Client *targetClient = getClientByNickname(modeArg);
					if (targetClient && channel->isMember(targetClient))
					{
						adding ? channel->setMemberAsOperator(targetClient)
							   : channel->unsetMemberAsOperator(targetClient);
						appliedModes += (adding ? "+o" : "-o");
						appliedArgs += " " + modeArg;
					}
					else if (!targetClient)
					{
						client.reply(ERR_NOSUCHNICK, modeArg + " :No such nick");
					}
					else
					{
						client.reply(ERR_USERNOTINCHANNEL, modeArg + " " + target + " :They aren't on that channel");
					}
				}
				else
				{
					client.reply(ERR_NEEDMOREPARAMS, "MODE " + std::string(adding ? "+o" : "-o") + " :Not enough parameters");
				}
				break;

			default:
				client.reply(ERR_UNKNOWNMODE, std::string(1, c) + " :is unknown mode char to me");
				break;
		}
	}

	// Broadcast applied mode changes to the entire channel
	if (!appliedModes.empty())
	{
		std::string broadcastMsg = client.getPrefix() + " MODE " + target
								   + " " + appliedModes + appliedArgs;
		channel->broadcast(broadcastMsg);
	}
}
