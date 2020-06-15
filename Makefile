GREEN = \033[1;32m
WHITE = \033[0;m

NAME = server

CC = gcc -g

RM = rm -f

SRCS = server.c \
       pages_control.c \

THREAD_SRCS = thread_pool.c

OBJS = $(SRCS:.c=.o)
THREAD_OBJS = $(THREAD_SRCS:.c=.o)

LDFLAGS = -pthread

all: $(NAME)

$(NAME): $(OBJS) $(THREAD_OBJS)
	@$(CC) $^ -o $(NAME) $(LDFLAGS)
	@printf "\n[$(GREEN)OK$(WHITE)] Binary: $(NAME)\n"
	@echo "-------------------\n"

%.o: %.c %.h
	@$(CC) -c -o $@ $< 

clean:
	$(RM) $(OBJS)
