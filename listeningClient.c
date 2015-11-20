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

#define PORT 6000  //listening in Port 6000, if changed needs change also in server

int main(int argc, char *argv[]){
	int sockfd;
	struct sockaddr_in me;    //identify self
	struct sockaddr_in client; //identify client
	int addr_len, numbytes; //sturcture length and number of bytes
	char buffer[92]; //longer buffer to be able to receive also username

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("Socket error");
		exit(1);
	}

	me.sin_family = AF_INET;         
	me.sin_port = htons(PORT);     
	me.sin_addr.s_addr = INADDR_ANY; 
	memset(&(me.sin_zero), '\0', 8); 

	if (bind(sockfd, (struct sockaddr *)&me, sizeof(struct sockaddr)) == -1) {
		perror("Binding error");
		exit(1);
	}

	addr_len = sizeof(struct sockaddr);
	printf("Listening port 6000, open sending client to log on to chat and send messages\n");

	do{
		// Receive string from server
		if ((numbytes=recvfrom(sockfd, buffer, 92-1 , 0, 
				(struct sockaddr *) &client, &addr_len)) == -1) {
			perror("Error in recvfrom");
			exit(1);
		}
		buffer[numbytes] = '\0';
		printf("%s\n", buffer);

	} while (1);

	close(sockfd);
	return 0;
} 