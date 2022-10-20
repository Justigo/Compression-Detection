#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>

struct packet {
	int length;
	char bytes[1000];
};



int main(int argc, char *argv[]){
	// //TODO create UDP connection with the server to send config file stuff.
	int tcp_socket;

	char port_num[256] = "8080";
	struct sockaddr_in probe_address;
	tcp_socket = socket(AF_INET,SOCK_STREAM,0);

	probe_address.sin_family = AF_INET;
	probe_address.sin_port = htons(8080);
	probe_address.sin_addr.s_addr = INADDR_ANY;


	if(connect(tcp_socket,(struct sockaddr*)&probe_address,sizeof(probe_address))<0){
		printf("cannot connect.");
		return -1;
	}

	if(send(tcp_socket,port_num,strlen(port_num),0)<0){
		printf("cant send message\n");
		return -1;
	}
	close(tcp_socket);



	// int packet_length = 1000;
	// //Create the socket
	// //
	// int network_socket;
	// //Sock stream =TCP
	// //zero equals default protocol
	// network_socket = socket(AF_INET,SOCK_DGRAM, 0); 
	
	// // give the address for the socket
	// struct sockaddr_in server_address;
	// server_address.sin_family = AF_INET;
	// server_address.sin_port = htons(8765);
	// server_address.sin_addr.s_addr = INADDR_ANY;

	// int val = IP_PMTUDISC_DONT;
	// setsockopt(network_socket,IPPROTO_IP, IP_MTU_DISCOVER,&val,sizeof(val));

	// //initialize the packet train.
	// struct packet *low_train = (struct packet*)malloc(6000 * sizeof(struct packet));
	// struct packet *high_train = (struct packet*)malloc(6000 * sizeof(struct packet));

	// int randomData = open("/dev/urandom", O_RDONLY);
	// unsigned char myRandomData[6000];


	// //initialized the train to be low entropy
	// for(int j = 0;j<6000;j++){
	// 	high_train[j].length = packet_length;
	// 	low_train[j].length = packet_length;
	// 	for (int k = 0;k<packet_length;k++){
	// 		high_train[j].bytes[k] = myRandomData[k];
	// 		low_train[j].length = packet_length;
	// 	}
		
	// }

	// //start to send the low and high entropy data
	// for(int b = 0;b<6000;b++){
	// 	if(sendto(network_socket,low_train[b].bytes,low_train[b].length,0,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
	// 		perror("error");
	// 	}else{
	// 		printf("low packet sent!\n");
	// 	}
	// }
	// for(int i =0;i<6000;i++){
	// 	if(sendto(network_socket,high_train[i].bytes,high_train[i].length,0,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
	// 		perror("error");
	// 	}else{
	// 		printf("high packet sent!\n");
	// 	}
	// }

	// printf("packets sent!\n");
	// //Free the array of packets
	// free(low_train);
	// free(high_train);

	// // int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

	// //TODO: Create packets with high entropy data: randome sequence of bits.
	// // recv(network_socket, &server_response,sizeof(server_response),0);

	// //print out the server's response
	// // printf("The server successfully sent the data: %s\n", server_response);

	// //close the socket
	// close(network_socket);
	return 0;
}
