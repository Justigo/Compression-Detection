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
#include <net/ethernet.h>
#include<sys/stat.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#define IP4_HDRLEN 20
#define TCP_HDRLEN 20

typedef struct 
{   
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
	 char standalone_client_ip[256];
}configurations;

//get the information from a config file and parse it to a struct
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

	item = cJSON_GetObjectItemCaseSensitive(json,"standalone_client_ip");
	if(item == NULL){
		printf("Missing standalone client port.\n");
		exit(1);
	}
	strcpy(settings.standalone_client_ip,item->valuestring);


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

	item = cJSON_GetObjectItemCaseSensitive(json,"server_ip");
	if(item == NULL){
		printf("missing server IP");
		exit(1);
	}
	strcpy(settings.server_ip,item->valuestring);

    item = cJSON_GetObjectItemCaseSensitive(json,"ttl");
	if(item == NULL){
		strcpy(settings.ttl,"255");
		exit(1);
	}else{
		strcpy(settings.ttl,item->valuestring);
	}

    item = cJSON_GetObjectItemCaseSensitive(json,"head_port");
	if(item == NULL){
		printf("no head port");
		exit(1);
	}
    strcpy(settings.head_port,item->valuestring);

    item = cJSON_GetObjectItemCaseSensitive(json,"tail_port");
	if(item == NULL){
		printf("no tail port");
		exit(1);
	}
    strcpy(settings.tail_port,item->valuestring);

    cJSON_Delete(json);
    return settings;

}

struct pseudo_header{
	u_int32_t source_address;
	u_int32_t destination_address;
	u_int8_t protocol;
	u_int8_t placeholder;
	u_int16_t tcp_length;
}; 


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
		fprintf(stderr,"ERROR: Cannot allocate memory for array allocate_intmem().\n");
		exit(EXIT_FAILURE);
	}

	
}

// Allocate memory for an array of chars.
char * allocate_strmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_strmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (char *) malloc (len * sizeof (char));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (char));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_strmem().\n");
    exit (EXIT_FAILURE);
  }
}

