NAME		= webserv
CC			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
AR			= ar
ARFLAGS		= rs
ARFILE		= webserv.a

INCLUDES_HEADER  = -I src

SRCS		= src/server/Connector.cpp \
					src/Configuration.cpp \
					src/Manager.cpp \
					src/Request.cpp \
					src/RequestData.cpp \
					src/Response.cpp \
					src/Worker.cpp \

MAIN		= test/ConnectionTestServer.cpp

OBJS		= $(SRCS:.cpp=.o) $(MAIN:.cpp=.o)

all: $(NAME)

$(NAME): $(ARFILE)
	$(CC) $(CXXFLAGS) $^ -o $@

$(ARFILE): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.cpp
	$(CC) $(CXXFLAGS) $(INCLUDES_HEADER) -c $^ -o $@

clean:
	$(RM) $(OBJS) $(ARFILE)

fclean: clean
	$(RM) $(NAME)

re: fclean all
