/*
	servidorUDP.c
	Servidor que escucha por paquetes en el puerto 5000.
	Imprime en la línea de comandos los datos recibidos
	así como la dirección y puerto del emisor.
	Contesta con el string recibido en mayúsculas
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

#define PORT 5000 //define port

typedef struct
{
	struct sockaddr_in; 
	string username; 

} addrUsername;


int main(void){
	int sockfd;
	struct sockaddr_in me;    //identify self
	struct sockaddr_in client; //identify client
	int addr_len, numbytes; //sturcture length and number of bytes
	char buffer[80];

	//structure to hold 30 addresses
	int ADDR_BUFFER_SIZE = 30; 
	struct addrUsername addrBuff[ADDR_BUFFER_SIZE];

	//Technically socket is a connection IP-PORT connected to IP-PORT
	//From own perspective it is the other machines IP-PORT

	//create socket file descriptor
	//0 means the protocol => UDP
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
	printf("Server is listening port 5000\n");

	do{

	// Revieve string of the client 
		if ((numbytes=recvfrom(sockfd, buffer, 80-1 , 0, 
				(struct sockaddr *) &client, &addr_len)) == -1) {
			perror("Error in recvfrom");
			exit(1);
		}

		buffer[numbytes] = '\0';
		printf("Received from %s/%d: %s\n",inet_ntoa(client.sin_addr), 
						ntohs(client.sin_port), buffer);

		//if address is in array
			//add username to message or something
			//for loop to send message to every address in array

		//else 

			//add address & username to struct


		addAddressToBuff(client);

// Send modified string to client
		if ((numbytes=sendto(sockfd, buffer, strlen(buffer), 0, 
			(struct sockaddr *)&client, sizeof(struct sockaddr))) == -1){
			perror("sendto");
			exit(1);
		}

	}while(1);
	close(sockfd);
	return 0;
}

void addAddressToBuff(sockaddr_in *addr){
	assert(addrBuffTail >= 0 && addrBuffTail < ADDR_BUFFER_SIZE);
    addrBuff[addrBuffTail++] = *addr;
    if (addrBuffTail >= ADDR_BUFFER_SIZE)
        addrBuffTail = 0;
}