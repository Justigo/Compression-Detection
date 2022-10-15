#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>

struct packet {
	int length;
	char bytes[256];
}
int main(){
	
	char server_message[256] = "balls";

	//server socket created.
	int server_socket;
	server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	//define server address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	server_address.sin_addr.s_addr=INADDR_ANY;

	//bind the socket to the port and ip
	bind(server_socket,(struct sockaddr*) &server_address, sizeof(server_address));

	listen(server_socket, 5);

	int client_socket;
	client_socket = accept(server_socket, NULL,NULL);

	//send the message
	send(client_socket, server_message, sizeof(server_message),0);

	//close the socket
	close(server_socket);
	return 0;
}


