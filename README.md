_This project has been created as part of the 42 curriculum by mael-hou._

# ft_irc - Internet Relay Chat Server

<p align="center">
  <img src="https://img.shields.io/badge/Language-C++98-00599C?logo=c%2B%2B" alt="Language">
  <img src="https://img.shields.io/badge/Network-Non--blocking_I%2FO-brightgreen" alt="Network">
  <img src="https://img.shields.io/badge/Multiplexing-poll()-orange" alt="Multiplexing">
  <img src="https://img.shields.io/badge/RFC-1459_%2F_2812-blueviolet" alt="RFC">
  <img src="https://img.shields.io/badge/Evaluation-Ready-success" alt="Status">
</p>

## Description

**ft_irc** is a custom Internet Relay Chat (IRC) server developed from scratch in **C++ 98** as part of the 42 school curriculum. The project implements RFC 1459 and RFC 2812 protocols, providing real-time text-based group and private messaging.

The server is built with a single-threaded, fully non-blocking I/O architecture using `poll()` to multiplex incoming events from hundreds of clients simultaneously without thread contention, forking, or blocking.

### Project Goal

Build a fully functioning IRC server capable of interacting seamlessly with official IRC clients (such as Irssi, HexChat, and WeeChat) while strictly conforming to the 42 project rules:

- **Single event loop**: Only 1 `poll()` (or equivalent) handles all network read/write and listening operations.
- **Non-blocking sockets**: Sockets are configured using `fcntl(fd, F_SETFL, O_NONBLOCK)` with zero blocking I/O calls.
- **Packet reassembly**: Proper aggregation of partial TCP packets (e.g., when packets are sent chunked via `nc -C`).
- **No forking**: Entire server is managed in a single synchronous event-driven process.
- **Zero crashes & zero memory leaks**: Resilient to out-of-order commands, dropped connections, and malformed inputs.

This project covers fundamental concepts in:

- **Network programming in C++**: TCP/IP sockets, `bind()`, `listen()`, `accept()`, `send()`, and `recv()`
- **I/O Multiplexing**: Event-driven architecture with `poll(2)`
- **Protocol Engineering**: RFC 1459 and RFC 2812 command parsing and numeric replies
- **State Management**: Channel lifecycle, operator permissions, user registration handshake
- **Defensive C++98 Programming**: Resource acquisition, canonical class design, and const-correctness

### Features

#### Mandatory Part

- ✅ Multiplexed non-blocking server handling multiple clients simultaneously
- ✅ 3-step client authentication handshake (`PASS`, `NICK`, `USER`)
- ✅ Channel creation and membership management (`JOIN`, `PART`, `QUIT`)
- ✅ Channel message broadcasting and private messaging (`PRIVMSG`)
- ✅ Operator privilege system (channel founders and promoted users)
- ✅ Operator channel commands:
  - `KICK` — Eject a user from a channel
  - `INVITE` — Invite a user to a channel
  - `TOPIC` — View or modify the channel topic
  - `MODE` — Manage channel configuration:
    - `+i` / `-i`: Set/remove Invite-only mode
    - `+t` / `-t`: Restrict `TOPIC` editing to channel operators
    - `+k` / `-k`: Set/remove channel key (password)
    - `+o` / `-o`: Grant/revoke channel operator privileges
    - `+l` / `-l`: Set/remove user limit on a channel
- ✅ Partial TCP packet aggregation (passing the `nc -C` Ctrl+D split-packet test)
- ✅ Clean server shutdown on `SIGINT` (Ctrl+C) with complete resource deallocation

---

## Instructions

### Prerequisites

To compile and run the project, ensure you have:

```bash
# On Linux / Ubuntu / WSL
sudo apt-get update
sudo apt-get install build-essential g++ make

# On macOS
xcode-select --install
```

### Compilation

Build the `ircserv` executable using the provided `Makefile`:

```bash
# Compile the server
make

# Clean object files
make clean

# Remove object files and the executable
make fclean

# Rebuild from scratch
make re
```

The Makefile compiles using `c++` with the mandatory flags:
`-Wall -Wextra -Werror -std=c++98`.

### Execution

Launch the server by passing the listening port and a network password:

