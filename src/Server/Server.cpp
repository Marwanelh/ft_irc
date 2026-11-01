/* ========================================================================== */
/*  Server.cpp - Core server implementation                                   */
/*                                                                            */
/*  Handles socket setup, the poll() event loop, client connections,          */
/*  message reception with partial-data reassembly, and clean shutdown.        */
/* ========================================================================== */

#include "Server.hpp"

/* -------------------------------------------------------------------------- */
/*  Constructor / Destructor / Copy                                           */
/* -------------------------------------------------------------------------- */

Server::Server(int port, const std::string &password, bool debug)
	: _port(port),
	  _password(password),
	  _debug(debug),
	  _hostname("ircserver"),
	  _serverSocketFd(-1)
{
}

Server::~Server()
{
	/* Clean up all clients (close FDs and free memory) */
	for (std::map<int, Client*>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	_clients.clear();

	/* Clean up all channels */
	for (size_t i = 0; i < _channels.size(); ++i)
		delete _channels[i];
	_channels.clear();

	/* Close the listening socket */
	if (_serverSocketFd != -1)
		close(_serverSocketFd);
}

/* Note: copy constructor and assignment operator are required by the
   Orthodox Canonical Form but should NOT be used — they would shallow-copy
   raw pointers to Client and Channel objects. */
Server::Server(const Server &src)
	: _port(src._port),
	  _password(src._password),
	  _debug(src._debug),
	  _hostname(src._hostname),
	  _serverSocketFd(src._serverSocketFd),
	  _pollfds(src._pollfds),
	  _clients(src._clients),
	  _channels(src._channels)
{
}

Server &Server::operator=(const Server &src)
{
	if (this != &src)
	{
		// _port, _password, _hostname are const — cannot be reassigned
		_debug          = src._debug;
		_serverSocketFd = src._serverSocketFd;
		_pollfds        = src._pollfds;
		_clients        = src._clients;
		_channels       = src._channels;
	}
	return *this;
}

/* -------------------------------------------------------------------------- */
/*  Server Socket Setup                                                       */
/* -------------------------------------------------------------------------- */

/*  Creates, configures, binds, and starts listening on the server socket.
 *  Steps: socket() -> setsockopt(REUSEADDR) -> fcntl(NONBLOCK) -> bind() -> listen() */
void Server::serverSocketStart()
{
	// 1. Create a TCP socket
	_serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocketFd == -1)
		throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
	std::cout << "Socket created successfully." << std::endl;

	// 2. Allow address reuse (avoids "Address already in use" on restart)
	int opt = 1;
	if (setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(_serverSocketFd);
		throw std::runtime_error("setsockopt(SO_REUSEADDR) failed: " + std::string(strerror(errno)));
	}

	// 3. Set non-blocking mode (required by subject: fcntl(fd, F_SETFL, O_NONBLOCK))
	if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(_serverSocketFd);
		throw std::runtime_error("fcntl(O_NONBLOCK) failed: " + std::string(strerror(errno)));
	}

	// 4. Bind to the specified port on all interfaces (0.0.0.0)
	struct sockaddr_in serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family      = AF_INET;
	serverAddr.sin_port        = htons(_port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(_serverSocketFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		close(_serverSocketFd);
		throw std::runtime_error("bind() failed: " + std::string(strerror(errno)));
	}

	// 5. Start listening for incoming connections
	if (listen(_serverSocketFd, SOMAXCONN) == -1)
	{
		close(_serverSocketFd);
		throw std::runtime_error("listen() failed: " + std::string(strerror(errno)));
	}
	std::cout << "Server listening on port " << _port << std::endl;
}

/* -------------------------------------------------------------------------- */
/*  Main Event Loop                                                           */
/* -------------------------------------------------------------------------- */

/*  The heart of the server: initializes the socket, then enters a poll()
 *  loop that handles new connections, stdin commands, and client messages.    */
void Server::serverRun()
{
	serverSocketStart();

	// pollfds[0] = server socket (for new connections)
	addPollfd(_pollfds, _serverSocketFd, POLLIN);
	// pollfds[1] = stdin (for server operator "exit" command)
	addPollfd(_pollfds, STDIN_FILENO, POLLIN);

	if (_debug)
		std::cout << "[LOG] Server is running. Waiting for connections..." << std::endl;

	while (g_serverRunning)
	{
		// poll() with 100ms timeout to avoid busy-waiting
		int pollCount = poll(&_pollfds[0], _pollfds.size(), 100);
		if (pollCount == -1)
		{
			if (errno == EINTR)    // Interrupted by signal (e.g. SIGINT)
				break;
			throw std::runtime_error("poll() failed: " + std::string(strerror(errno)));
		}
		if (pollCount == 0)
			continue;   // Timeout, no events — loop back

		// --- Check for new client connections on the server socket ---
		if (_pollfds[0].revents & POLLIN)
		{
			try
			{
				if (_debug)
					std::cout << "[LOG] New client connection incoming..." << std::endl;
				newClientConnection();
			}
			catch (const std::exception &e)
			{
				std::cerr << "Connection error: " << e.what() << std::endl;
				continue;
			}
		}

		// --- Check for stdin input ("exit" command from server operator) ---
		if (_pollfds[1].revents & POLLIN)
		{
			std::string input;
			std::getline(std::cin, input);
			if (input == "EXIT" || input == "exit")
				break;
		}

		// --- Process client messages (starting at index 2) ---
		for (size_t i = 2; i < _pollfds.size(); ++i)
		{
			// Skip if this FD has no client (shouldn't happen, but safety check)
			if (_clients.find(_pollfds[i].fd) == _clients.end())
				continue;

			if (_pollfds[i].revents & POLLIN)
			{
				if (_debug)
					std::cout << "[LOG] Data incoming on FD: " << _pollfds[i].fd << std::endl;
				int idx = static_cast<int>(i);
				handleClientMessage(idx, *_clients[_pollfds[i].fd]);
			}
		}
	}
	std::cout << "Server is shutting down..." << std::endl;
}

