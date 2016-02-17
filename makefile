CFLAGS	=	-std=c++11 -Wall 

server:	server.cpp 
	g++ $(CFLAGS) server.cpp -o $@


client:	client.cpp 
	g++ $(CFLAGS) client.cpp -o client
