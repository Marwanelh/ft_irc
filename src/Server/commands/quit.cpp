/* ========================================================================== */
/*  quit.cpp - QUIT command handler and client disconnection cleanup          */
/*                                                                            */
/*  Notifies all channels the quitting user was part of, removes them from     */
/*  each channel, deletes empty channels, and flags the client for             */
/*  disconnection by the main loop.                                            */
/* ========================================================================== */

#include "../Server.hpp"

void Server::cmdQuit(Client &client, std::stringstream &ss)
{
	std::string reason;
	std::getline(ss, reason);

	// Clean up reason string
	while (!reason.empty() && reason[0] == ' ')
		reason.erase(0, 1);
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	if (reason.empty())
		reason = "Client Quit";

	// 1. Notify channels and clean up membership
	handleQuitCleanup(client, reason);

	// 2. Mark client for immediate disconnection in handleClientMessage
	client.setWasDisconnected(true);
}

void Server::handleQuitCleanup(Client &client, const std::string &reason)
{
	std::string quitMsg = client.getPrefix() + " QUIT :" + reason;

	for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); )
	{
		if ((*it)->isMember(&client))
		{
			// Broadcast quit notification to all channel peers
			(*it)->broadcast(quitMsg);
			(*it)->removeMember(&client);

			// Delete channel if it has become empty
			if ((*it)->getMemberCount() == 0)
			{
				delete *it;
				it = _channels.erase(it);
				continue;
			}
		}
		++it;
	}
}