/* -------------------------------------------------------------------------- */
/*  New Client Connection                                                     */
/* -------------------------------------------------------------------------- */

/*  Accepts a pending connection, creates a Client object, and registers
 *  the new FD with the poll array.                                           */
void Server::newClientConnection()
{
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);

	int clientFd = accept(_serverSocketFd, (sockaddr*)&clientAddr, &addrLen);
	if (clientFd == -1)
		throw std::runtime_error("accept() failed: " + std::string(strerror(errno)));

	// Set client socket to non-blocking (subject requirement: fcntl(fd, F_SETFL, O_NONBLOCK))
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(clientFd);
		throw std::runtime_error("fcntl(O_NONBLOCK) failed: " + std::string(strerror(errno)));
	}

	// Add the new FD to the poll set
	addPollfd(_pollfds, clientFd, POLLIN | POLLHUP);

	// Resolve the client's IP address
	char hostBuf[NI_MAXHOST];
	int result = getnameinfo((struct sockaddr *)&clientAddr, addrLen,
							 hostBuf, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
	if (result != 0)
	{
		close(clientFd);
		throw std::runtime_error("getnameinfo failed: " + std::string(gai_strerror(result)));
	}

	// Create and store the Client object
	Client *newClient = new Client(clientFd, _hostname, hostBuf, _debug);
	_clients.insert(std::make_pair(clientFd, newClient));

	std::cout << "Client connected: " << newClient->getHostname()
			  << " (FD " << clientFd << ")" << std::endl;
}

/* -------------------------------------------------------------------------- */
/*  Client Message Handling                                                   */
/* -------------------------------------------------------------------------- */

/*  Reads data from a client socket, appends it to the client's buffer,
 *  then processes complete lines (delimited by \n).
 *  This handles IRC's partial-data requirement (nc ctrl+D test).             */
void Server::handleClientMessage(int pollIndex, Client &client)
{
	char buf[BUFSIZ];
	std::memset(buf, 0, sizeof(buf));

	int bytesRead = recv(_pollfds[pollIndex].fd, buf, sizeof(buf) - 1, 0);
	if (_debug)
		std::cout << FRED("[LOG] Received ") << bytesRead << " bytes" << std::endl;

	// Connection closed or error
	if (bytesRead <= 0)
	{
		handleQuitCleanup(client, "Connection lost");
		disconnectClient(pollIndex);
		return;
	}

	// Append received data to the client's buffer
	buf[bytesRead] = '\0';
	client.appendBuffer(buf);

	// Process all complete lines in the buffer
	while (client.getBuffer().find("\n") != std::string::npos)
	{
		size_t pos = client.getBuffer().find("\n");
		std::string line = client.getBuffer().substr(0, pos);

		// Strip trailing \r (IRC uses \r\n line endings)
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		// Advance the buffer past this line
		client.setBuffer(client.getBuffer().substr(pos + 1));

		// Process the command
		if (!line.empty())
			processCommand(client, line);

		// If QUIT was processed, disconnect immediately
		if (client.getWasDisconnected())
		{
			disconnectClient(pollIndex);
			return;
		}
	}
}

/* -------------------------------------------------------------------------- */
/*  Client Disconnection                                                      */
/* -------------------------------------------------------------------------- */

/*  Closes a client's socket, removes them from _clients and _pollfds,
 *  and adjusts the poll index so the loop doesn't skip the next client.      */
void Server::disconnectClient(int &pollIndex)
{
	int fd = _pollfds[pollIndex].fd;
	Client *client = _clients[fd];

	if (_debug)
	{
		std::string name = client->getNickname().empty()
						   ? "Unregistered" : client->getNickname();
		std::cout << FRED("[LOG] ") << name << " (FD " << fd
				  << ") disconnected." << std::endl;
	}

	// Close the socket and free the Client
	close(fd);
	delete client;
	_clients.erase(fd);
	_pollfds.erase(_pollfds.begin() + pollIndex);

	// Adjust index so the for-loop doesn't skip the next element
	pollIndex--;
}

/* -------------------------------------------------------------------------- */
/*  Client Lookup                                                             */
/* -------------------------------------------------------------------------- */

/* Returns true if any connected client has this nickname */
bool Server::hasClient(const std::string &nickname) const
{
	for (std::map<int, Client*>::const_iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return true;
	}
	return false;
}

/* Returns the Client pointer for a given nickname, or NULL if not found */
Client *Server::getClientByNickname(const std::string &nickname) const
{
	for (std::map<int, Client*>::const_iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return it->second;
	}
	return NULL;
}

/* -------------------------------------------------------------------------- */
/*  Utility Methods                                                           */
/* -------------------------------------------------------------------------- */

/* Register a file descriptor with the poll array */
void Server::addPollfd(std::vector<pollfd> &fds, int fd, short events)
{
	pollfd pfd;
	pfd.fd      = fd;
	pfd.events  = events;
	pfd.revents = 0;
	fds.push_back(pfd);
}

/* Find a channel by its name (e.g. "#general"), returns NULL if not found */
Channel *Server::getChannelByName(const std::string &name) const
{
	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->getName() == name)
			return _channels[i];
	}
	return NULL;
}
