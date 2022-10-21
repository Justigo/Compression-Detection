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

	//char port_num[256];
	//struct sockaddr_in probing_address,client_probing_address;
	//memset(port_num, '\0',sizeof(port_num));

	//create the server socket for pre probing phase
	//int probing_socket,client_probe,client_size;
	//probing_socket = socket(AF_INET, SOCK_STREAM,0);

	//probing_address.sin_family = AF_INET;
	//probing_address.sin_family = htons(40329);
	//probing_address.sin_addr.s_addr = INADDR_ANY;
	
	//if(bind(probing_socket, (struct sockaddr*)&probing_address,sizeof(probing_address))<0){
		//printf("could not bind to port.");
	//}

	//if(listen(probing_socket,1) <0){
	//	printf("Error while listening");
	//	return -1;
	//}

	//client_size = sizeof(client_probing_address);
	//client_probe = accept(probing_socket,(struct sockaddr *)&client_probing_address, &client_size);

	//if(client_probe < 0){
		//printf("cannot accept address");
		//return -1;
	//}

	//if (recv(client_probe,port_num,sizeof(port_num),0) <0){
		//printf("couldn't receive");	
		//return -1;
	//}

	//printf("Port Number: %s\n",port_num);

	//close(probing_socket);
	//close(client_probe);

	//server socket created.
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


