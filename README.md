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

### How to run it

- `make` will generate the executable. The Makefile handles *relink*, *header dependencies* and places objects in a */build* folder. 
- The usage is `./ircserver <port> <password>`, but `password` authentication isn't implemented yet.
- Once the server is running in a terminal, open another terminal and enter `nc <ipaddr> <port>`, followed by whatever you want to send requests to the server, command parsing isn't implemented yet. `irssi` will not be able to send requests to your ircserver until parsing is implemented, but you can still connect by using `/connect <ipaddr> <port>` inside the client. `ipaddr` will always be `localhost`, but `port` can be any free port.
- To exit the server, you can hit `Ctrl + C` and the program will cleanly exit, however later on the only ways to exit the server will be sending a `SIGTERM` from another terminal or typing `/exit`.

For a detailed visualization of the FT_IRC server project, you can refer to the [Miro board](https://miro.com/app/board/uXjVMz5U6PI=/?share_link_id=179689716548).

![image](https://github.com/Djbrl/ft_irc/assets/29091732/f4badb7e-bd3d-4aba-a15f-4fe2b1cf438b)

### IrcServer

The `IrcServer` class represents the core of the FT_IRC server. It handles client connections, message processing, and user management. The server listens for incoming connections, establishes data sockets with clients, and responds to IRC commands. The server uses the poll(or equivalent) system call for efficient event handling and non-blocking I/O so we can serve multiple clients at once.

### Channel

The `Channel` class represents an IRC channel, where users can join and exchange messages. The class manages the users in the channel, broadcasts messages to all users, and handles channel-specific commands.

### User

The `User` class represents an IRC user connected to the server. It stores user-specific information, such as nickname and connection status. Users can join channels, send and receive messages, and interact with other users on the server.

## Branching & Implementation Strategy

For this project I recommend settings weekly goal-oriented [sprints](https://github.com/Djbrl/ft_irc/commit/3e75b711564c96aa0ce4d978e64f5f13b42832e9) and working on feature-branches. Once a feature is implemented and fully tested and functional, it can be merged onto the main branch.

