NAME			= codexion

CC				= cc
CFLAGS			= -Wall -Werror -Wextra -pthread -MMD -MP
OBJDIR			= objs

MANDATORY_SRCS	= parsing.c	\
				  coders.c	\
				  utils.c	\
				  heap.c	\
				  init.c	\
				  dongle.c	\
				  main.c

MANDATORY_OBJS	= $(MANDATORY_SRCS:%.c=$(OBJDIR)/%.o)

DEPFILES		= $(MANDATORY_OBJS:.o=.d)

all: $(NAME)

$(NAME): $(MANDATORY_OBJS)
	$(CC) $(CFLAGS) $(MANDATORY_OBJS) -o $(NAME) 

$(OBJDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
	rm -f $(DEPFILES)

fclean: clean
	rm -f $(NAME) $(BONUS_NAME)

re: fclean all

-include $(DEPFILES)

.PHONY: all clean fclean re
