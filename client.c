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
     char source_port[256];  
     char destination_port[256]; 
	 char tcp_port[256]; 
     char payload[256];  
     char packets[256];  
	 char intermit_time[256]; 
	 char server_ip[256];
	 char client_ip[256];
}configurations;  

configurations cJSON_to_struct(char* text, configurations settings){
    cJSON *json,*item;
    int i =0;
    json = cJSON_Parse(text);

    item = cJSON_GetObjectItemCaseSensitive(json,"source_port");
	if(item == NULL){
		printf("Missing source port.\n");
		exit(1);
	}
	strcpy(settings.source_port,item->valuestring);

	item = cJSON_GetObjectItemCaseSensitive(json,"TCP_port");
	if(item == NULL){
		printf("Missing TCP port.\n");
		exit(1);
	}
	strcpy(settings.tcp_port,item->valuestring);

    item= cJSON_GetObjectItemCaseSensitive(json,"destination_port");
	if(item == NULL){
		printf("Missing UDP port.\n");
		exit(1);
	}
	strcpy(settings.destination_port,item->valuestring);

    item = cJSON_GetObjectItemCaseSensitive(json,"size_of_payload");
	if(item == NULL){
		strcpy(settings.payload,"1000");
	}else{
		strcpy(settings.payload,item->valuestring);
	}

    item = cJSON_GetObjectItemCaseSensitive(json,"number_of_udp_packets");
	if(item == NULL){
		strcpy(settings.packets,"6000");
	}else{
		strcpy(settings.packets,item->valuestring);

	}
	item = cJSON_GetObjectItemCaseSensitive(json,"intermit_time");
	if(item == NULL){
		strcpy(settings.intermit_time,"15");
	}else{
		strcpy(settings.intermit_time,item->valuestring);
	}

	item = cJSON_GetObjectItemCaseSensitive(json,"client_ip");
	if(item == NULL){
		printf("missing client IP");
		exit(1);
	}
	strcpy(settings.client_ip,item->valuestring);

	item = cJSON_GetObjectItemCaseSensitive(json,"server_ip");
	if(item == NULL){
		printf("missing server IP");
		exit(1);
	}
	strcpy(settings.server_ip,item->valuestring);

    cJSON_Delete(json);
    return settings;

}

//helper function to set the socket address
void set_address(int socket, int port, struct sockaddr_in *address,char * ip_address){
	address->sin_family = AF_INET;
	address->sin_port = htons(port);
	address->sin_addr.s_addr =inet_addr(ip_address);
}

char ** populate_array(char **array,char *payload,int length,int size){
	array = (char**)malloc(size*sizeof(char*));

	for(unsigned short int j=0;j<size;j++){
		array[j] = (char*)malloc(length*sizeof(char));
		memcpy(array[j],payload,length);
		array[j][0] = (uint8_t)(j & 0xff);
		array[j][1] = (uint8_t)(j >> 8);
	} 

	return array;
}

void free_array(char**array,int size){
	for(int i=0;i<size;i++){
		free(array[i]);
	}
	free(array);
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
	if(argc !=2){
		printf("Missing config file");
		exit(1);
	}
	
	//read and parse the json struct
	configurations settings = read_file(argv[1]);
	int packet_length = atoi(settings.payload);
	int train_size = atoi(settings.packets);
	int source_port = atoi(settings.source_port);
	int destination_port = atoi(settings.destination_port);
	int tcp_port = atoi(settings.tcp_port);
	int intermit_time = atoi(settings.intermit_time);

	//preprobing phase for sending the information to the config file
	printf("Starting pre probing phase...\n");
	int probe_socket;
	probe_socket = socket(AF_INET,SOCK_STREAM, 0); 
	
	// // give the address for the socket
	struct sockaddr_in probe_address;
	char * server_ip = settings.server_ip;
	char * client_ip = settings.client_ip;
	memset(&probe_address,0,sizeof(probe_address));
	set_address(probe_socket,tcp_port,&probe_address,server_ip);

	int connection_status = connect(probe_socket, (struct sockaddr *) &probe_address, sizeof(probe_address));

	// //Check for error in the connection
	if(connection_status == -1){
		printf("Cannot Connect to server.\n");
	}

	send(probe_socket, (char *)settings.server_ip,sizeof(settings.server_ip),0);
	send(probe_socket, (char *)settings.packets,sizeof(settings.packets),0);
	send(probe_socket,(char *)settings.destination_port,sizeof(settings.destination_port),0);

	// //close the socket
	printf("Ending probing phase...\n");
	close(probe_socket);

	printf("Sending packets...\n");
	//Create the socket
	
	int network_socket;
	//Sock stream =TCP
	//zero equals default protocol
	network_socket = socket(AF_INET,SOCK_DGRAM, 0); 
	
	// give the address for the udp socket for both server and client.
	struct sockaddr_in server_address, client_address;
	memset(&server_address,0,sizeof(server_address));
	memset(&client_address,0,sizeof(client_address));
	set_address(network_socket,destination_port,&server_address,server_ip);
	set_address(network_socket,source_port,&client_address,client_ip);

	if(bind(network_socket,(struct sockaddr *)&client_address,sizeof(client_address))<0){
		perror("error in binding");
		return-1;
	}

	//initialize the packet train for both low and high entropy data.

	char ** packet_list = (char**)malloc(train_size*sizeof(char*));

	char * low_entropy = (char *)malloc(packet_length*sizeof(char));
	memset(low_entropy,0,packet_length);

	char ** low_train2 = populate_array(packet_list,low_entropy,packet_length,train_size);

	//initialize the payload for high entropy data
	char * myRandomData = (char *)malloc(packet_length * sizeof(char));
	//Note: May create function right here
	unsigned int randomData = open("random", O_RDONLY);
	read(randomData,myRandomData,packet_length);
	close(randomData);

	char** high_train2 = populate_array(packet_list,myRandomData,packet_length,train_size);

	//start to send the low and high entropy data
	for(int b = 0;b<train_size;b++){
		if(sendto(network_socket,low_train2[b],packet_length,0,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
			perror("error");
		}
		usleep(100);
		
	}
	sleep(intermit_time);
	
	for(int i =0;i<train_size;i++){
		if(sendto(network_socket,high_train2[i],packet_length,0,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
			perror("error");
		}
		usleep(100);
	}

	printf("packets sent!\n");
	
	//Free the array of packets and the payload
	free(low_entropy);
	free(myRandomData);
	free_array(low_train2,train_size);
	free_array(high_train2,train_size);
	close(network_socket);

	//creation of the postprobe tcp socket
	char message[256];
	int postProbe_socket;
	postProbe_socket = socket(AF_INET,SOCK_STREAM,0);

	int post_probe_connection = connect(postProbe_socket, (struct sockaddr *) &probe_address, sizeof(probe_address));
	if(post_probe_connection == -1){
		perror("error in connecting to server");
	}
	recv(postProbe_socket,&message,sizeof(message),0);

	printf("report %s\n",message);
	close(postProbe_socket);

	return 0;
}