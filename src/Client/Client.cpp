/* ========================================================================== */
/*  Client.cpp - Implementation of the Client class                           */
/* ========================================================================== */

#include "Client.hpp"

/* -------------------------------------------------------------------------- */
/*  Constructor / Destructor / Copy                                           */
/* -------------------------------------------------------------------------- */

Client::Client(int fd, const std::string &serverHostname,
			   const std::string &hostname, bool debug)
	: _fd(fd),
	  _nickname(""),
	  _username(""),
	  _hostname(hostname),
	  _servername(""),
	  _realname(""),
	  _serverHostname(serverHostname),
	  _hasPass(false),
	  _isRegistered(false),
	  _wasDisconnected(false),
	  _debug(debug),
	  _userMode(0)
{
}

Client::~Client()
{
	/* The Server manages closing the fd before deleting the Client,
	   so we do NOT close it here to avoid a double-close. */
}

Client::Client(const Client &other)
	: _fd(other._fd),
	  _buffer(other._buffer),
	  _nickname(other._nickname),
	  _username(other._username),
	  _hostname(other._hostname),
	  _servername(other._servername),
	  _realname(other._realname),
	  _serverHostname(other._serverHostname),
	  _hasPass(other._hasPass),
	  _isRegistered(other._isRegistered),
	  _wasDisconnected(other._wasDisconnected),
	  _debug(other._debug),
	  _userMode(other._userMode)
{
}

Client &Client::operator=(const Client &other)
{
	if (this != &other)
	{
		_fd              = other._fd;
		_buffer          = other._buffer;
		_nickname        = other._nickname;
		_username        = other._username;
		_hostname        = other._hostname;
		_servername      = other._servername;
		_realname        = other._realname;
		_serverHostname  = other._serverHostname;
		_hasPass         = other._hasPass;
		_isRegistered    = other._isRegistered;
		_wasDisconnected = other._wasDisconnected;
		_debug           = other._debug;
		_userMode        = other._userMode;
	}
	return *this;
}

/* -------------------------------------------------------------------------- */
/*  IRC Reply Helper                                                          */
/* -------------------------------------------------------------------------- */

/*  Sends a formatted IRC reply to this client's socket.
 *  Format: ":serverHostname <code> <nick> <message>\r\n"                     */
void Client::reply(const std::string &code, const std::string &message)
{
	std::string nick = _nickname.empty() ? "*" : _nickname;
	std::string replyMsg = ":" + _serverHostname + " " + code
						   + " " + nick + " " + message + CRLF;
	if (_debug)
		std::cout << FBLU("[LOG] Reply -> ") << replyMsg.substr(0, replyMsg.length() - 2) << std::endl;
	send(_fd, replyMsg.c_str(), replyMsg.length(), 0);
}

/* -------------------------------------------------------------------------- */
/*  Getters                                                                   */
/* -------------------------------------------------------------------------- */

int         Client::getFd() const             { return _fd; }
std::string Client::getBuffer() const         { return _buffer; }
std::string Client::getNickname() const       { return _nickname; }
std::string Client::getUsername() const        { return _username; }
std::string Client::getHostname() const       { return _hostname; }
std::string Client::getRealname() const       { return _realname; }
std::string Client::getServername() const     { return _servername; }
bool        Client::getIsRegistered() const   { return _isRegistered; }
bool        Client::getHasPass() const        { return _hasPass; }
bool        Client::getWasDisconnected() const{ return _wasDisconnected; }
int         Client::getUserMode() const       { return _userMode; }

/* -------------------------------------------------------------------------- */
/*  Setters                                                                   */
/* -------------------------------------------------------------------------- */

void Client::setFd(int fd)                                  { _fd = fd; }
void Client::setBuffer(const std::string &buffer)           { _buffer = buffer; }
void Client::setNickname(const std::string &nickname)       { _nickname = nickname; }
void Client::setUsername(const std::string &username)        { _username = username; }
void Client::setHostname(const std::string &hostname)       { _hostname = hostname; }
void Client::setRealname(const std::string &realname)       { _realname = realname; }
void Client::setServername(const std::string &servername)    { _servername = servername; }
void Client::setIsRegistered(bool status)                   { _isRegistered = status; }
void Client::setHasPass(bool status)                        { _hasPass = status; }
void Client::setWasDisconnected(bool status)                { _wasDisconnected = status; }

/* Adds a mode flag using bitwise OR (does not replace existing modes) */
void Client::addUserMode(int mode) { _userMode |= mode; }

/* -------------------------------------------------------------------------- */
/*  Buffer Management (partial-message reassembly per IRC spec)               */
/* -------------------------------------------------------------------------- */

void Client::appendBuffer(const std::string &data) { _buffer += data; }
void Client::clearBuffer()                          { _buffer.clear(); }

/* -------------------------------------------------------------------------- */
/*  IRC Prefix                                                                */
/* -------------------------------------------------------------------------- */

/* Returns ":nick!user@host" used as the source prefix in relayed messages */
std::string Client::getPrefix() const
{
	return ":" + _nickname + "!" + _username + "@" + _hostname;
}