```bash
./ircserv <port> <password>
```

**Example:**
```bash
./ircserv 6667 mypassword123
```

To enable verbose logging of incoming bytes, events, and command dispatches, add the `--debug` flag:
```bash
./ircserv 6667 mypassword123 --debug
```

---

## Connecting with Clients

You can connect to your server using any standard IRC client or raw TCP utilities:

### 1. Using `nc` (Netcat)

```bash
nc -C 127.0.0.1 6667
```

Once connected, complete the authentication handshake:
```irc
PASS mypassword123
NICK alice
USER alice 0 * :Alice Liddell
```

### 2. Using `irssi`

```bash
irssi -c 127.0.0.1 -p 6667 -w mypassword123 -n alice
```

Within Irssi:
```irc
/join #general
/msg #general Hello everyone!
/topic #general Welcome to ft_irc!
```

### 3. Using `HexChat`

1. Open HexChat → **Network List** → **Add** new network.
2. Edit network:
   - Server: `127.0.0.1/6667`
   - Password: `mypassword123`
3. Connect and chat!

---

## Supported Commands Reference

| Command | Syntax | Description | Operator Only |
| :--- | :--- | :--- | :---: |
| **`PASS`** | `PASS <password>` | Authenticates client with the server password | No |
| **`NICK`** | `NICK <nickname>` | Sets or changes the user's nickname | No |
| **`USER`** | `USER <username> <hostname> <server> :<realname>` | Completes user registration | No |
| **`JOIN`** | `JOIN <#channel> [<password>]` | Creates or joins a channel | No |
| **`PRIVMSG`** | `PRIVMSG <target> :<message>` | Sends a private message to a user or channel | No |
| **`PING`** | `PING :<token>` | Responds with `PONG` to verify connection liveness | No |
| **`TOPIC`** | `TOPIC <#channel> [:<new topic>]` | Views or sets channel topic (respects `+t` mode) | If `+t` |
| **`INVITE`** | `INVITE <nickname> <#channel>` | Invites a user to a channel | If `+i` |
| **`KICK`** | `KICK <#channel> <nickname> [:<reason>]` | Ejects a user from a channel | **Yes** |
| **`MODE`** | `MODE <#channel> <modes> [<args>]` | Configures channel flags (`i`, `t`, `k`, `o`, `l`) | **Yes** |
| **`QUIT`** | `QUIT [:<reason>]` | Disconnects client and cleans up channel states | No |

### Channel Modes (`MODE`)

| Flag | Argument | Description |
| :---: | :---: | :--- |
| **`+i` / `-i`** | *none* | Sets / removes invite-only restriction |
| **`+t` / `-t`** | *none* | Restricts `TOPIC` modification to channel operators |
| **`+k` / `-k`** | `<key>` | Sets / removes channel password (key) |
| **`+o` / `-o`** | `<nickname>` | Grants / revokes channel operator privileges |
| **`+l` / `-l`** | `<limit>` | Sets / removes user limit on the channel |

---

## Project Structure

```
ft_irc/
├── Makefile                          # Clean build system with color diagnostics
├── README.md                         # Project documentation and evaluation guide
├── src/
│   ├── main.cpp                      # Argument validation, signal handling, entry point
│   ├── Client/
│   │   ├── Client.hpp                # Client class declaration
│   │   └── Client.cpp                # Socket state, packet buffers, authentication flags
│   ├── Channel/
│   │   ├── Channel.hpp               # Channel class declaration & mode flags
│   │   └── Channel.cpp               # Member maps, operator privileges, broadcasting
│   ├── Server/
│   │   ├── Server.hpp                # Server class declaration & pollfd registry
│   │   ├── Server.cpp                # Socket setup, poll() event loop, disconnect logic
│   │   ├── commandHandler.cpp        # Command lexing, uppercase parsing, dispatching
│   │   └── commands/
│   │       ├── cap.cpp               # CAP negotiation handler
│   │       ├── pass.cpp              # PASS authentication handler
│   │       ├── nick.cpp              # NICK uniqueness & registration handler
│   │       ├── user.cpp              # USER realname parsing & registration handler
│   │       ├── privmsg.cpp           # Direct & channel message routing
│   │       ├── join.cpp              # Channel creation, key/limit/invite validation
│   │       ├── mode.cpp              # Channel mode parser (+itkol)
│   │       ├── topic.cpp             # Channel topic view & modification (+t guard)
│   │       ├── invite.cpp            # Channel invitation & target notification
│   │       ├── kick.cpp              # Operator kick execution & state removal
│   │       └── quit.cpp              # Peer notification & empty channel cleanup
│   └── utils/
│       ├── IRCmacros.hpp             # Numeric replies & RFC error codes
│       └── stdoutColors.hpp          # Terminal ANSI color macros
```

