/*
	servidorUDP.c
	Servidor que escucha por paquetes en el puerto 5000.
	Imprime en la línea de comandos los datos recibidos
	así como la dirección y puerto del emisor.
	Contesta con el string recibido en mayúsculas
*/
#include <assert.h>
#include <stdbool.h>
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
	struct sockaddr_in addr; 
	char username[10]; 

} addrUsername;

bool isAddrInBuff(struct sockaddr_in *addr);
void addAddressToBuff(struct sockaddr_in addr, char *username);

//structure to hold 30 addresses
int ADDR_BUFFER_SIZE; 
addrUsername addrBuff[30];
int addrBuffTail; 

int length, i;

int main(void){
	int sockfd;
	struct sockaddr_in me;    //identify self
	struct sockaddr_in client; //identify client
	int addr_len, numbytes; //sturcture length and number of bytes
	char buffer[80];

	//initialize global variables
	ADDR_BUFFER_SIZE = 30; 
	addrBuffTail = 0; 

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

		if (isAddrInBuff(&client))
		{
			//add username to message or something

			length = sizeof(addrBuff) / sizeof(addrBuff[0]); 
			for (i=0; i<length-1; i++)
			{
				printf("Sending to buffer %s \n", inet_ntoa(addrBuff[i].addr.sin_addr));
				if ((numbytes=sendto(sockfd, buffer, strlen(buffer), 0, 
					(struct sockaddr *)&(addrBuff[i].addr), sizeof(struct sockaddr))) == -1){
					perror("sendto");
					exit(1);
				}
			}
		}
		else 
		{
			if (sizeof(buffer) > 10)
			{
				//send username too long or something
			} 
			addAddressToBuff(client, buffer);
		}

		//DELETING THE USERNAME && IP WHEN CLIENT ENDING!!!

	}while(1);
	close(sockfd);
	return 0;
}

void addAddressToBuff(struct sockaddr_in addr, char *username){

	addrUsername newStruct; 
	strcpy(newStruct.username, username); 
	newStruct.addr = addr; 

	assert(addrBuffTail >= 0 && addrBuffTail < ADDR_BUFFER_SIZE);
    addrBuff[addrBuffTail++] = newStruct;
    if (addrBuffTail >= ADDR_BUFFER_SIZE)
        addrBuffTail = 0;
}

bool isAddrInBuff(struct sockaddr_in *addr){

	length = sizeof(addrBuff) / sizeof (addrBuff[0]); 

	for (i = 0; i<length; i++)
	{
		if (addrBuff[i].addr.sin_addr.s_addr == (&addr->sin_addr)->s_addr)
		{
			return 1; 
		}
	}
	return 0; 
}