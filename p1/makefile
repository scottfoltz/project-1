CFLAGS	=	-std=c++11 -Wall 

all: server.cpp client.cpp
	make server client

server:	server.cpp 
	g++ $(CFLAGS) server.cpp -o $@


client:	client.cpp 
	g++ $(CFLAGS) client.cpp -o client

