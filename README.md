# Compression-Detection

The goal of this project is to be able to detect network compression between two parties on both the client and server implementation and the standalone implementation of this project. This project is broken up into two parts: the first part consists of a client and server applications, where you would need two vms to run this program and see it communicate with each other to report if there is "compression" detected. In the other part of the project, we have a standalone application that will send the UDP and TCP packets without any cooperation from a server.

To edit the configurations for the program go to the config file. In the config file there are many things you can edit. Here is a list of things that you are able to modify for the project:

* TCP port
* Source Port
* Destination Port
* Client IP Address
* Server IP Address
* TTL(Time to Live)
* Itermit Time
* Size of The Payload
* Number of Packets

For the standalone part of the project there are two additional parameters that you will need to account for:
 
* The Head Port
* The Tail Port

What happens is that the server will read the config file and only grab the TCP port to listen to any client connections... Next it will begin to listen to important config file information to receive the packets which are the client ip, the destination port, the size of the payload, and the number of packets. After that the server will get the packets received from the client


## Building

* To have the first part running, we are going to need two VMs that are linked together.(Note You will have to hardcode your MAC address and the interface for this part of the project.
* To compile the client and server application run these two commands:

```
gcc cJSON.c client.c -o client.
 
gcc cJSON.c server.c -o server.
```
 * For the standalone application we only need one VM that can be run on the standard connection.
 * Next, to run the program you will have to call the file name plus the config file so it will be like this
```
./server myconfig.json
./client myconfig.json
```


## Important Notes
The first part is fully functional with the client being able to send the determinante amount of udp packets based on the information from the configuration file. The server is able to receive the packets, as well as calculate the time it takes for both the high and low entropy data to be received. In addition, the server is able to determine if there is compression detected or not, based on a certain threshhold for time taken to send the packets. It is compared to 100 milliseconds, where if it exceeds 100 milliseconds, there will be compression. 

What has not been done is capturing the RST packets and timing it for part 2. The only thing that is happening for project 2 is sending the head and tail packets for both low and high entropy data.

## TroubleShooting
Sometimes things can happen that will make it so that there are errors in the process. In the event that it happens, make sure that you do these things. 

* Make sure that the config file has the correct information that you want for the ports and the IP addresses
* For part 1 make sure the server is running first.
* Make sure that the VM is set to the correct connection profile with the corresponding address
* Make sure that the MAC address and interface is set correctly. To check the destination MAC address, run ``` arp -n```. To check if the interface is set correctly check by running ```route -n```

## Contact Information

Github:tlin41390
email:(tlin41@dons.usfca.edu)



