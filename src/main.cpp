/* ========================================================================== */
/*  main.cpp - Entry point for the ft_irc server                              */
/*                                                                            */
/*  Usage: ./ircserver <port> <password> [--debug]                            */
/*  Validates arguments, sets up signal handling, and starts the server.      */
/* ========================================================================== */

#include "Server/Server.hpp"
#include <csignal>

/* Global shutdown flag — set to false by SIGINT handler */
bool g_serverRunning = true;

/* -------------------------------------------------------------------------- */
/*  Signal Handling                                                           */
/* -------------------------------------------------------------------------- */

static void signalHandler(int signum)
{
	(void)signum;
	std::cout << "\n[SIGNAL] Shutdown signal received. Cleaning up..." << std::endl;
	g_serverRunning = false;
}

/*  Register signal handlers:
 *  - SIGINT  (Ctrl+C)  → graceful shutdown
 *  - SIGPIPE           → ignored (broken pipe when client disconnects mid-send)
 *  - SIGTERM/SIGQUIT   → ignored (prevent accidental termination)                */
static void setupSignals()
{
	std::signal(SIGINT,  signalHandler);
	std::signal(SIGPIPE, SIG_IGN);
	std::signal(SIGTERM, SIG_IGN);
	std::signal(SIGQUIT, SIG_IGN);
}

/* -------------------------------------------------------------------------- */
/*  Main                                                                      */
/* -------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
	setupSignals();

	// Validate argument count: ./ircserver <port> <password> [--debug]
	if (argc < 3 || argc > 4
		|| (argc == 4 && std::string(argv[3]) != "--debug"))
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password> [--debug]" << std::endl;
		return 1;
	}

	// Validate that port is a positive numeric value
	std::string portStr = argv[1];
	if (portStr.find_first_not_of("0123456789") != std::string::npos)
	{
		std::cerr << "Error: Port must be a numeric value." << std::endl;
		return 1;
	}
	int port = std::atoi(argv[1]);
	if (port <= 0 || port > 65535)
	{
		std::cerr << "Error: Port must be between 1 and 65535." << std::endl;
		return 1;
	}

	std::string password = argv[2];
	bool debug = (argc == 4);

	// Create and run the server
	try
	{
		Server ircServer(port, password, debug);
		ircServer.serverRun();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
