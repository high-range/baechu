NAME		= webserv
CC			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -g
AR			= ar
ARFLAGS		= rs
ARFILE		= webserv.a

INCLUDES_HEADER  = -Iinclude

SRCS		= src/ConfigHelper.cpp \
				src/Configuration.cpp \
				src/Connector.cpp \
				src/Manager.cpp \
				src/Request.cpp \
				src/RequestData.cpp \
				src/RequestUtility.cpp \
				src/Response.cpp \
				src/ResponseData.cpp \
				src/Worker.cpp \
				src/WorkerUtils.cpp \
				src/Utils.cpp \

MAIN		= main.cpp

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
