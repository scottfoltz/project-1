/*	Datagram sample server.
	CSC 3600 - Carthage College - Prof. Kivolowitz - Spring 2016
*/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <memory.h>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
using namespace std;

const unsigned short DEFAULT_PORT = 5077;
const int MAXIMUM_DATA_LENGTH = 500;

struct Packet{
char command;
unsigned int sequence_number;
unsigned short length;
timeval tv;
unsigned char payload[];
};

int main(int argc, char *argv[])
{
int udp_socket;
int server_port = DEFAULT_PORT;
//socklen_t client_length;
struct sockaddr_in server_sockaddr;
struct sockaddr_in client_sockaddr;

char c;

// Command line arguements
while ((c = getopt(argc, argv, "hp:")) != -1)
  {
switch (c)
  {
 case 'h':
cerr << argv[0] << "options:" << endl;
cerr << "	-h displays help" << endl;
cerr << "	-p port_number ... defaults to 4095" << endl;
exit(0);

 case 'p':
server_port	= atoi(optarg);
break;
}
}

// The UDP (DGRAM) socket is to be created in the INTERNET domain rather than
// on the local file system.
udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
if (udp_socket < 0)
  {
perror("ERROR opening socket");
exit(1);
}

// The next two commented lines are really  interesting. But why? It has something to
// with the size of Earth and the speed of light. But mostly, it has to do with worst
// case scenarios and picking a nice round number. We'll explore this later this term.
//int optval = 1;
//setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

// Structures associated with socket programming are often sparsely filled in.
// Therefore initializing the unused fields is critical. memset() is a good method.
memset(&server_sockaddr, 0, sizeof(server_sockaddr));

// We're going to bind() to all of the network adapters on this computer. This
// is the effect of INADDR_ANY. Notice the call to host-to-network-short (htons())
server_sockaddr.sin_family = AF_INET;
server_sockaddr.sin_addr.s_addr = INADDR_ANY;
server_sockaddr.sin_port = htons(server_port);
// This port is MINE! Pending error check, of course.
if (bind(udp_socket, (const struct sockaddr *) &server_sockaddr, socklen_t(sizeof(server_sockaddr))) < 0)
  {
close(udp_socket);
perror("ERROR on binding");
exit(1);
}

cout << "Socket bound on port: " << server_port << endl;

socklen_t client_length = sizeof(client_sockaddr);
//***************************************************************
//now, this is the part of the code in which we will have to make
//changes to the example code Perry has given us

Packet p;
size_t bytes_read;
size_t bytes_sent;
memset(&p, 0, sizeof(p));
bool contains_payload = false;
Packet send_back_packet;
struct timeval this_time;

while ((bytes_read = recvfrom(udp_socket, &p, sizeof(p) + p.length, 0, (struct sockaddr *) &client_sockaddr, &client_length)) >= 0)
  {

//we will now take apart the packet that has been recieved and
//parse out that meta-data (a struct called a packet)
//and either send back an 'A' or drop the packet.
switch(p.command){
 case 'D':
contains_payload = true;
break;
 case 'E':
exit(0);
break;
//the first two if's are checking for the command being recieved and that the packet was not too big
if(contains_payload){
if(!(bytes_read > (size_t)MAXIMUM_DATA_LENGTH)){
//if all our conditions are satisfied, then we can send back a packet that contains command 'A', and other info

send_back_packet.command = 'A';
send_back_packet.sequence_number = p.sequence_number;
gettimeofday(&this_time, nullptr);
send_back_packet.tv = this_time;
//now that we have packed our packet full of info, its time to send it on its journey
bytes_sent = sendto(udp_socket, (void *) &send_back_packet, sizeof(send_back_packet), 0, (struct sockaddr *) &client_sockaddr, sizeof(client_sockaddr));
if (bytes_sent != sizeof(p))
		{
			cerr << "Number of bytes sent [" << bytes_sent << "] does not match packet size." << endl;
			break;
		}
}
 else
   {
cout<< "The packet #" << p.sequence_number << " was dropped because it exceeded maximum packet size" << endl;
}
}
 else{
cout << "The packet #" << p.sequence_number << " was dropped because the command 'D' was not issued" << endl;
}
}
memset(&p, 0, sizeof(p));
}

close(udp_socket);
return 0;
}
