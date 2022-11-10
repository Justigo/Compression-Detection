#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "cJSON.h"
#include<sys/stat.h>

typedef struct 
{  
     char address[256];  
     char source_port[256];  
     char destination_port[256]; 
     char tcp_port[256]; 
     char payload[256];  
     char packets[256];  
     char intermit_time[256]; 
     char server_ip[256];
     char ttl[256];
     char head_port[256];
     char tail_port[256];
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

	item = cJSON_GetObjectItemCaseSensitive(json,"intermit_time");
	strcpy(settings.intermit_time,item->valuestring);

	item = cJSON_GetObjectItemCaseSensitive(json,"server_ip");
	strcpy(settings.server_ip,item->valuestring);

    item = cJSON_GetObjectItemCaseSensitive(json,"ttl");
    strcpy(settings.ttl,item->valuestring);

    item = cJSON_GetObjectItemCaseSensitive(json,"head_port");
    strcpy(settings.head_port,item->valuestring);

    item = cJSON_GetObjectItemCaseSensitive(json,"tail_port");
    strcpy(settings.tail_port,item->valuestring);

    cJSON_Delete(json);
    return settings;

}
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
		array[j][0] = j%256;
		array[j][1] = j/256;
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

unsigned short csum(unsigned short *buf, int nwords)
{
  unsigned long sum;
  for(sum=0; nwords>0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum &0xffff);
  sum += (sum >> 16);
  return (unsigned short)(~sum);
}

int main(int argc, char **argv){
    configurations settings = read_file(argv[1]);
	int packet_length = atoi(settings.payload);
	int train_size = atoi(settings.packets);
	int source_port = atoi(settings.source_port);
	int destination_port = atoi(settings.destination_port);
	int tcp_port = atoi(settings.tcp_port);
	int intermit_time = atoi(settings.intermit_time);
	int ttl = atoi(settings.ttl);
    int head_port = atoi(settings.head_port);
    int tail_port = atoi(settings.tail_port);

    int raw_socket = socket(AF_INET,SOCK_RAW,IPPROTO_TCP);
    if(raw_socket == -1){
        perror("error in creating socket");
    }

    //Datagram to represent the packet
	char datagram[4096] , source_ip[32] , *data , *pseudogram;
	
	//zero out the packet buffer
	memset (datagram, 0, 4096);

    struct iphdr *iph = (struct iphdr *) datagram;


    return 0;

}
