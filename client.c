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
#include<sys/stat.h>

typedef struct 
{  
     char address[256];  
     char source_port[256];  
     char destination_port[256]; 
	 char tcp_port[256]; 
     char payload[256];  
     char packets[256];   
}configurations;  

configurations cJSON_to_struct(char* text, configurations settings){
    cJSON *json,*item;
    int i =0;
    json = cJSON_Parse(text);

    item = cJSON_GetObjectItemCaseSensitive(json,"server_address");
	strcpy(settings.address,item->valuestring);

    item = cJSON_GetObjectItemCaseSensitive(json,"source_port");
	strcpy(settings.source_port,item->valuestring);

	item = cJSON_GetObjectItemCaseSensitive(json,"TCP_port");
	strcpy(settings.tcp_port,item->valuestring);

    item= cJSON_GetObjectItemCaseSensitive(json,"destination_port");
	strcpy(settings.destination_port,item->valuestring);

    item = cJSON_GetObjectItemCaseSensitive(json,"size_of_payload");
	strcpy(settings.payload,item->valuestring);

    item = cJSON_GetObjectItemCaseSensitive(json,"number_of_udp_packets");
	strcpy(settings.packets,item->valuestring);

    cJSON_Delete(json);
    return settings;

}



size_t get_file_size(const char *filepath){
    if (filepath == NULL){
        return 0;
    }
    struct stat filestat;
    memset(&filestat,0,sizeof(struct stat));

    if(stat(filepath,&filestat)==0){
        return filestat.st_size;
    }else{
        return 0;
    }
}

configurations read_file(const char *filename)
{
    FILE *fp;

    configurations settings;

    size_t size = get_file_size(filename);
    if(size ==0){
        printf("size failed\n");
    }

    char *buffer = malloc(size+1);
    if(buffer == NULL){
        printf("malloc not successful\n");
    }
    memset(buffer,0,size+1);

    fp = fopen(filename,"rb");
    
    fread(buffer,1,size,fp);
    fclose(fp);

    settings = cJSON_to_struct(buffer,settings);
    free(buffer);
    return settings;
}



int main(int argc, char **argv){
	


	configurations settings = read_file(argv[1]);
	int packet_length = atoi(settings.payload);
	int train_size = atoi(settings.packets);
	int source_port = atoi(settings.source_port);
	int destination_port = atoi(settings.destination_port);
	int tcp_port = atoi(settings.tcp_port);


	struct packet {
		int length;
		char bytes[packet_length-2];
	};

	// int probe_socket;
	// //Sock stream =TCP
	// //zero equals default protocol
	int probe_socket;
	probe_socket = socket(AF_INET,SOCK_STREAM, 0); 
	
	// // give the address for the socket
	struct sockaddr_in probe_address;
	probe_address.sin_family = AF_INET;
	probe_address.sin_port = htons(tcp_port);
	probe_address.sin_addr.s_addr =inet_addr("192.168.86.249");

	int connection_status = connect(probe_socket, (struct sockaddr *) &probe_address, sizeof(probe_address));

	// //Check for error in the connection
	if(connection_status == -1){
		printf("Bro your connection sucks");
	}
	// // receive data from the server
	char server_response[256]="balls";
	send(probe_socket, (char *)settings.address,sizeof(settings.address),0);
	send(probe_socket, (char *)settings.packets,sizeof(settings.packets),0);
	send(probe_socket,(char *)settings.destination_port,sizeof(settings.packets),0);

	// //print out the server's response
	printf("The server successfully sent the data: %s\n", server_response);

	// //close the socket
	close(probe_socket);

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
	server_address.sin_port = htons(destination_port);
	server_address.sin_addr.s_addr = inet_addr("192.168.86.249");

	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(source_port);
	client_address.sin_addr.s_addr = inet_addr("192.168.86.248");

	if(bind(network_socket,(struct sockaddr *)&client_address,sizeof(client_address))<0){
		perror("error in binding");
		return-1;
	}

	//initialize the packet train.
	struct packet *low_train = (struct packet*)malloc(train_size * sizeof(struct packet));
	struct packet *high_train = (struct packet*)malloc(train_size* sizeof(struct packet));


	//initialized the train to be low entropy
	for(int j = 0;j<train_size;j++){
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

	unsigned char myRandomData[packet_length];
	//Note: May create function right here
	unsigned int randomData = open("rand", O_RDONLY);
	read(randomData,myRandomData,packet_length);
	close(randomData);

	for(int i =0;i<train_size;i++){
		high_train[i].length = packet_length;
		for(int d = 0;d<(packet_length-2);d++){
			high_train[i].bytes[d] = myRandomData[d];
		}
		id = i;
		char conversion[50];
		sprintf(conversion,"%d",id);
		char* payload = (char *)malloc(strlen(high_train[i].bytes) + strlen(conversion)+1);

		strcpy(payload,conversion);
		
		strcat(payload,high_train[i].bytes);
		strcpy(high_train[i].bytes,payload);
		strcpy(high_train[i].bytes,conversion);
	}

	//start to send the low and high entropy data
	for(int b = 0;b<train_size;b++){
		if(sendto(network_socket,low_train[b].bytes,low_train[b].length,0,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
			perror("error");
		}
		
	}
	sleep(15);
	
	for(int i =0;i<train_size;i++){
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
