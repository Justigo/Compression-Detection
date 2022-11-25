# Compression-Detection

The goal of this project is to be able to detect network compression between two parties on both the client and server implementation and the standalone implementation of this project. This project is broken up into two parts: the first part consists of a client and server applications, where you would need two vms to run this program and see it communicate with each other to report if there is "compression" detected. In the other part of the project, we have a standalone application that will send the UDP and TCP packets without any cooperation from a server.

## Building

TODO: how to compile project
* To compile the client and server application run these two commands.\

 ** gcc cJSON.c client.c -o client.
 ** gcc cJSON.c server.c -o server.
 
 * Next, to run the program you will have to call the file name plus the config file so it will be
 
 ** ./server myconfig.json
 ** ./client myconfig.json


## Import Notes
The first part is fully functional with the client being able to send the determinante amount of udp packets based on the information from the configuration file. The server is able to receive the packets, as well as calculate the time it takes for both the high and low entropy data to be received. In addition, the server is able to determine if there is compression detected or not, based on a certain threshhold for time taken to send the packets. It is compared to 100 milliseconds, where if it exceeds 100 milliseconds, there will be compression.

TODO: libraries needed to be added

## Screenshots

TODO: Project shown working
