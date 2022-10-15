#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
struct packet {
	int length;
	char bytes[1000];
}
int main(){
	//Create the socket
	//
	int network_socket;
	//Sock stream =TCP
	//zero equals default protocol
	network_socket = socket(AF_INET,SOCK_DGRAM, 0); 
	
	// give the address for the socket
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8765);
	server_address.sin_addr.s_addr = INADDR_ANY;

	for(int i =0;i<600;i++){
		sendto()
	}

	// int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

	//Check for error in the connection
	if(connection_status == -1){
		printf("Bro your connection sucks");
	}
	// receive data from the server
	char server_response[256];
	//TODO: Create a packet with low entropy data: all 0s
	//TODO: Create packets with high entropy data: randome sequence of bits.
	// recv(network_socket, &server_response,sizeof(server_response),0);

	//print out the server's response
	printf("The server successfully sent the data: %s\n", server_response);

	//close the socket
	close(network_socket);
	return 0;
}
