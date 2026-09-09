# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fjose-hi <fjose-hi@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/08/17 21:06:09 by fjose-hi          #+#    #+#              #
#    Updated: 2026/09/09 16:48:41 by fjose-hi         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -I includes

SRCS =	src/main.c \
		src/parsing/parse.c \
        src/init/init_data_coders.c \
        src/init/init_dongles.c \
        src/heap/heap.c \
        src/heap/heap_utils.c \
        src/dongle/dongle_acquire.c \
        src/dongle/dongle_release.c \
		src/coder/coder_routine.c \
        src/coder/coder_progress.c \
        src/log/log.c \
        src/monitor/monitor.c \
        src/utils/clean_utils.c \
		src/utils/sim_state.c \
		src/utils/time_utils.c

OBJS = $(SRCS:.c=.o)

all: $(NAME) 

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	
clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean fclean re