# FT_IRC Onboarding Document

## Description

FT_IRC is an IRC (Internet Relay Chat) **server implementation** that allows **users** to connect to the server using an IRC client and communicate with each other **using channels and direct messaging** in real-time, by using sockets.

---

### Read This First
- [Subject](https://github.com/Djbrl/ft_irc/blob/main/en.subject.pdf)
- [Basic socket communication tutorial](https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa)

### Useful Resources
- [IRC Protocol Overview](https://tools.ietf.org/html/rfc2810)
- [Internet Relay Chat (IRC) - Wikipedia](https://en.wikipedia.org/wiki/Internet_Relay_Chat)
- ["IRC - Things to know" by a 42 student ](https://ircgod.com/docs/irc/to_know/)
- [ChatGPT to quickly digest all of the above !](https://chat.openai.com)

---
## Project Overview

![image](https://github.com/Djbrl/ft_irc/assets/29091732/f816c94e-5a23-47a8-9f35-95583164a733)

### IrcServer

The `IrcServer` class represents the core of the FT_IRC server. It handles client connections, message processing, and user management. The server listens for incoming connections, establishes data sockets with clients, and responds to IRC commands. The server uses the poll(or equivalent) system call for efficient event handling and non-blocking I/O so we can serve multiple clients at once.

### Channel

The `Channel` class represents an IRC channel, where users can join and exchange messages. The class manages the users in the channel, broadcasts messages to all users, and handles channel-specific commands.

### User

The `User` class represents an IRC user connected to the server. It stores user-specific information, such as nickname and connection status. Users can join channels, send and receive messages, and interact with other users on the server.

## Project Visualization

For a detailed visualization of the FT_IRC server project, you can refer to the [Miro board](https://miro.com/app/board/uXjVMz5U6PI=/?share_link_id=179689716548).

![image](https://github.com/Djbrl/ft_irc/assets/29091732/f4badb7e-bd3d-4aba-a15f-4fe2b1cf438b)


