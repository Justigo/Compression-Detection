#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include "cJSON.h"
#include <sys/ioctl.h>
#include <net/if.h>
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
int * allocate_intmem(int len){
	void *tmp;

	if(len <=0){
		fprintf (stderr,"ERROR: Cannot allocate memory because len = %i in allocate_intmem().\n",len);
		exit(EXIT_FAILURE);
	}

	tmp = (int *) malloc (len * sizeof(int));
	if(tmp != NULL){
		memset (tmp,0,len*sizeof(int));
		return(tmp);
	}else{
		fprintf(stderr,"ERROR: Cannot allocate memory for array allocate_intmem().\n",len);
		exit(EXIT_FAILURE);
	}

	
}

uint8_t * allocate_ustrmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (uint8_t));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
    exit (EXIT_FAILURE);
  }
}
uint16_t checksum (uint16_t *addr, int len)
{
  int count = len;
  register uint32_t sum = 0;
  uint16_t answer = 0;

  // Sum up 2-byte values until none or only one byte left.
  while (count > 1) {
    sum += *(addr++);
    count -= 2;
  }

  // Add left-over byte, if any.
  if (count > 0) {
    sum += *(uint8_t *) addr;
  }

  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  // Checksum is one's compliment of sum.
  answer = ~sum;

  return (answer);
}

int main(int argc, char **argv){
	int i, status, datalen, frame_length, sd, bytes, *ip_flags;
	char *interface, *target;
	uint8_t *data, *src_mac, *dst_mac, *ether_frame;
  	struct addrinfo hints, *res;
  	struct sockaddr_in *ipv4;
	void *tmp;
	struct ip iphdr;
	struct ifreq ifr;
	struct tcphdr tcp_hdr;
	struct sockaddr_ll device;

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
    char * server_ip = settings.server_ip;
	char * client_ip = "192.168.86.248";

	//Allocate memory for various arrays
	src_mac = allocate_ustrmem(6);
	dst_mac = allocate_ustrmem(6);
	data = allocate_ustrmem(IP_MAXPACKET);
	ether_frame = allocate_strmem(INET_ADDRSTRLEN);
	interface = allocate_strmem(40);
	target = allocate_strmem(40);
	ip_flags = allocate_intmem(4);

	strcpy(interface,"enp0s3");

    int network_socket;
	//Sock stream =TCP
	//zero equals default protocol
	network_socket = socket(AF_INET,SOCK_DGRAM, 0);
	if(socket == -1){
		perror("error in creating socket");
	} 
	
	// give the address for the udp socket for both server and client.
    struct sockaddr_in server_address, client_address;
	memset(&server_address,0,sizeof(server_address));
	memset(&client_address,0,sizeof(client_address));
	set_address(network_socket,destination_port,&server_address,server_ip);
	set_address(network_socket,source_port,&client_address,client_ip);

	int frag = IP_PMTUDISC_DO;
    setsockopt(network_socket,IPPROTO_IP,IP_MTU_DISCOVER,&frag,sizeof(frag));
    setsockopt(network_socket,IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

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
	unsigned int randomData = open("random",O_RDONLY);
	read(randomData,myRandomData,packet_length);
	close(randomData);

	char** high_train2 = populate_array(packet_list,myRandomData,packet_length,train_size);

	//start to send the low and high entropy data
	for(int b = 0;b<train_size;b++){
		if(sendto(network_socket,low_train2[b],packet_length,MSG_CONFIRM,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
			perror("error");
		}
		
	}
	
	for(int i =0;i<train_size;i++){
		if(sendto(network_socket,high_train2[i],packet_length,MSG_CONFIRM,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
			perror("error");
		}
	}

	printf("packets sent!\n");
	
	//Free the array of packets and the payload
	free(low_entropy);
	free(myRandomData);
	free_array(low_train2,train_size);
	free_array(high_train2,train_size);
	close(network_socket);
    // close(raw_socket);


    return 0;

}
