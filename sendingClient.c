/*
	UDPclient.c
	It connects to a server and sends messages read form the keyboard.
	The communication is UDP and the server port is 5000.
	The messages will be sent until 'q' is typed and sent.
	In the command line the IP address or the domain name of the server
	has to be specified.
*/

#include <signal.h>
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

#define PORT 5000    //PORT number of the server

void endHandler(int dummy); //handles ending with ctrl+c

int numbytes;
int sockfd;
struct sockaddr_in server;

int main(int argc, char *argv[]){
	struct hostent *he;
	char buffer[80];
	int addr_len = sizeof(struct sockaddr);

	signal(SIGINT, endHandler); //handles ctrl+c ending

	if (argc != 2) {
		fprintf(stderr,"use: clientProgramName serverIPaddr\n");
		exit(1);
	}

	//he = host entity
	if ((he=gethostbyname(argv[1])) == NULL) {
		perror("gethostbyname");
		exit(1);
	}

	//socket("tcp-ip", "datagram", "udp")
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	server.sin_family = AF_INET;     
	server.sin_port = htons(PORT); 
	server.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(server.sin_zero), '\0', 8); 

	//print instructions of the program use
	printf("Instructions:\n Open listening client first to join the chat. \n \"/p username message\" for private messages.\n \"/q\" for quitting.\n");

	do{
		// Read a string from command line
		printf("Write a message: ");
		fgets(buffer, 80, stdin);
		buffer[strlen(buffer)-1] = '\0';
		
		// Send string to server
		if ((numbytes=sendto(sockfd, buffer, strlen(buffer), 0,
				(struct sockaddr *)&server, sizeof(struct sockaddr))) == -1){
			perror("sendto");
			exit(1);
		}
		printf("Sent %d bytes to %s\n", numbytes, inet_ntoa(server.sin_addr));

	} while (strcmp(buffer,"/q") != 0);

	close(sockfd);
	return 0;
} 

void endHandler(int dummy) {
    if ((numbytes=sendto(sockfd, "/q", strlen("/q"), 0,
				(struct sockaddr *)&server, sizeof(struct sockaddr))) == -1){
			perror("sendto");
			exit(1);
		}

	close(sockfd);
	exit(0); 
}