/* ========================================================================== */
/*  Client.hpp - Represents a single connected IRC client                     */
/*                                                                            */
/*  Each Client holds its socket FD, nickname, username, hostname, and        */
/*  registration state. The Server owns the lifetime of Client objects.       */
/* ========================================================================== */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>

#include "../utils/IRCmacros.hpp"
#include "../utils/stdoutColors.hpp"

class Client
{
	private:
		int			_fd;              // Socket file descriptor
		std::string	_buffer;          // Partial-message accumulation buffer

		std::string	_nickname;        // IRC nickname (unique on the server)
		std::string	_username;        // Username from USER command
		std::string	_hostname;        // Client's resolved hostname / IP
		std::string	_servername;      // Server name echoed back to the client
		std::string	_realname;        // Real name from USER command

		std::string	_serverHostname;  // Our server's hostname for reply prefix

		bool		_hasPass;         // True after a correct PASS command
		bool		_isRegistered;    // True after PASS + NICK + USER complete
		bool		_wasDisconnected; // Flagged when the client should be removed
		bool		_debug;           // Debug logging enabled

		int			_userMode;        // Bitmask of user modes (e.g. invisible)

	public:
		/* --- Orthodox Canonical Form --- */
		Client(int fd, const std::string &serverHostname,
			   const std::string &hostname, bool debug);
		~Client();
		Client(const Client &other);
		Client &operator=(const Client &other);

		/* --- IRC reply helper: formats and sends ":server CODE nick msg\r\n" --- */
		void reply(const std::string &code, const std::string &message);

		/* --- Getters --- */
		int         getFd() const;
		std::string getBuffer() const;
		std::string getNickname() const;
		std::string getUsername() const;
		std::string getHostname() const;
		std::string getRealname() const;
		std::string getServername() const;
		bool        getIsRegistered() const;
		bool        getHasPass() const;
		bool        getWasDisconnected() const;
		int         getUserMode() const;

		/* --- Setters --- */
		void setFd(int fd);
		void setBuffer(const std::string &buffer);
		void setNickname(const std::string &nickname);
		void setUsername(const std::string &username);
		void setHostname(const std::string &hostname);
		void setRealname(const std::string &realname);
		void setServername(const std::string &servername);
		void setIsRegistered(bool status);
		void setHasPass(bool status);
		void setWasDisconnected(bool status);
		void addUserMode(int mode);

		/* --- Buffer management for partial-message reassembly --- */
		void appendBuffer(const std::string &data);
		void clearBuffer();

		/* --- Build the IRC prefix ":nick!user@host" for this client --- */
		std::string getPrefix() const;
};

#endif