uint8_t * allocate_ustrmem (int len){
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

void process_syn(struct ip iphdr,struct sockaddr_in* ipv4, char* data,struct tcphdr tcp_hdr, char* pseudogram,char* datagram, int seq_num,int dest_port,int pseudo_size, int raw_tcp,int packet_length){
	//recalculate the ipheader checksum
	iphdr.ip_sum = 0;
	iphdr.ip_sum = checksum((unsigned short *) datagram,iphdr.ip_len);

	//update the tcp sequence number and destination port
	tcp_hdr.th_seq = htonl(seq_num);
	tcp_hdr.th_dport = htons(dest_port);
	tcp_hdr.th_sum = 0;

	memcpy(pseudogram+sizeof(struct pseudo_header), &tcp_hdr,sizeof(struct tcphdr));
	memcpy(pseudogram + sizeof(struct pseudo_header) + sizeof(struct tcphdr),data,strlen(data));

	tcp_hdr.th_sum = checksum((unsigned short*)pseudogram, pseudo_size);

	memcpy(datagram + sizeof(struct ip), &tcp_hdr,sizeof(struct tcphdr));

	if(sendto(raw_tcp,datagram,packet_length,0,(struct sockaddr *)ipv4,sizeof(struct sockaddr_in)) < 0){
	 	perror("sendto failed");
	 }
}

int main(int argc, char **argv){
	if(argc !=2){
		printf("Missing config file");
		exit(1);
	}
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
	char * client_ip = settings.standalone_client_ip;

	//Allocate memory for various arrays
	src_mac = allocate_ustrmem(6);
	dst_mac = allocate_ustrmem(6);
	data = allocate_ustrmem(IP_MAXPACKET);
	ether_frame = allocate_strmem(INET_ADDRSTRLEN);
	interface = allocate_strmem(40);
	target = allocate_strmem(40);
	ip_flags = allocate_intmem(4);

	strcpy(interface,"enp0s3");


	if((sd = socket(AF_INET, SOCK_RAW,IPPROTO_RAW)) < 0){
		perror("socket() failed to get socket descriptor for using ioctl() ");
		return (EXIT_FAILURE);
	}

	memset(&ifr,0,sizeof (ifr));
	snprintf(ifr.ifr_name, sizeof (ifr.ifr_name),"%s",interface);
	if(ioctl(sd, SIOCGIFHWADDR, &ifr)<0){
		perror("ioctl() failed to get source MAC address");
		return (EXIT_FAILURE);
	}
	close(sd);

	memcpy(src_mac,ifr.ifr_hwaddr.sa_data, 6);
	printf("MAC address for interface %s is \n", interface);
	for(int i=0;i<5;i++){
		printf("%02x\n", src_mac[i]);
	}
	printf("%02x\n",src_mac[5]);
	memset(&device, 0, sizeof(device));
	if ((device.sll_ifindex = if_nametoindex(interface)==0)){
		perror("if_nametoindex() failed to obtain interface index");
		exit (EXIT_FAILURE);
	}

	//set destination MAC address
	dst_mac[0] = 0xe4;
	dst_mac[1] = 0xc3;
	dst_mac[2] = 0x2a;
	dst_mac[3] = 0x07;
	dst_mac[4] = 0x18;
	dst_mac[5] = 0xca;

	//fill out hints for getaddrinfo().
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = hints.ai_flags | AI_CANONNAME;

	//Resolve target using getaddrinfo()
	if((status = getaddrinfo(server_ip,NULL, &hints,&res))!=0){
		fprintf(stderr,"getaddrinfo() failed: %s\n", gai_strerror(status));
		exit(EXIT_FAILURE);
	}
	ipv4 = (struct sockaddr_in *)res->ai_addr;
	tmp = &(ipv4->sin_addr);
	if(inet_ntop (AF_INET, tmp, server_ip, INET_ADDRSTRLEN) == NULL){
		status = errno;
		fprintf(stderr,"inet_ntop() failed.\nError message: %s", strerror(status));
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(res);


	//Fill out sockaddr_ll
	device.sll_family = AF_PACKET;
	device.sll_protocol = htons(ETH_P_IP);
	memcpy (device.sll_addr, dst_mac, 6);
	device.sll_halen=6;

	//TCP data 
	data = "Head";

	//IPv4 header
	iphdr.ip_hl = IP4_HDRLEN/sizeof(uint32_t);

	//Internet Protocol version (4 bits): IPv4
	iphdr.ip_v = 4;

	//Type of service (8 bits)
	iphdr.ip_tos = 0;

	//ID sequence number(16 bits): unused, since single datagram
	iphdr.ip_id = htons(0);

	//Flags, and fragmentation offset (3, 13 bits):

	//Zero (1 bit)
	ip_flags[0] = 0;

	//Do not fragment flag (1 bit)
	ip_flags[1] = 0;

	//More fragments following flag (1 bit)
	ip_flags[2] = 0;

	//Fragmentation offset (13 bits)
	ip_flags[3] = 0;

	iphdr.ip_off = htons((ip_flags[0] << 15)
						+(ip_flags[1] << 14)
						+(ip_flags[2] << 13)
						+ ip_flags[3]);

	//Time-to-live (8 bits): default to maximum value
	iphdr.ip_ttl = ttl;

	// Transport layer protocol (8 bits): 
	iphdr.ip_p = IPPROTO_TCP;

	//Source IPv4 address (32 bits)
	if((status = inet_pton(AF_INET, client_ip,&(iphdr.ip_dst)))!=1){
		fprintf(stderr, "inet_pton() failed.\nError message: %s",strerror(status));
		exit(EXIT_FAILURE);
	}

	//Destination IPv4 address (32 bits)
	if((status = inet_pton(AF_INET, server_ip, &(iphdr.ip_dst)))!=1){
		fprintf(stderr,"inet_pton() falied.\nError message: %s",strerror(status));
		exit(EXIT_FAILURE);
	}

	//set the datagram for tcp packet
	char datagram[4096];
	memset(datagram,0,4096);
	memcpy(datagram, &iphdr, sizeof(struct iphdr));
	memcpy(datagram + sizeof(struct iphdr) + sizeof(struct tcphdr), data, strlen(data));
	
	iphdr.ip_sum = 0;
	iphdr.ip_sum = checksum((uint16_t *)&iphdr, IP4_HDRLEN);

	memset(&tcp_hdr, 0, sizeof(tcp_hdr));

	//fill out the tcp headers
	tcp_hdr.th_sport = htons(source_port);
	tcp_hdr.th_dport = htons(head_port);
	tcp_hdr.th_seq = htonl(0);
	tcp_hdr.th_ack = htonl(0);
	tcp_hdr.th_off = 5;

	//set the flag for SYN packet
	int* tcp_flags = allocate_intmem(6);
	tcp_hdr.th_flags = 0;//set inital tchdr flags
	tcp_hdr.th_flags +=TH_SYN;

	//set the window size for the TCP packet
	tcp_hdr.th_win = htons(65535);

	tcp_hdr.th_sum = 0;
	tcp_hdr.th_urp = htons(0);

	//copy tcp header int datagram
	memcpy(datagram + sizeof(struct iphdr) , &tcp_hdr, sizeof(struct tcphdr));

	char * pseudogram;
	struct pseudo_header psh;

	//populate pseudo ip header

	psh.source_address = inet_addr(client_ip);
	psh.destination_address = ipv4->sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(sizeof(struct tcphdr) + strlen(data));
	
	int pseudo_size = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + strlen(data);
	pseudogram = malloc(pseudo_size);

	//copy data to pseudogram
	memcpy(pseudogram, (char*) &psh,sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header) , &tcp_hdr, sizeof(struct tcphdr));
	memcpy(pseudogram + sizeof(struct pseudo_header) + sizeof(struct tcphdr), data,strlen(data));

	//calculate tcp checksum
	tcp_hdr.th_sum = checksum((unsigned short*) pseudogram, pseudo_size);

	//frame length: IP header + TCP header + data
	int tcp_packet_length = IP4_HDRLEN + TCP_HDRLEN + datalen;

	int raw_tcp;

	if((raw_tcp = socket(PF_INET, SOCK_RAW, IPPROTO_TCP)) < 0){
		perror("Failed to create raw socket");
		exit(1);
	}

	//set socket to include ip header

	int one = 1;
	if(setsockopt(raw_tcp,IPPROTO_IP, IP_HDRINCL,&one,sizeof(one))<0){
		perror("Error seeting IP header");
		exit(0);
	}

	if(sendto(raw_tcp, datagram,tcp_packet_length, 0, (struct sockaddr *)ipv4, sizeof(struct sockaddr_in)) <0){
		perror("sendto failed");
	}

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
		usleep(100);
		if(sendto(network_socket,low_train2[b],packet_length,MSG_CONFIRM,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
			perror("error");
		}
		
	}

	data = "tail1";

	struct ip iphdr_2;
	memcpy(&iphdr_2, &iphdr, sizeof(struct ip));

	char datagram_2[4096];
	memset(datagram_2,0,4096);
	memcpy(datagram_2,&iphdr_2,sizeof(struct ip));
	memcpy(datagram_2+ sizeof(struct ip) + sizeof(struct tcphdr),data,strlen(data));

	struct tcphdr tcphdr_2;
	memcpy(&tcphdr_2,&tcp_hdr, sizeof(struct tcphdr));

	process_syn(iphdr_2,ipv4,data,tcphdr_2,pseudogram,datagram_2,1,tail_port,pseudo_size,raw_tcp,tcp_packet_length);

	 data = "head2";
	 struct ip iphdr_3;
	 memcpy(&iphdr_3,&iphdr,sizeof(struct ip));

	 char datagram_3[4096];
	 memset(datagram_3,0,4096);
	 memcpy(datagram_3,&iphdr_3,sizeof(struct ip));
	 memcpy(datagram_3 + sizeof(struct ip) + sizeof(struct tcphdr),data,strlen(data));

	 struct tcphdr tcphdr_3;
	 memcpy(&tcphdr_3,&tcp_hdr, sizeof(struct tcphdr));

	 process_syn(iphdr_3,ipv4,data,tcphdr_3,pseudogram,datagram_3,2,head_port,pseudo_size,raw_tcp,tcp_packet_length);
	
	for(int i =0;i<train_size;i++){
		usleep(100);
		if(sendto(network_socket,high_train2[i],packet_length,MSG_CONFIRM,(const struct sockaddr*)&server_address,sizeof(server_address))<0){
			perror("error");
		}
	}

	data = "tail2";
	struct ip iphdr_4;
	memcpy(&iphdr_4,&iphdr,sizeof(struct ip));

	char datagram_4[4096];
	memset(datagram_4,0,4096);
	memcpy(datagram_4,&iphdr_4,sizeof(struct ip));
	memcpy(datagram_4 + sizeof(struct ip) + sizeof(struct tcphdr),data,strlen(data));

	struct tcphdr tcphdr_4;
	memcpy(&tcphdr_4,&tcp_hdr, sizeof(struct tcphdr));
	process_syn(iphdr_4,ipv4,data,tcphdr_4,pseudogram,datagram_4,3,tail_port,pseudo_size,raw_tcp,tcp_packet_length);

	printf("packets sent!\n");
	
	//Free the array of packets and the payload
	free(low_entropy);
	free(myRandomData);
	free_array(low_train2,train_size);
	free_array(high_train2,train_size);
	close(network_socket);
	close(raw_tcp);

    return 0;

}
