#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include<string.h>

struct packet {
	int length;
	char bytes[998];
};

int main(){

	cJSON *str;
	// str = cJSON_GetObjectItemCaseSensitive();
	//TODO create UDP connection with the server to send config file stuff.
	int tcp_socket;

	// char port_num[256] = "8080";
	// tcp_socket = socket(AF_INET,SOC_STREAM,0);

	// struct sockaddr_in probe_address;
	// probe_address.sin_family = AF_INET;
	// probe_address.sin_port = htons(8080);
	// probe_address.sin_addr.s_addr = INADDR_ANY;

	// if(connect(tcp_socket,(struct sockaddr*)&probe_address,sizeof(probe_address))<0){
	// 	printf("cannot connect.");
	// }

	// if(send(tcp_socket))



	int packet_length = 1000;
	unsigned short id;
	//Create the socket
	//
	int network_socket;
	//Sock stream =TCP
	//zero equals default protocol
	network_socket = socket(AF_INET,SOCK_DGRAM, 0); 
	
	// give the address for the socket
	struct sockaddr_in server_address, client_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8765);
	server_address.sin_addr.s_addr = inet_addr("192.168.86.248");

	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(9876);
	client_address.sin_addr.s_addr = inet_addr("192.168.86.249");

	if(bind(network_socket,(struct sockaddr *)&client_address,sizeof(client_address))<0){
		perror("error in binding");
		return-1;
	}

	//initialize the packet train.
	struct packet *low_train = (struct packet*)malloc(6000 * sizeof(struct packet));
	struct packet *high_train = (struct packet*)malloc(6000 * sizeof(struct packet));


	//initialized the train to be low entropy
	for(int j = 0;j<6000;j++){
		low_train[j].length = packet_length;
		for (int k = 0;k<(packet_length-2);k++){
			low_train[j].bytes[k] = 0;
		}
		id = j;
		char conversion[50];
		
		sprintf(conversion,"%d",id);
		char* payload = (char *)malloc(strlen(low_train[j].bytes) + strlen(conversion)+1);
		
		strcpy(payload,conversion);
		
		strcat(payload,low_train[j].bytes);
		strcpy(low_train[j].bytes,payload);
		strcpy(low_train[j].bytes,conversion);
	}
	unsigned char myRandomData[1000];
	for(int i =0;i<6000;i++){
		unsigned int randomData = open("/dev/urandom", O_RDONLY);
		read(randomData,myRandomData,1000);
		close(randomData);
		high_train[i].length = packet_length;
		for(int d = 0;d<1000;d++){
			high_train[i].bytes[d] = myRandomData[d];
		}
	}

	//start to send the low and high entropy data
	for(int b = 0;b<6000;b++){
		if(sendto(network_socket,low_train[b].bytes,low_train[b].length,0,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
			perror("error");
		}
		
	}
	sleep(15);
	
	for(int i =0;i<6000;i++){
	if(sendto(network_socket,high_train[i].bytes,high_train[i].length,0,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
	perror("error");
	}else{
	printf("high packet sent!\n");
	}
	}

	printf("packets sent!\n");
	
	//Free the array of packets
	free(low_train);
	free(high_train);

	// int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

	//TODO: Create packets with high entropy data: randome sequence of bits.
	// recv(network_socket, &server_response,sizeof(server_response),0);

	//print out the server's response
	// printf("The server successfully sent the data: %s\n", server_response);

	//close the socket
	close(network_socket);
	return 0;
}
