#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "cJSON.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <signal.h>

typedef struct
{
	char tcp_port[256];
	char server_ip[256];
}configurations;

void cleanExit(){
	exit(0);
}

size_t get_file_size(const char *filepath)
{
	if (filepath == NULL)
	{
		return 0;
	}
	struct stat filestat;
	memset(&filestat, 0, sizeof(struct stat));

	if (stat(filepath, &filestat) == 0)
	{
		return filestat.st_size;
	}
	else
	{
		return 0;
	}
}

//method to get the port number for tcp and the server address
configurations get_config(const char *filename,configurations settings)
{
	FILE *fp;

	size_t size = get_file_size(filename);
	if (size == 0)
	{
		printf("no size\n");
	}

	char *buffer = malloc(size + 1);
	if (buffer == NULL)
	{
		printf("malloc not successful\n");
	}

	memset(buffer, 0, size + 1);

	fp = fopen(filename, "rb");

	fread(buffer, 1, size, fp);
	fclose(fp);

	cJSON *json, *item;
	json = cJSON_Parse(buffer);
	item = cJSON_GetObjectItemCaseSensitive(json, "TCP_port");
	if(item == NULL){
		printf("TCP port information missing\n");
		exit(1);
	}
	strcpy(settings.tcp_port,item->valuestring);
	item = cJSON_GetObjectItemCaseSensitive(json,"server_ip");
	if(item == NULL){
		printf("Server IP missing\n");
		exit(1);
	}
	strcpy(settings.server_ip,item->valuestring);
	free(buffer);
	return settings;
}

int main(int argc, char **argv)
{
	char server_message[256];
	char udp_train[256];
	char destination_port[256];
	configurations settings;
	settings = get_config(argv[1],settings);

	// server socket created.
	int probe_socket;
	probe_socket = socket(AF_INET, SOCK_STREAM, 0);
	printf("beginning pre-probing phase...\n");

	// define server address
	struct sockaddr_in probe_address;
	probe_address.sin_family = AF_INET;
	probe_address.sin_port = htons(atoi(settings.tcp_port));
	probe_address.sin_addr.s_addr = inet_addr(settings.server_ip);
	int value = 1;
	setsockopt(probe_socket, SOL_SOCKET, SO_REUSEADDR,&value,sizeof(value));

	// bind the socket to the port and ip
	bind(probe_socket, (struct sockaddr *)&probe_address, sizeof(probe_address));

	listen(probe_socket, 5);

	int client_socket;
	client_socket = accept(probe_socket, NULL, NULL);

	// receive the message
	recv(client_socket, server_message, sizeof(server_message), 0);
	recv(client_socket, udp_train, sizeof(udp_train), 0);
	recv(client_socket, destination_port, sizeof(destination_port), 0);
	int train = atoi(udp_train);

	// close the socket
	close(probe_socket);
	signal(SIGTERM, cleanExit);
	signal(SIGINT, cleanExit);
	printf("preprobing phase finished...\n");

	char bytes[atoi(udp_train)];

	int server_socket;
	server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	// //define server address
	struct sockaddr_in server_address, client_address;

	memset(&server_address, 0, sizeof(server_address));
	memset(&client_address, 0, sizeof(client_address));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(destination_port));
	server_address.sin_addr.s_addr = inet_addr(settings.server_ip);

	client_address.sin_family = AF_INET;

	int val = IP_PMTUDISC_DO;
	setsockopt(server_socket, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));

	// bind the socket to the port and ip
	if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		printf("Binding failed");
	};

	int len, n;
	len = sizeof(client_address);

	printf("Receiving packets...\n");

	clock_t start_timer,end_timer;

	for (int i = 0; i < train; i++)
	{
		int packets = recvfrom(server_socket, bytes, sizeof(bytes), MSG_WAITALL, (struct sockaddr *)&client_address, &len);
		if(i==0 && packets > 0){
			start_timer = clock();
		}
	}
	end_timer = clock();
	//calculate the total time and convert it to milliseconds
	double total_time = (((double)end_timer) - ((double)start_timer))/((double)CLOCKS_PER_SEC);
	double low_entropy_time = total_time * 1000;
	printf("low entropy time %f\n",low_entropy_time);
	printf("low entropy packets received.\n");

	printf("Receiving high entropy packets...\n");
	start_timer = clock();
	for (int i = 0; i < train; i++)
	{
		int packets = recvfrom(server_socket, bytes, sizeof(bytes), MSG_WAITALL, (struct sockaddr *)&client_address, &len);
		if(i==0 && packets > 0){
			start_timer = clock();
		}
	}
	end_timer = clock();
	total_time = (((double)end_timer) - ((double)start_timer))/((double)CLOCKS_PER_SEC);
	double high_entropy_time = total_time * 1000;
	printf("high_entropy_time %f\n",high_entropy_time);
	printf("High entropy packets received!\n");
	// close the socket
	close(server_socket);
	//create the report on whether there is compression or not
	char *report;
	report = (char *)malloc(sizeof(char)*256);

	if(report == NULL){
		printf("error in allocating size for report\n");
	}

	if( high_entropy_time-low_entropy_time>(double)100){
		strcpy(report,"compression detected");
	}else{
		strcpy(report,"no compression detected");
	}

	//create the tcp socket for post probing
	int postProbe_socket;
	postProbe_socket = socket(AF_INET,SOCK_STREAM,0);
	
	struct sockaddr_in post_probe_address;
	post_probe_address.sin_family = AF_INET;
	post_probe_address.sin_port = htons(atoi(settings.tcp_port));
	post_probe_address.sin_addr.s_addr = inet_addr(settings.server_ip);

	//check to see if there is compression or not in the program
	if(postProbe_socket == -1){
		perror("no socket");
	}
	setsockopt(postProbe_socket, SOL_SOCKET, SO_REUSEADDR,&value,sizeof(val));

	if(bind(postProbe_socket, (struct sockaddr *)&post_probe_address,sizeof(post_probe_address))<0){
		perror("error in post probing phase");
	};
	listen(postProbe_socket, 5);

	client_socket = accept(postProbe_socket,NULL,NULL);
	send(client_socket,report,256,0);
	close(postProbe_socket);
	free(report);
	return 0;
}