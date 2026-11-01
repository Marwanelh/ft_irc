/* ========================================================================== */
/*  Server.hpp - The core IRC server class                                    */
/*                                                                            */
/*  Manages the listening socket, poll loop, client connections, channels,    */
/*  and dispatches incoming commands to their handlers.                        */
/* ========================================================================== */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <cstdlib>
#include <map>
#include <sys/socket.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <netdb.h>
#include <sstream>
#include <cstdio>
#include <csignal>

#include "../Client/Client.hpp"
#include "../Channel/Channel.hpp"
#include "../utils/IRCmacros.hpp"
#include "../utils/stdoutColors.hpp"

class Channel;

/* Global flag set to false by the signal handler to trigger shutdown */
extern bool g_serverRunning;

/* --- Command identifiers for the switch dispatcher --- */
enum CommandType
{
	CMD_CAP,
	CMD_PASS,
	CMD_NICK,
	CMD_USER,
	CMD_PRIVMSG,
	CMD_PING,
	CMD_MODE,
	CMD_JOIN,
	CMD_INVITE,
	CMD_KICK,
	CMD_TOPIC,
	CMD_QUIT,
	CMD_UNKNOWN
};

class Server
{
	private:
		const int               _port;           // Port the server listens on
		const std::string       _password;       // Connection password
		bool                    _debug;          // Debug logging flag
		const std::string       _hostname;       // Server's hostname string

		int                     _serverSocketFd; // Listening socket FD

		std::vector<pollfd>     _pollfds;        // Poll descriptors array
		std::map<int, Client*>  _clients;        // FD -> Client* mapping
		std::vector<Channel*>   _channels;       // Active channels

	public:
		/* --- Orthodox Canonical Form --- */
		Server(int port, const std::string &password, bool debug);
		~Server();
		Server(const Server &src);
		Server &operator=(const Server &src);

		/* --- Server lifecycle --- */
		void serverRun();
		void serverSocketStart();

		/* --- Client management --- */
		void newClientConnection();
		void handleClientMessage(int pollIndex, Client &client);
		void disconnectClient(int &pollIndex);
		void handleQuitCleanup(Client &client, const std::string &reason);

		/* --- Client lookup --- */
		bool    hasClient(const std::string &nickname) const;
		Client* getClientByNickname(const std::string &nickname) const;

		/* --- Command processing --- */
		void processCommand(Client &client, const std::string &line);
		int  parseCommand(const std::string &cmd) const;

		/* --- Individual command handlers --- */
		void cmdCap(Client &client);
		void cmdPass(Client &client, std::stringstream &ss);
		void cmdNick(Client &client, std::stringstream &ss);
		void cmdUser(Client &client, std::stringstream &ss);
		void cmdPrivmsg(Client &client, const std::string &line, std::stringstream &ss);
		void cmdMode(Client &client, std::stringstream &ss);
		void cmdJoin(Client &client, std::stringstream &ss);
		void cmdInvite(Client &client, std::stringstream &ss);
		void cmdKick(Client &client, std::stringstream &ss);
		void cmdTopic(Client &client, std::stringstream &ss);
		void cmdQuit(Client &client, std::stringstream &ss);

		/* --- Utilities --- */
		void     addPollfd(std::vector<pollfd> &fds, int fd, short events);
		Channel* getChannelByName(const std::string &name) const;
};

#endif
