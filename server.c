#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

int main(){
	char bytes;
	int opt = 1;
	char server_message[256] = "balls";

	int probing_socket;
	probing_socket = socket(AF_INET, SOCK_STREAM,0);

	if(setsockopt(probing_socket, SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &opt,sizeof(opt))){
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	probing_socket.sin_family = AF_INET;
	probing_socket.sin_family = htons(8080);
	probing_socket.sin_addr.s_addr = INADDR_ANY;
	//server socket created.
	int server_socket;
	server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	//define server address
	struct sockaddr_in server_address, client_address;

	memset(&server_address,0,sizeof(server_address));
	memset(&client_address,0, sizeof(client_address));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8765);
	server_address.sin_addr.s_addr=INADDR_ANY;

	//bind the socket to the port and ip
	if(bind(server_socket,(struct sockaddr*) &server_address, sizeof(server_address))<0){
		printf("Binding failed");
	};

	int len, n;
	len = sizeof(client_address);
	recvfrom(server_socket,bytes,sizeof(bytes),MSG_WAITALL,(struct sockaddr*) &client_address,len);
	printf("server received!:");
	//close the socket
	close(server_socket);
	return 0;

}


