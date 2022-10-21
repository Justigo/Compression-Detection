#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

int main(){
	char server_message[256] = "balls";

	//server socket created.
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	//define server address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	server_address.sin_addr.s_addr = inet_addr("192.168.86.248");

	//bind the socket to the port and ip
	bind(server_socket,(struct sockaddr*) &server_address, sizeof(server_address));

	listen(server_socket, 5);

	int client_socket;
	client_socket = accept(server_socket, NULL,NULL);

	//send the message
	send(client_socket, server_message, sizeof(server_message),0);

	//close the socket
	close(server_socket);
	
	char bytes;

	 int server_socket;
	 server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	// //define server address
	struct sockaddr_in server_address, client_address;

	memset(&server_address,0,sizeof(server_address));
	memset(&client_address,0, sizeof(client_address));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8765);
	server_address.sin_addr.s_addr=inet_addr("192.168.86.248");

	//bind the socket to the port and ip
	if(bind(server_socket,(struct sockaddr*) &server_address, sizeof(server_address))<0){
	 	printf("Binding failed");
	 };

	 int len, n;
	 len = sizeof(client_address);
	 for(int i = 0;i<6000;i++){
	 	recvfrom(server_socket,bytes,sizeof(bytes),MSG_WAITALL,(struct sockaddr*) &client_address,len);
	 }
	 printf("server received!:");

	 	 for(int i = 0;i<6000;i++){
	 	recvfrom(server_socket,bytes,sizeof(bytes),MSG_WAITALL,(struct sockaddr*) &client_address,len);
	 }
	//close the socket
	close(server_socket);
	return 0;

}


