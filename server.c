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
#define CPORT 6000 //define listening client port

struct addrUsername
{
	struct sockaddr_in addr; 
	char *username; 
	struct addrUsername *next; //points to the next structure in "list" or null if is the last

};

int addAddressToBuff(struct sockaddr_in addr, char *username);
bool isUsernameAvailable(char *username);
void deleteAddr(struct sockaddr_in *addr);
char *getUserName(struct sockaddr_in *addr);
bool isInBuffer(struct sockaddr_in *addr);

//pointer to the first addrUsername structure in list
struct addrUsername *first = NULL; 

int length, i;

int main(void){
	int sockfd, users;
	struct addrUsername *current; //pointer to "current" address, used in loops
	struct sockaddr_in me;    //identify self
	struct sockaddr_in client; //identify client
	int addr_len, numbytes; //sturcture length and number of bytes
	char buffer[80];

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

		if (isInBuffer(&client)) 
		{

			//CHECK IF PRIVATE MESSAGE OR QUIT MESSAGE 

			char tempbuff[92]; 
			//copy the username to tempbuff
			strcpy(tempbuff, getUserName(&client));

			//add message (buffer) to tempbuff which contains the username
			strcat(tempbuff, ": ");
			strcat(tempbuff, buffer);

			current = first; 
			while (current != NULL)
			{
				printf("Send to %s/%d: %s\n",inet_ntoa((current->addr).sin_addr), ntohs((current->addr).sin_port), buffer);
				if ((numbytes=sendto(sockfd, tempbuff, strlen(buffer), 0, 
					(struct sockaddr *)&(current->addr), sizeof(struct sockaddr))) == -1){
					perror("sendto");
					exit(1);
				}

				current = current->next; 
			}
		}
		else 
		{
			if (!isUsernameAvailable(buffer))
			{
				if ((numbytes=sendto(sockfd, "usrErr", strlen(buffer), 0, 
					(struct sockaddr *)&client, sizeof(struct sockaddr))) == -1){
					perror("sendto");
					exit(1);
				}
			}
			else{

				char *username;
				username = strdup(buffer);

				//Add address to address buffer
				users = addAddressToBuff(client, buffer);

				//Send welcome message to listening client 
				sprintf(buffer, "Welcome to the chat! Currently we have %d users: ", users);
				current = first; 
				while (current != NULL)
				{
					strcat(buffer, " ");
					strcat(buffer, current->username);
					current = current->next; 
				}
				printf("Sending %s\n", buffer);
				if ((numbytes=sendto(sockfd, buffer, strlen(buffer), 0, 
						(struct sockaddr *) &client, sizeof(struct sockaddr))) == -1){
						perror("sendto");
						exit(1);
				}

				//Send new users username to all the clients in chat 
				sprintf(buffer, "%s joined the chat", username);
				current = first; 
				while (current != NULL)
				{
					if ((numbytes=sendto(sockfd, buffer, strlen(buffer), 0, 
						(struct sockaddr *)&(current->addr), sizeof(struct sockaddr))) == -1){
						perror("sendto");
						exit(1);
					}

					current = current->next; 
				}
				free(username);
			}
		}

		//DELETING THE USERNAME && IP WHEN CLIENT ENDING!!!

	}while(1);
	close(sockfd);

	//this would free the memory, if normal exit of program happened
	//however, server is always terminated with ctrl+c or similar so memory is freed automatically
	//this point is actually never reached 
	struct addrUsername *former; 
	current = first; 
	while (current->next != NULL)
	{
		former = current; 
		current = current->next; 
		free(former->username);
		free(former);
	}
	free(current->username);
	free(current);
	return 0;
}

int addAddressToBuff(struct sockaddr_in addr, char *username){

	int users = 0; 

	//allocate memory for the new addrUsername structure
	struct addrUsername *newStruct = malloc(sizeof(struct addrUsername));

	newStruct->username = strdup(username);
	newStruct->addr = addr; 
	//the port number of listening client needs to be separately defined
	(newStruct->addr).sin_port = htons(CPORT); 

	newStruct->next = NULL; 

	//first is a global variable pointing to the first addrUsername structure
	if (!first)
	{
		//if there is no first addrUsername structure, the new structure is set as the first
		first = newStruct; 
		users++; 
	}
	else
	{	
		users = 1; //first found, so atleast one user
		//loop to find out which is the last structure of the list
		//new structure is added to be the next from that
		struct addrUsername *current; 
		current = first; 
		while (current->next != NULL)
		{
			current = current->next; 
			users++; 
		}
		current->next = newStruct; 
		users++; 
	}
	return users; 
}

char *getUserName(struct sockaddr_in *addr){
	struct addrUsername *current; 
	current = first; 
	//loop to go thourgh all the addresses and check if the wanted address is found
	while (current != NULL)
	{
		if (((current->addr).sin_addr).s_addr == (&addr->sin_addr)->s_addr)
		{
			return current->username; 
		}
		current = current->next; 
	}
	return NULL; 
}

bool isInBuffer(struct sockaddr_in *addr){
	struct addrUsername *current; 
	current = first; 
	//loop to go thourgh all the addresses and check if the wanted address is found
	while (current != NULL)
	{
		if (((current->addr).sin_addr).s_addr == (&addr->sin_addr)->s_addr)
		{
			return 1; 
		}
		current = current->next; 
	}
	return 0; 
}

bool isUsernameAvailable(char *username){
	struct addrUsername *current; 
	current = first; 
	while (current != NULL)
	{
		if (strcmp(current->username, username) == 0)
		{
			return 0;
		}
	}
	return 1; 
}

void deleteAddr(struct sockaddr_in *addr){
	struct addrUsername *current; 
	struct addrUsername *former = NULL; 
	struct addrUsername *next = NULL; 
	current = first; 
	//loop to go through all the addresses and check if the wanted address is found
	while (current != NULL)
	{
		if (((current->addr).sin_addr).s_addr == (&addr->sin_addr)->s_addr)
		{
			next = current->next; 
			break; 
		}

		former = current; 
		current = current->next; 
	}

	if (former){
		//if there is next for the current, the next becomes the next of the former
		//if there is no next, the next points to NULL and next of former becomes NULL
		former->next = next; 

	}
	else{
		//the one to delete is the first one
		//if there is next for the current, the next becomes the first
		//if there is no next, the next points to NULL and first becomes NULL
		first->next; 
	}
	//free the memory allocated for the addrUsername structure
	free(current->username);
	free(current);
}