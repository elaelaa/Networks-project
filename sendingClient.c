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

#define PORT 5000    //sending through port 5000

int main(int argc, char *argv[]){
	int sockfd;
	struct sockaddr_in server;
	struct hostent *he;
	int numbytes;
	char buffer[80];
	char *username;
	int addr_len = sizeof(struct sockaddr);

	if (argc != 3) {
		fprintf(stderr,"use: clientProgramName serverIPaddr username\n");
		exit(1);
	}

	//he = host entity
	if ((he=gethostbyname(argv[1])) == NULL) {
		perror("gethostbyname");
		exit(1);
	}

	//Username length doesn't need to be checked anymore
	username = strdup(argv[2]);

	//socket("tcp-ip", "datagram", "udp")
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	server.sin_family = AF_INET;     
	server.sin_port = htons(PORT); 
	server.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(server.sin_zero), '\0', 8); 

	//Send given username and password
	if ((numbytes=sendto(sockfd, username, strlen(username), 0,
				(struct sockaddr *)&server, sizeof(struct sockaddr))) == -1){
			perror("sendto");
			exit(1);
	}

	do{
		if ((numbytes=recvfrom(sockfd, buffer, 80-1 , 0, (struct sockaddr *) &server, &addr_len)) == -1) {
			perror("Error in recvfrom");
			exit(1);
		}
		buffer[numbytes] = '\0';
		printf("Received from %s/%d: %s\n",inet_ntoa(server.sin_addr), ntohs(server.sin_port), buffer);
		if (strcmp("usrErr", buffer) != 0)
		{
			printf("%s\n", buffer);
			break;
		}
		printf("Username not available. Write a new one: ");
		fgets(buffer, 80, stdin);
		buffer[strlen(buffer)-1] = '\0';
		if ((numbytes=sendto(sockfd, buffer, strlen(buffer), 0,
				(struct sockaddr *)&server, sizeof(struct sockaddr))) == -1){
			perror("sendto");
			exit(1);
		}
	}while(1);

	//Receive OK or request for new username ??
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
// Received modified  string from server
		/*if ((numbytes=recvfrom(sockfd, buffer, 80-1 , 0, (struct sockaddr *) &server, &addr_len)) == -1) {
			perror("Error in recvfrom");
			exit(1);
		}
		buffer[numbytes] = '\0';
		printf("Received from %s/%d: %s\n",inet_ntoa(server.sin_addr), ntohs(server.sin_port), buffer);
	*/
	} while (strcmp(buffer,"Q") != 0);

	close(sockfd);
	return 0;
} 