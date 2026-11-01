/* ========================================================================== */
/*  cap.cpp - CAP command handler                                             */
/*                                                                            */
/*  Handles IRC capability negotiation. Our server doesn't support any        */
/*  extended capabilities, so we simply reply with an empty list.             */
/* ========================================================================== */

#include "../Server.hpp"

void Server::cmdCap(Client &client)
{
	client.reply(RPL_CAP, ":No supported capabilities available");
}
