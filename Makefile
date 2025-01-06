# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: zsoltani <zsoltani@student.42lausanne.ch>  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/09/30 11:36:55 by wcorrea-          #+#    #+#              #
#    Updated: 2024/11/29 00:53:25 by zsoltani         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

SRC = src/main.cpp \
		src/Service_0.cpp \
		src/Service_1_setup.cpp \
		src/Service_2_launch.cpp \
		src/Parser_0.cpp \
		src/Parser_1_getConfig.cpp \
		src/Parser_2_checkServer.cpp \
		src/Parser_3_checkLocation.cpp \
		src/Server.cpp \
		src/Client.cpp \
		src/Client_Request.cpp \
		src/Client_Response.cpp \
		src/Client_File.cpp \
		src/Client_CGI.cpp \
		src/cgi.cpp \
		src/utils.cpp

OBJS = ${SRC:.cpp=.o}

INCLUDE = -I include

CC = c++
RM = rm -f
CPPFLAGS = -Wall -Wextra -Werror -pedantic -std=c++98 -g

.cpp.o:
		${CC} ${CPPFLAGS} ${INCLUDE} -c $< -o ${<:.cpp=.o}

$(NAME): ${OBJS}
		${CC} ${CPPFLAGS} ${INCLUDE} ${OBJS} -o ${NAME}

all:	${NAME}

clean:
		${RM} ${OBJS}

fclean:	clean
		${RM} ${NAME}

re: clean all

.PHONY: all clean fclean re