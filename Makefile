# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: dsy <marvin@42.fr>                         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/02 19:35:10 by dsy               #+#    #+#              #
#    Updated: 2023/05/02 19:35:11 by dsy              ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#---------------------------------VARIABLES------------------------------------#

BWHITE =	\033[1;97m
GREEN =		\033[0;32m
GREY =		\033[90m
RESET =		\033[0m

CC =			c++
CFLAGS =		-Wall -Wextra -Werror -std=c++98 -g3 -fsanitize=address
NAME =			ircserver
BUILD_PATH =	build/

SRCS = src/main.cpp src/Server/AServer.cpp src/Server/IrcServer.cpp		\
       src/Utils/Utils.cpp src/User/User.cpp src/Channel/Channel.cpp	\
	   src/Server/IrcServer_Connections.cpp

HEADERS = include/IrcServer.hpp include/AServer.hpp include/Utils.hpp	\
          include/User.hpp include/Channel.hpp include/_defines.hpp


OBJS = $(addprefix $(BUILD_PATH), $(notdir $(SRCS:.cpp=.o)))

#-------------------------------MAKEFILE TOOLS---------------------------------#

# VPATH Needed to build objects in the /build folder
VPATH = src src/Server src/Utils src/User src/Channel

#-----------------------------------RULES--------------------------------------#

# OBJECTs pattern rule : builds all objects with headers and cpp files as dep.
$(BUILD_PATH)%.o: %.cpp $(HEADERS)
	@mkdir -p $(BUILD_PATH)
	@echo "$(GREY)Compiling...$(RESET)                $(WHITE)$(RESET)$<"
	@$(CC) $(CFLAGS) -Iinclude -c $< -o $@

# RELINK guard : checks if NAME exists with objects as dep.
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@echo "Ready : ${GREEN}`pwd`/${NAME}${RESET}"

#stty -echoctl : disable echo for control characters like 'ˆC' on Ctrl+C
all: $(NAME) client
	@stty -echoctl 
	@echo "Ready : ${GREEN}`pwd`/${NAME}${RESET}"

clean:
	@rm -f $(OBJS)

fclean:
	@rm -rf $(BUILD_PATH) $(NAME)

re: fclean all

.PHONY: all client clean fclean re