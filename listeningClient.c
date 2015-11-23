/*
	UDPclient.c
	It connects to a server and sends messages read form the keyboard.
	The communication is UDP and the server port is 5000.
	The messages will be sent until 'q' is typed and sent.
	In the command line the IP address or the domain name of the server
	has to be specified.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 5000 //server port number

int main(int argc, char *argv[]){
	int numbytes;
	int sockfd;
	struct sockaddr_in server;
	struct hostent *he;
	char buffer[110];
	char username[10];
	int addr_len = sizeof(struct sockaddr);

	if (argc != 3) {
		fprintf(stderr,"use: clientProgramName serverIPaddr username[10]\n");
		exit(1);
	}

	//he = host entity
	if ((he=gethostbyname(argv[1])) == NULL) {
		perror("gethostbyname");
		exit(1);
	}

	//check the length of username
	if (strlen(argv[2])>10){
		perror("username too long");
		exit(1);
	}
	//store the username
	strcpy(username, argv[2]);

	//socket("tcp-ip", "datagram", "udp")
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	server.sin_family = AF_INET;     
	server.sin_port = htons(PORT); 
	server.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(server.sin_zero), '\0', 8); 
	
	//Send given username to the server
	if ((numbytes=sendto(sockfd, username, strlen(username), 0,
				(struct sockaddr *)&server, sizeof(struct sockaddr))) == -1){
			perror("sendto");
			exit(1);
	}

	do{
		//listen the response of the server (about the username validity)
		if ((numbytes=recvfrom(sockfd, buffer, 110-1 , 0, (struct sockaddr *) &server, &addr_len)) == -1) {
			perror("Error in recvfrom");
			exit(1);
		}
		buffer[numbytes] = '\0';
		//printf("Received from %s/%d: %s\n",inet_ntoa(server.sin_addr), ntohs(server.sin_port), buffer);
		
		//if username is not valid, usrErr is received
		if (strcmp("usrErr", buffer) != 0)
		{
			//if username was valid, print whatever was received  (welcome...)
			printf("%s\n", buffer);
			break; //break out of the loop
		}
		//username was not valid, print ...
		printf("Username not available. Write a new one: \n");
		
		//get input from command line again
		fgets(buffer, 110, stdin);
		buffer[strlen(buffer)-1] = '\0';
		//check the length and get new one if too long
		while(strlen(buffer)>10){
			printf("Username too long. Write a new one: \n");
			fgets(buffer, 110, stdin);
			buffer[strlen(buffer)-1] = '\0';
		}
		//send new username to the server again
		if ((numbytes=sendto(sockfd, buffer, strlen(buffer), 0,
				(struct sockaddr *)&server, sizeof(struct sockaddr))) == -1){
			perror("sendto");
			exit(1);
		}
	}while(1); //loop until breaked out from

	//actual listening
	do{
		// Receive string from server
		if ((numbytes=recvfrom(sockfd, buffer, 110-1 , 0, (struct sockaddr *) &server, &addr_len)) == -1) {
			perror("Error in recvfrom");
			exit(1);
		}
		buffer[numbytes] = '\0';
		printf("%s\n", buffer); //print what was received

	} while (strcmp(buffer,"/q") != 0);

	close(sockfd);
	return 0;
} 