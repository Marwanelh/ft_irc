/* ========================================================================== */
/*  Channel.cpp - Implementation of the Channel class                         */
/* ========================================================================== */

#include "Channel.hpp"

/* -------------------------------------------------------------------------- */
/*  Constructor / Destructor / Copy                                           */
/* -------------------------------------------------------------------------- */

Channel::Channel(const std::string &name, bool debug)
	: _name(name),
	  _topic(""),
	  _password(""),
	  _userLimit(0),
	  _modes(0),
	  _debug(debug)
{
}

Channel::~Channel()
{
}

Channel::Channel(const Channel &other)
	: _name(other._name),
	  _topic(other._topic),
	  _password(other._password),
	  _members(other._members),
	  _inviteList(other._inviteList),
	  _userLimit(other._userLimit),
	  _modes(other._modes),
	  _debug(other._debug)
{
}

Channel &Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		// _name is const, cannot be reassigned
		_topic      = other._topic;
		_password   = other._password;
		_members    = other._members;
		_inviteList = other._inviteList;
		_userLimit  = other._userLimit;
		_modes      = other._modes;
		_debug      = other._debug;
	}
	return *this;
}

/* -------------------------------------------------------------------------- */
/*  Name / Topic / Password / Limit Accessors                                 */
/* -------------------------------------------------------------------------- */

std::string Channel::getName() const     { return _name; }
std::string Channel::getTopic() const    { return _topic; }
std::string Channel::getPassword() const { return _password; }
int         Channel::getUserLimit() const{ return _userLimit; }
int         Channel::getModes() const    { return _modes; }

void Channel::setTopic(const std::string &topic)       { _topic = topic; }
void Channel::setPassword(const std::string &password)  { _password = password; }
void Channel::setUserLimit(int limit)                   { _userLimit = limit; }
void Channel::setModes(int modes)                       { _modes = modes; }

/* Set a mode flag bit (e.g. addMode(MD_INV) turns on invite-only) */
void Channel::addMode(int mode)    { _modes |= mode; }

/* Clear a mode flag bit (e.g. removeMode(MD_INV) turns off invite-only) */
void Channel::removeMode(int mode) { _modes &= ~mode; }

/* -------------------------------------------------------------------------- */
/*  Member Management                                                         */
/* -------------------------------------------------------------------------- */

std::map<Client*, bool> Channel::getMembers() const { return _members; }

/* Add a client as a regular (non-operator) member */
void Channel::addMember(Client *client)
{
	_members[client] = false;
}

/* Remove a client from this channel */
void Channel::removeMember(Client *client)
{
	_members.erase(client);
}

/* Return the client pointer if found, NULL otherwise */
Client *Channel::findMember(Client *client) const
{
	std::map<Client*, bool>::const_iterator it = _members.find(client);
	return it != _members.end() ? it->first : NULL;
}

/* Check membership by pointer lookup */
bool Channel::isMember(Client *client) const
{
	return _members.find(client) != _members.end();
}

size_t Channel::getMemberCount() const { return _members.size(); }

/* -------------------------------------------------------------------------- */
/*  Operator Management                                                       */
/* -------------------------------------------------------------------------- */

void Channel::setMemberAsOperator(Client *client)
{
	std::map<Client*, bool>::iterator it = _members.find(client);
	if (it != _members.end())
		it->second = true;
}

void Channel::unsetMemberAsOperator(Client *client)
{
	std::map<Client*, bool>::iterator it = _members.find(client);
	if (it != _members.end())
		it->second = false;
}

/* Returns true if the client is in the channel AND is an operator */
bool Channel::isMemberOperator(Client *client) const
{
	std::map<Client*, bool>::const_iterator it = _members.find(client);
	return it != _members.end() ? it->second : false;
}

/* Count how many operators are currently in the channel */
int Channel::getOperatorsCount() const
{
	int count = 0;
	for (std::map<Client*, bool>::const_iterator it = _members.begin();
		 it != _members.end(); ++it)
	{
		if (it->second)
			count++;
	}
	return count;
}

/* -------------------------------------------------------------------------- */
/*  Invite List                                                               */
/* -------------------------------------------------------------------------- */

std::vector<std::string> Channel::getInviteList() const { return _inviteList; }

/* Add a nickname to the invite list (no duplicates) */
void Channel::addToInviteList(const std::string &nickname)
{
	if (!isInInviteList(nickname))
		_inviteList.push_back(nickname);
}

/* Remove a nickname from the invite list */
void Channel::removeFromInviteList(const std::string &nickname)
{
	std::vector<std::string>::iterator it =
		std::find(_inviteList.begin(), _inviteList.end(), nickname);
	if (it != _inviteList.end())
		_inviteList.erase(it);
}

/* Check if a nickname is on the invite list */
bool Channel::isInInviteList(const std::string &nickname) const
{
	return std::find(_inviteList.begin(), _inviteList.end(), nickname)
		   != _inviteList.end();
}

/* -------------------------------------------------------------------------- */
/*  Broadcasting                                                              */
/* -------------------------------------------------------------------------- */

/*  Send a PRIVMSG-formatted message to every member except the sender.
 *  Used when a client sends a message to the channel.                        */
void Channel::broadcastMessage(const std::string &message, Client *sender) const
{
	std::string fullMsg = sender->getPrefix() + " PRIVMSG " + _name
						  + " " + message + "\r\n";

	for (std::map<Client*, bool>::const_iterator it = _members.begin();
		 it != _members.end(); ++it)
	{
		Client *member = it->first;
		if (member != sender)
			send(member->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
	}
}

/*  Send a raw IRC message to ALL members of the channel.
 *  Automatically appends \r\n if the message doesn't end with it.            */
void Channel::broadcast(const std::string &message) const
{
	std::string msg = message;
	if (msg.length() < 2 || msg.substr(msg.length() - 2) != "\r\n")
		msg += "\r\n";

	for (std::map<Client*, bool>::const_iterator it = _members.begin();
		 it != _members.end(); ++it)
	{
		send(it->first->getFd(), msg.c_str(), msg.length(), 0);
	}
}

/*  Build a space-separated names list for RPL_NAMREPLY.
 *  Operators are prefixed with '@'.                                          */
std::string Channel::getNamesList() const
{
	std::string list;
	for (std::map<Client*, bool>::const_iterator it = _members.begin();
		 it != _members.end(); ++it)
	{
		if (!list.empty())
			list += " ";
		if (it->second)   // operator flag
			list += "@";
		list += it->first->getNickname();
	}
	return list;
}
