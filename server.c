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
	int tcp_port;
} server_init;

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

int get_port(const char *filename)
{
	FILE *fp;

	server_init port;

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
	port.tcp_port = item->valueint;
	free(buffer);
	return port.tcp_port;
}

int main(int argc, char **argv)
{
	char server_message[256];
	char udp_train[256];
	char destination_port[256];
	int preprobe_port = get_port(argv[1]);

	// server socket created.
	int probe_socket;
	probe_socket = socket(AF_INET, SOCK_STREAM, 0);

	// define server address
	struct sockaddr_in probe_address;
	probe_address.sin_family = AF_INET;
	probe_address.sin_port = htons(preprobe_port);
	probe_address.sin_addr.s_addr = inet_addr("192.168.86.249");
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

	char bytes[atoi(udp_train)];

	int server_socket;
	server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	// //define server address
	struct sockaddr_in server_address, client_address;

	memset(&server_address, 0, sizeof(server_address));
	memset(&client_address, 0, sizeof(client_address));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(destination_port));
	server_address.sin_addr.s_addr = inet_addr("192.168.86.249");

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

	struct timespec start1,stop1,start2,stop2;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start1);
	for (int i = 0; i < train; i++)
	{
		recvfrom(server_socket, bytes, sizeof(bytes), MSG_WAITALL, (struct sockaddr *)&client_address, &len);
	}
	clock_gettime(CLOCK_MONOTONIC_RAW, &stop1);
	uint64_t result1 = (stop1.tv_sec - start1.tv_sec) * 1000000 + (stop1.tv_nsec - start1.tv_nsec) / 1000;


	// timer = clock() - timer;
	// double time_taken = ((double)timer) / CLOCKS_PER_SEC;

	printf("Receiving high entropy packets...\n");
	clock_t timer2;
	// timer2 = clock();
	clock_gettime(CLOCK_MONOTONIC_RAW, &start2);
	for (int i = 0; i < train; i++)
	{
		recvfrom(server_socket, bytes, sizeof(bytes), MSG_WAITALL, (struct sockaddr *)&client_address, &len);
	}
	uint64_t result2 = (stop2.tv_sec - start2.tv_sec) * 1000000 + (stop2.tv_nsec - start2.tv_nsec) / 1000;
	// timer2 = clock() - timer2;
	// double time_taken2 = ((double)timer2) / CLOCKS_PER_SEC;
	// printf("%f\n", time_taken2);

	printf("High entropy packets received!\n");
	// close the socket
	close(server_socket);
	// double total_time = (time_taken2 - time_taken) * ((double)1000);
	// printf("send time: %f\n", total_time);
	char *report;

	uint64_t res = result2 - result1;
	printf("%"PRIu64 "\n",res);

	if( res> 100){
		report = "compression detected";
	}else{
		report = "no compression detected";
	}

	int postProbe_socket;
	postProbe_socket = socket(AF_INET,SOCK_STREAM,0);
	
	struct sockaddr_in post_probe_address;
	post_probe_address.sin_family = AF_INET;
	post_probe_address.sin_port = htons(preprobe_port);
	post_probe_address.sin_addr.s_addr = inet_addr("192.168.86.249");

	if(postProbe_socket == -1){
		perror("gay");
	}
	setsockopt(postProbe_socket, SOL_SOCKET, SO_REUSEADDR,&value,sizeof(val));

	if(bind(postProbe_socket, (struct sockaddr *)&post_probe_address,sizeof(post_probe_address))<0){
		perror("skill issue");
	};
	listen(postProbe_socket, 5);
	char sent_time[256]="balls";

	client_socket = accept(postProbe_socket,NULL,NULL);
	send(client_socket,(char *)report,sizeof(sent_time),0);
	close(postProbe_socket);
	return 0;
}
