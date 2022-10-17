#include <stdio.h>
#include <stdlib.h>

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

int main(){
	int packet_length = 1000;
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

	int val = IP_PMTUDISC_DONT;
	setsockopt(network_socket,IPPROTO_IP, IP_MTU_DISCOVER,&val,sizeof(val));

	//initialize the packet train.
	struct packet low_train[6000];

	for(int j = 0;j<6000;j++){
		for (int k = 0;k<packet_length;k++){
			low_train[j].bytes[k] = 0;
		}
		low_train[j].length = packet_length;
	}

	for(int i =0;i<6000;i++){
		if(sendto(network_socket,low_train[i].bytes,low_train[i].length,0,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
			perror("error");
		}
	}
	printf("packets sent!\n");
	int randomData = open("/dev/urandom", O_RDONLY);
	if (randomData < 0)
	{
		printf("FUCK\n");
	}
	else
	{
    	char myRandomData[50];
    	ssize_t result = read(randomData, myRandomData, sizeof myRandomData);
   	 if (result < 0)
    	{
			printf("shit");
    	}

		printf("%c\n",myRandomData);
}

	// int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

	//Check for error in the connection
	// receive data from the server
	// char server_response[256];
	//TODO: Create a packet with low entropy data: all 0s
	//TODO: Create packets with high entropy data: randome sequence of bits.
	// recv(network_socket, &server_response,sizeof(server_response),0);

	//print out the server's response
	// printf("The server successfully sent the data: %s\n", server_response);

	//close the socket
	close(network_socket);
	return 0;
}
