NAME		= webserv
CC			= c++
CFLAGS		= -Wall -Wextra -Werror -std=c++98
AR			= ar
ARFLAGS		= rs
ARFILE		= webserv.a

INCLUDES_HEADER  = -I src

SRCS		= src/Configuration.cpp \
					src/Request.cpp \
					src/RequestData.cpp \
					src/Response.cpp \
					src/Worker.cpp \

MAIN		= test/WorkerTest.cpp

OBJS		= $(SRCS:.cpp=.o) $(MAIN:.cpp=.o)

all: $(NAME)

$(NAME): $(ARFILE)
	$(CC) $(CCFLAGS) $^ -o $@

$(ARFILE): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.cpp
	$(CC) $(CCFLAGS) $(INCLUDES_HEADER) -c $^ -o $@

clean:
	$(RM) $(OBJS) $(ARFILE)

fclean: clean
	$(RM) $(NAME)

re: fclean all
