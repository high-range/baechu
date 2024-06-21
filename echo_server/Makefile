CXX = c++
CFLAGS = -Wall -Wextra -Werror
SRCC = client.cpp
SRCS = server.cpp
NAMEC = client
NAMES = server
OBJC = $(SRCC:.cpp=.o)
OBJS = $(SRCS:.cpp=.o)

all : $(NAMES) $(NAMEC)

$(NAMEC) : $(OBJC)
	$(CXX) $(CFLAGS) -o $(NAMEC) $(OBJC)

$(NAMES) : $(OBJS)
	$(CXX) $(CFLAGS) -o $(NAMES) $(OBJS)

$(OBJC) : $(SRCC)
	$(CXX) $(CFLAGS) -c $(<) -o $(@)

$(OBJS) : $(SRCS)
	$(CXX) $(CFLAGS) -c $(<) -o $(@)

clean:
	rm -f $(OBJC)
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAMEC)
	rm -f $(NAMES)

re:
	make fclean
	make all

.PHONY: all clean fclean re