/*	Datagram sample client.
	CSC 3600 - Carthage College - Prof. Kivolowitz - Spring 2016
*/
#include <sys/types.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unordered_set>
#include <time.h>

const unsigned short DEFAULT_PORT = 5077;
const int MAXIMUM_DATA_LENGTH = 500;
//#define MSG_NOBLOCK 0x01
struct Packet
{
	char command;
	unsigned int sequence_number;
	unsigned short length;
	struct timeval tvSec, tvUsec;


	// Data begins here
	unsigned char payload[];	
};

bool getAcknowledgementPacket(Packet& m, struct sockaddr_in& server_sockaddr, int& udp_socket);
using namespace std;
void printResults(unordered_set <int>& droppedPackets, int totalPackets);

// busy_wait will wait the specified number of microseconds before returning.
// I'd love to say more but what I'd say is posed as questions in P1's 
// specification.
void busy_wait(unsigned int microseconds)
{
	struct timeval start, now;
	double elapsed_time;
	
	gettimeofday(&start, nullptr);
	do 
	{
		gettimeofday(&now, nullptr);
		elapsed_time = (now.tv_sec - start.tv_sec) * 1000000.0;
		elapsed_time += (now.tv_usec - start.tv_usec);	
	} while (elapsed_time < microseconds);
}

int main(int argc, char *argv[])
{
  	cout << "start of main!" << endl;
    unsigned int uinterval = 1000;
	int udp_socket;
	int server_port = DEFAULT_PORT;
	int number_of_packets = 10000;
	bool userExit = false;
	//int userExitInt = 0;
	//Ammount of time the client will run before auto exiting (in seconds)
	int timeToRun = 1000;
	char * server_address = (char *) "127.0.0.1";
	unsigned short uLength;
	
	struct sockaddr_in server_sockaddr;
	struct hostent * server_hostent;

	char c;

	// Process command line options using getopt. Note that getopt makes use of an
	// externally defined char pointer "optarg". Note that the use of atoi() is
	// risky in general but suffices here. I would ward you away from using it in
	// production code.
	cout << "Before while loop!" << endl;
	while ((c = getopt(argc, argv, "hs:n:r:p:t:ed:")) != -1)
	{
	  cout << "Before switch cases!" << endl;
		switch (c)
		{
			case 'h':
				cerr << argv[0] << "options:" << endl;
				cerr << "	-h displays help" << endl;
				cerr << "	-s server_address ... defaults to 127.0.0.1" << endl;
				cerr << "	-p port_number ... defaults to 4095" << endl;
				cerr << "	-n number_of_packets ... defaults to " << number_of_packets << endl;
				cerr << "   -r delay in microseconds ... defaults to " << uinterval << endl;
				cerr << "   -t timeToRun ... defaults to " << timeToRun << endl;
				cerr << "   -e sends an E pack before exiting to the server " << endl;
				cerr << "   -d sets packet length" << endl;
                exit(0);

			case 's':
				server_address = optarg;
				break;

			case 'n':
				number_of_packets = atoi(optarg);
				break;
				//Ammount of time between each packet being sent
			case 'r':
				uinterval = atoi(optarg);
				break;
				
			case 'p':
				server_port	= atoi(optarg);
				break;
			case 't':
				timeToRun = atoi(optarg);
				break;
			case 'e':
			//Perhaps use this bool to check at the end of the client whether to send e packet
				userExit = true;
				break;
			case 'd':
				uLength = atoi(optarg);
				break;
	              	default:
		                break;

		}
	}

//Testing exit
//Doesnt seem to be getting the value I enter


	// Create a UDP (DGRAM) socket and check the return value.
	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket < 0)
	{
		// perror makes use of "errno" to provide an error message meaningful
		// to humans. It emits the string you provode as a prefix to the error message.
		perror("ERROR opening socket");
		exit(1);
	}
	cout << "Made it to line 134" << endl;
	// gethostbyname() may cause a DNS lookup to translate a URI into an IPv4 address.
	// Note that gethostbyname() is deprecated but most sample programs use it. The
	// function has been replaced by getaddrinfo() which is WAY more complicated. Maybe
	// it is for the best that we stick with gethostbyname(). 
	server_hostent = gethostbyname(server_address);
	if (server_hostent == nullptr)
	{
		cerr << "ERROR, no such host: " << server_address << endl;
		exit(1);
	}

	// Zero out the entire server_sockaddr. Note that many sample programs use the
	// less portable bzero(). memset() is preferred.
	memset(&server_sockaddr, 0, sizeof(server_sockaddr));
	
	// The family specifies an INTERNET socket rather than a file system socket.
	server_sockaddr.sin_family = AF_INET;


	// Initialize the server address by copying the results of gethostbyname. Note
	// many samples online use bcopy. This is not portable. memmove is more powerful
	// than memcopy so is used here (even though, by construction, the benefits of
	// memmove are not being used).
	memmove(&server_sockaddr.sin_addr.s_addr, server_hostent->h_addr, server_hostent->h_length);
	
	// To assign the port number, use host-to-network-short.
	server_sockaddr.sin_port = htons(server_port);

	Packet m;
	ssize_t bytes_sent;

	//Initialize the data for numOfPackets to the total number of packets to send
	//These will then be deleted as 'A' packets are received leaving only dropped sequence numbers
	unordered_set <int> numOfPackets;
	for (int x = 0; x < number_of_packets; x++)
	{
		numOfPackets.insert(x);
	}

	for (int i = 0; i < number_of_packets; i++)
	{
		// Fill in the sequece number in a network portable manner.
		//htonl breaks unsigned int
		m.command = 'D';
		//cout << "after command" << endl;
		m.sequence_number = htonl(i);
		// sendto() contains everything we need to know as UDP is entirely stateless. It returns
		// the number of bytes sent. It behooves you to ensure this is correct especially if
		// MSG_DONTWAIT is used (it is not used here).
		//cout << "before packet sent!" << endl;
		bytes_sent = sendto(udp_socket, (void *) &m, sizeof(m), 0, (struct sockaddr *) &server_sockaddr, sizeof(server_sockaddr));
		//cout << "Packet sent!" << endl;
		if (bytes_sent != sizeof(m))
		{
			cerr << "Number of bytes sent [" << bytes_sent << "] does not match message size." << endl;
			break;
		}
		//busy wait so that the server has time to process our packet and send its own back	
		//cout <<"before busy wait" << endl;
		if (uinterval > 0)
	  	{		busy_wait(uinterval);}
	//get acknowledgement packet from server
		if(getAcknowledgementPacket(m, server_sockaddr, udp_socket)){
	  //if acknowledgement has been recieved, good, if not then send another packet anyways
			//Deletes the received packets, leaving only the dropped ones at the end
			numOfPackets.erase(m.sequence_number);
	}
	else
        {
	  //else remove the sequence number from the list!
      
	}
	//then send another packet once recieved?
	}
	//Checks if an E command is to be sent
	//Needs to busywait/sleep before being sent to make sure the message is received
	
	printResults(numOfPackets, number_of_packets);
	if(userExit)
	{
		Packet e;
		e.sequence_number = 0;
		e.command = 'E';
		bytes_sent = sendto(udp_socket, (void *) &e, sizeof(e), 0, (struct sockaddr *) &server_sockaddr, sizeof(server_sockaddr));
	}

	return 0;
}

