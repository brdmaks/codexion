# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: seledka <seledka@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/09/09 15:30:00 by seledka           #+#    #+#              #
#    Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread -Iincludes

SRCDIR		= src
OBJDIR		= obj

SRCS		= $(SRCDIR)/main.c \
			  $(SRCDIR)/parse.c \
			  $(SRCDIR)/init.c \
			  $(SRCDIR)/cleanup.c \
			  $(SRCDIR)/time.c \
			  $(SRCDIR)/heap.c \
			  $(SRCDIR)/heap_order.c \
			  $(SRCDIR)/dispatch.c \
			  $(SRCDIR)/coder.c \
			  $(SRCDIR)/monitor.c \
			  $(SRCDIR)/log.c

OBJS		= $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.c includes/codexion.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
