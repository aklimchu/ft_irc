# 🚀 ft_irc 

![C++](https://img.shields.io/badge/C++-20-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Status](https://img.shields.io/badge/status-complete-brightgreen.svg)

`ft_irc` is an Internet Relay Chat (IRC) server implemented in C++20 as part of the 42 school curriculum. This project focuses on creating a robust, standards-compliant IRC server that supports multiple simultaneous client connections, non-blocking I/O operations, and core IRC functionalities. The server adheres to the IRC protocol (primarily RFC 2812) and is designed to work seamlessly with official IRC clients such as **irssi** 📡.

## 📚 Table of Contents
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Supported Commands](#supported-commands)
- [Testing](#testing)
- [License](#license)

## 🌟 Features 
- **Multi-client Support**: Handles multiple client connections simultaneously using a single `poll()` for non-blocking I/O.
- **TCP/IP Communication**: Utilizes TCP/IP (IPv4) for reliable client-server communication.
- **Authentication**: Supports password-based connection authentication 🔒.
- **Core IRC Functionalities**:
  - Nickname and username registration.
  - Channel creation, joining, and management.
  - Private messaging and channel broadcasting.
  - Operator privileges and operator-specific commands.
- **Standards Compliance**: Implements key IRC commands and replies as per RFC 1459 and RFC 2812.
- **Error Handling**: Provides appropriate error replies for invalid commands or unauthorized actions.
- **Reference Client**: Tested for compatibility with **irssi**, ensuring a seamless user experience.

## 📋 Requirements 
- **Compiler**: A C++20-compliant compiler (e.g., `clang++` or `g++`).
- **Operating System**: Linux or macOS (developed and tested on Linux).
- **Dependencies**: Standard C++ library and POSIX socket libraries (no external dependencies).
- **IRC Client**: An IRC client like **irssi** or **nc** (netcat) for testing.
- **Build Tool**: `make` for compiling the project.

## 🛠️ Installation 
1. **Clone the Repository**:
   ```bash
   git clone https://github.com/aklimchu/ft_irc.git
   cd ft_irc
   ```

2. **Compile the Project**:
   Run the following command to build the server:
   ```bash
   make
   ```
   This will generate the `ircserv` executable in the project directory. The Makefile ensures compilation with `-Wall -Wextra -Werror` flags for robust code.

3. **Clean Up (Optional)**:
   - Remove object files: `make clean`
   - Remove object files and executable: `make fclean`
   - Recompile from scratch: `make re`

## 📲 Usage 
1. **Start the Server**:
   Run the server with a specified port and password:
   ```bash
   ./ircserv <port> <password>
   ```
   - `<port>`: A valid port number (e.g., 6667, must be between 1 and 65535).
   - `<password>`: The connection password required for clients to connect.

   Example:
   ```bash
   ./ircserv 6667 mypassword
   ```

2. **Connect with an IRC Client**:
   Use an IRC client like **irssi** to connect to the server:
   ```bash
   irssi
   ```
   In irssi, connect to the server:
   ```
   /connect localhost 6667 mypassword
   ```
   Alternatively, use **netcat** for basic testing:
   ```bash
   nc -C localhost 6667
   ```
   Then manually send IRC commands (e.g., `PASS mypassword`, `NICK mynick`, `USER user 0 * :Real Name`).

3. **Interact with the Server**:
   Once connected, use standard IRC commands to authenticate, join channels, send messages, and manage the server. See [Supported Commands](#supported-commands) for details.

## 📜 Supported Commands 
The server implements the following IRC commands (based on RFC 1459/2812 and project requirements):

| Command       | Description                                                                 |
|---------------|-----------------------------------------------------------------------------|
| `PASS`        | Sets the connection password for authentication.                            |
| `NICK`        | Sets or changes the client's nickname.                                      |
| `USER`        | Specifies the username and real name for a new user.                        |
| `JOIN`        | Joins a specified channel (creates it if it doesn't exist).                 |
| `PART`        | Leaves a specified channel.                                                |
| `PRIVMSG`     | Sends private messages to a user or channel.                                |
| `MODE`        | Sets or removes channel/user modes (e.g., `+i`, `+t`, `+k`, `+o`, `+l`).    |
| `KICK`        | Removes a user from a channel (operator-only).                             |
| `INVITE`      | Invites a user to a channel (operator-only).                               |
| `TOPIC`       | Sets or views the topic of a channel.                                      |
| `QUIT`        | Terminates the client’s connection to the server.                          |
| `PING`        | Checks connection latency with the server.                                  |

**Channel Modes**:
- `i`: Invite-only channel.
- `t`: Restrict topic changes to operators.
- `k`: Set a channel key (password).
- `o`: Grant/revoke operator privileges.
- `l`: Set a user limit for the channel.

**User Modes**:
- `i`: Invisible mode.
- `o`: Operator status.

## 🧪 Testing 
- **Basic Testing with Netcat**:
  Connect using `nc -C localhost <port>` and send raw IRC commands to verify server responses. Example:
  ```
  PASS mypassword
  NICK mynick
  USER user 0 * :Real Name
  JOIN #mychannel
  PRIVMSG #mychannel :Hello, everyone!
  ```

- **Comprehensive Testing with Irssi**:
  Use **irssi** to test all features, including channel management, operator commands, and message broadcasting. Ensure replies match the expected format for an official IRC server.

- **Valgrind**:
  Check for memory leaks:
  ```bash
   valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./ircserv 6667 mypassword
   ```
  
📄 ## License 
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
