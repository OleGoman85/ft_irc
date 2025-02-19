

# ft_irc 🐲🔥

**ft_irc** is the most ultracool IRC server ever built in **C++17**! It’s an unstoppable force of networking mastery  capable of real-time messaging, dynamic channel management, a file transfer feature, and a life-altering bot that divides your time into “Before Bot” and “After Bot.” Seriously... it can even tell you the time! 

---

## Table of contents

- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Usage \& testing](#usage--testing)
- [Commands](#commands)
  - [Client commands](#client-commands)
  - [Channel \& operator commands](#channel--operator-commands)
  - [Bot commands](#bot-commands)
  - [File transfer commands](#file-transfer-commands)
- [Architecture](#architecture)
- [Authors](#authors)


---

## Introduction

**ft_irc combines the raw power of C++17 with the agility of a martial arts master. 🥋  
It uses a single poll loop and non-blocking I/O to efficiently handle all connected clients quickly and smoothly—like gliding through a bamboo forest, but without the splinters.**

---

## Features

- **Real-time messaging:** Instant private and public communication, faster than a flying kick.
- **User authentication:** Guard your server with PASS, NICK, and USER commands.
- **Channel management:** Create, join, and manage channels with smooth mastery.
- **Operator commands:** KICK, INVITE, TOPIC, and channel MODEs for total control.
- **File transfer:** A purely educational challenge you can show off, though in the real world DCC is recommended.
- **Life-changing bot:** Need jokes? Facts? Magic 8-ball wisdom? Dice rolls absolutely necessary for everyday existence? It’s all here. You’ll never look at IRC the same way again!

---

## Installation

### Prerequisites

- A **C++17** compatible compiler (e.g., g++ 7 or later)
- GNU Make
- A Unix-like environment (Linux, macOS, or WSL)

### Compilation

Clone the repository:

```bash
git clone https://github.com/OleGoman85/ft_irc.git
cd ft_irc
```

Build the project with:

```bash
make
```

An executable named `ircserv` will appear.

---

## Usage & testing

Start the server by specifying a port and password:

```bash
./ircserv 6667 mysecretpassword
```

Connect via:

- **Netcat (nc):**
  ```bash
  nc 127.0.0.1 6667
  ```
  Then enter commands like `PASS mysecretpassword`, `NICK MyNick`, etc.

- **IRSSI:**
  ```bash
  irssi -c localhost -p 6667 -w mysecretpassword
  ```

Feel free to use any other IRC client you prefer. Now walk the path of the unstoppable chat warrior!

---

## Commands

### Client commands

- **PASS `<password>`**  
  Authenticate with the server.

- **NICK `<nickname>`**  
  Set your nickname.

- **USER `<username> <hostname> <servername> <realname>`**  
  Register your user information.

- **JOIN `<#channel>`**  
  Join a channel.

- **PART `<#channel>`**  
  Leave a channel.

- **PRIVMSG `<target>` `:<message>`**  
  Send a message to a user or a channel.

- **QUIT**  
  Disconnect gracefully.

---

### Channel & operator commands

- **KICK `<#channel> <nickname>`**  
  Boot a user right out of the channel.  
- **INVITE `<nickname> <#channel>`**  
  Invite a user into a channel.  
- **TOPIC `<#channel> [new topic]`**  
  Set or view the channel topic.  
- **MODE `<#channel> [flags and parameters]`**  
  Adjust channel modes in any combination, for total equilibrium:
  - `+i` — Invite-only channel.
  - `+t` — Only operators can set the topic.
  - `+k <secretkey>` — Set a channel password.
  - `-k` — Remove the channel password.
  - `+l <limit>` — Set a user limit (e.g., `+l 10`).
  - `-l` — Remove the user limit.
  - `+o <nickname>` — Give operator privileges to a user.

---

### Bot commands

All bot commands start with `BOT`. Prepare for your life to be changed!

- **BOT HELP**  
  Shows all available bot commands.
- **BOT ROLL `[NdM]`**  
Roll dice (e.g., `BOT ROLL 2d20`). The first number (`N`) is the number of dice, and the second (`M`) is the number of sides on each die. This tool is absolutely vital for daily survival. If you don’t believe us, just try living a day without it!
- **BOT 8BALL `<question>`**  
  Consult the mystical 8-ball for cosmic wisdom.
- **BOT JOKE**  
  Get a sparkly, witty joke to brighten your day.
- **BOT FACT**  
  Receive a random piece of knowledge that might blow your mind.
- **BOT TIME**  
  Find out the server’s local time with just one command.

---

### File transfer commands

Use a custom protocol to test out non-blocking file transfers:

- **FILE SEND `<nickname> <filename> <filesize>`**  
  Initiate a file transfer to a specific user.
- **FILE DATA `<filename> <base64_chunk>`**  
  Transmit a portion of the file, base64-encoded.
- **FILE END `<filename>`**  
  Conclude the file transfer.

> *These commands are purely for demonstration purposes. In the real world, DCC remains the true kung fu master of file transfers, but our way has more fun and fewer ancient scrolls involved!*

---

## Architecture

- **Server Module:** Handles client connections, non-blocking I/O, and the main poll loop.  
- **Client Module:** Manages user state, buffers, and authentication progress.  
- **Channel Module:** Tracks membership, topics, and operator privileges.  
- **Command Modules:** Execute and parse IRC commands, file transfers, and bot interactions.  
- **Utilities:** Provide string handling, timestamps, and base64 decoding.

---

## Authors

**ft_irc** is the proud effort of:
- [Alisa Arbenina](https://github.com/aarbenin) — A software developer whose greatness is matched only by her infinite awesomeness 😎✨
- [Oleg Goman](https://github.com/OleGoman85) — A bald dude who codes fearlessly. 🐉


Now harness your unstoppable IRC might! Go forth, chat like a champion, and remember: true power comes from those who love their code. 🥠