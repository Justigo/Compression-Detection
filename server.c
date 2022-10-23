#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include "cJSON.h"
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <sys/stat.h>

typedef struct
{
	int tcp_port;
}server_init;

typedef struct 
{  
     char address[256];  
     int source_port;  
     int destination_port; 
	 int tcp_port; 
     int payload;  
     int packets;   
}configurations; 

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

int get_port(const char *filename){
	FILE *fp;

	server_init port;

	size_t size = get_file_size(filename);
	if(size ==0){
		printf("no size\n");
	}

	char *buffer = malloc(size+1);
	if(buffer == NULL){
		printf("malloc not successful\n");
	}

	memset(buffer,0,size+1);

	fp = fopen(filename,"rb");

	fread(buffer,1,size,fp);
	fclose(fp);

	cJSON *json, *item;
	json = cJSON_Parse(buffer);
	item = cJSON_GetObjectItemCaseSensitive(json,"TCP_port");
	port.tcp_port = item->valueint;
	free(buffer);
	return port.tcp_port;
}



int main(int argc, char **argv){
	char server_message[256];
	int preprobe_port = get_port(argv[1]);

	//server socket created.
	int probe_socket;
	probe_socket = socket(AF_INET, SOCK_STREAM, 0);

	//define server address
	struct sockaddr_in probe_address;
	probe_address.sin_family = AF_INET;
	probe_address.sin_port = htons(preprobe_port);
	probe_address.sin_addr.s_addr = inet_addr("192.168.86.248");

	//bind the socket to the port and ip
	bind(probe_socket,(struct sockaddr*) &probe_address, sizeof(probe_address));

	listen(probe_socket, 5);

	int client_socket;
	client_socket = accept(probe_socket, NULL,NULL);

	//receive the message
	recv(client_socket, server_message, sizeof(server_message),0);
	printf("message: %s\n",server_message);

	//close the socket
	close(probe_socket);
	
	char * bytes;

	 int server_socket;
	 server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	// //define server address
	struct sockaddr_in server_address, client_address;

	memset(&server_address,0,sizeof(server_address));
	memset(&client_address,0, sizeof(client_address));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8765);
	server_address.sin_addr.s_addr=inet_addr("192.168.86.248");

	int val = 1;
	setsockopt(server_socket,SOL_SOCKET,IPV6_DONTFRAG, &val,sizeof(val));

	//bind the socket to the port and ip
	if(bind(server_socket,(struct sockaddr*) &server_address, sizeof(server_address))<0){
	 	printf("Binding failed");
	 };

	 int len, n;
	 len = sizeof(client_address);
	 for(int i = 0;i<6000;i++){
	 	if(recvfrom(server_socket,(char *)bytes,sizeof(bytes),MSG_WAITALL,(struct sockaddr*) &client_address,len)>0){
			printf("packet loss.");
		}else{
			printf("packet received.");
		}
	 }

	for(int i = 0;i<6000;i++){
	 	if(recvfrom(server_socket,(char *)bytes,sizeof(bytes),MSG_WAITALL,(struct sockaddr*) &client_address,len)>0){
			printf("packet loss.");
		}else{
			printf("packet received.");
		}
	}
	//close the socket
	close(server_socket);
	return 0;

}