---

## Implementation & Architecture

### 1. Single Non-Blocking `poll()` Loop

Per the 42 subject requirements, the server avoids threads and forking completely:
- The listening server socket and all accepted client sockets are configured with `fcntl(fd, F_SETFL, O_NONBLOCK)`.
- All descriptors are registered into a single `std::vector<pollfd>` array.
- The `poll()` call operates with a short timeout (100ms) to ensure non-blocking execution with instant signal responsiveness, avoiding 100% CPU thread-spinning.

### 2. TCP Packet Reassembly (nc Ctrl+D Test)

Because TCP is a stream-oriented protocol without inherent message boundaries, commands can arrive split across multiple packets or merged together in a single read:
- Each `Client` maintains an internal `_buffer` string.
- Upon receiving `POLLIN`, newly read bytes are appended to the buffer.
- The server extracts full commands by finding delimiter sequences (`\n` / `\r\n`).
- Commands are processed sequentially, and incomplete chunks remain buffered until subsequent packets arrive.

### 3. Channel Lifecycle & Mode Architecture

- Channels are instantiated dynamically upon the first `JOIN` command and their creator is automatically granted operator privileges (`@`).
- Active modes are tracked efficiently using a bitmask (`MD_INV`, `MD_TOPIC`, `MD_PASSWORD`, `MD_USR_LIM`).
- When all members leave or disconnect, channels are safely deallocated to guarantee zero memory leaks.

### 4. Memory & Resource Safety

- Dedicated cleanup handlers ensure that when a client abruptly disconnects, its socket is closed, its memory freed, and all relevant channels are notified with a proper `QUIT` broadcast.
- The listening socket and client allocations are reclaimed cleanly during server shutdown (`SIGINT`).

---

## Resources

### Documentation

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 — Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [Modern IRC Client Protocol Specification](https://modern.ircdocs.horse/)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- Linux Manual Pages: `poll(2)`, `fcntl(2)`, `socket(2)`, `bind(2)`, `listen(2)`, `accept(2)`

### AI Usage Disclosure

In compliance with Chapter III of the 42 subject, Artificial Intelligence was utilized as a technical assistant:

#### Code Development
- **Bug Discovery & Diagnostic**: AI helped identify compilation issues, unclosed method definitions, and potential file descriptor leaks.
- **Protocol Verification**: Validating numeric reply codes against RFC 2812 specifications.
- **Portability Review**: Ensuring strict compatibility with the `-std=c++98` standard.

#### Documentation
- **Technical Writing**: Structuring and formatting documentation for clear peer-evaluation defense.
- **Code Comments**: Writing concise English explanatory docstrings across the codebase.

#### Learning
- **Network Multiplexing**: Deepening understanding of edge-triggered vs level-triggered socket polling.
- **Non-blocking TCP Streams**: Designing robust packet accumulation state machines.

*Note: All architecture decisions, implementation logic, and code adjustments were fully reviewed, verified, and tested.*

---

## Testing

The project has been tested through end-to-end integration test suites:

```bash
# 1. Compile the server
make

# 2. Launch the server in terminal 1
./ircserv 6667 password123

# 3. Test partial packet reassembly with netcat in terminal 2
nc -C 127.0.0.1 6667
PA[ctrl+D]SS password123[Enter]
NI[ctrl+D]CK tester[Enter]
USER tester 0 * :Tester[Enter]
JOIN #test[Enter]
```

---

## Author

**mael-hou** - [42 Intra Profile](https://profile.intra.42.fr/users/mael-hou)

_ft_irc — 42 School Curriculum_