//returns true if packet was recieved
bool getAcknowledgementPacket(Packet& m, struct sockaddr_in& server_sockaddr, int& udp_socket){
  bool retval = false;
  ssize_t bytes_read;
  socklen_t server_length = sizeof(server_sockaddr);
  //packet will contain data
  if((bytes_read = recvfrom(udp_socket, &m, sizeof(m) + m.length, MSG_DONTWAIT, (struct sockaddr *) &server_sockaddr, &server_length)) >= 0)
  {
   //we will now take apart the packet that has been recieved and
   //parse out that meta-data from the server and check for an acknowledged packet
   switch(m.command)
   {
   	case 'A':
		//packet acknowledment
	  	retval = true;
		cout << "\nClient Recieved Acknowledgement!" << endl;
		break;
	default:
		break;
			 //no packet!
       }
  }
  return retval;
}
//--
void printResults(unordered_set <int>& droppedPackets, int totalPackets)
{
	cout << droppedPackets.size() << " out of " << totalPackets << " dropped" << endl;
	
	//TotalToPrint will iterate up to 100
	//This could probably be written more coherently, but not right now, I guess
	int totalToPrint = 0;
	cout << "First 100 Dropped Packet sequence numbers:" << endl;
	for(int x = 0; x < totalPackets; x++)
	{
		if(droppedPackets.find(x) == droppedPackets.end())
		{
			//do nothing
		}
		else if(totalToPrint >= 100)
		{
			break;
		}
		else
		{
			cout << *droppedPackets.find(x) << endl;
			totalToPrint++;
		}
	}
}