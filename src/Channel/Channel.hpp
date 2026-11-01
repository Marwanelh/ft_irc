/* ========================================================================== */
/*  Channel.hpp - Represents a single IRC channel                             */
/*                                                                            */
/*  A channel has a name (always starts with '#'), a topic, optional          */
/*  password, a member list (with operator flags), and mode bitmask.          */
/* ========================================================================== */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include "../Client/Client.hpp"

/* --- Channel mode bitmask flags --- */
#define MD_INV       0x0001   // +i : invite-only channel
#define MD_TOPIC     0x0002   // +t : only operators can change topic
#define MD_PASSWORD  0x0004   // +k : channel requires a password to join
#define MD_USR_LIM   0x0008   // +l : channel has a user limit

class Channel
{
	private:
		std::string const           _name;       // Channel name (e.g. "#general")
		std::string                 _topic;      // Current channel topic
		std::string                 _password;   // Password if +k is set
		std::map<Client*, bool>     _members;    // Map of members; value = true if operator
		std::vector<std::string>    _inviteList; // Nicknames invited to this channel
		int                         _userLimit;  // Max members if +l is set
		int                         _modes;      // Bitmask of active channel modes
		bool                        _debug;      // Debug logging flag

	public:
		/* --- Orthodox Canonical Form --- */
		Channel(const std::string &name, bool debug);
		~Channel();
		Channel(const Channel &other);
		Channel &operator=(const Channel &other);

		/* --- Name (immutable after creation) --- */
		std::string getName() const;

		/* --- Topic --- */
		std::string getTopic() const;
		void setTopic(const std::string &topic);

		/* --- Password --- */
		std::string getPassword() const;
		void setPassword(const std::string &password);

		/* --- User limit --- */
		int  getUserLimit() const;
		void setUserLimit(int limit);

		/* --- Mode bitmask --- */
		int  getModes() const;
		void setModes(int modes);
		void addMode(int mode);      // Set a mode flag bit
		void removeMode(int mode);   // Clear a mode flag bit

		/* --- Member management --- */
		std::map<Client*, bool> getMembers() const;
		void    addMember(Client *client);
		void    removeMember(Client *client);
		Client* findMember(Client *client) const;
		bool    isMember(Client *client) const;
		size_t  getMemberCount() const;

		/* --- Operator management --- */
		void setMemberAsOperator(Client *client);
		void unsetMemberAsOperator(Client *client);
		bool isMemberOperator(Client *client) const;
		int  getOperatorsCount() const;

		/* --- Invite list --- */
		std::vector<std::string> getInviteList() const;
		void addToInviteList(const std::string &nickname);
		void removeFromInviteList(const std::string &nickname);
		bool isInInviteList(const std::string &nickname) const;

		/* --- Broadcasting to channel members --- */

		/* Send a PRIVMSG-formatted message to all members except sender */
		void broadcastMessage(const std::string &message, Client *sender) const;

		/* Send a raw IRC message to all members (auto-appends \r\n if missing) */
		void broadcast(const std::string &message) const;

		/* Build a space-separated list of nicks (operators prefixed with @) */
		std::string getNamesList() const;
};

#endif
