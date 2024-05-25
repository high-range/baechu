NAME		= webserv
CC			= c++
CFLAGS		= -Wall -Wextra -Werror -std=c++98
AR			= ar
ARFLAGS		= rs
ARFILE		= webserv.a

INCLUDES	= src/Configuration.hpp \
					src/Request.hpp \
					src/Response.hpp \
					src/Worker.hpp \

SRCS		= src/main.cpp \
					src/Worker.cpp \

OBJS		= $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(ARFILE)
	$(CC) $(CCFLAGS) $^ -o $@

$(ARFILE): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.cpp
	$(CC) $(CCFLAGS) -c $^ -o $@

clean:
	$(RM) $(OBJS) $(ARFILE)

fclean: clean
	$(RM) $(NAME)

re: fclean all
