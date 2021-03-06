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

struct addrUsername
{
	struct sockaddr_in addr; //address of client
	char *username; //username of client
	struct addrUsername *next; //points to the next structure in "list" or null if is the last
};

int addAddressToBuff(struct sockaddr_in addr, char *username);
bool isUsernameAvailable(char *username);
void deleteAddr(struct sockaddr_in *addr);
char *getUserName(struct sockaddr_in *addr);
bool isInBuffer(struct sockaddr_in *addr);
bool startsWith(const char *pre, const char *str); 
struct sockaddr_in getAddrByUsername(char *username);
struct sockaddr_in getAddrByAddress(struct sockaddr_in *addr);

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

		//Check if client IP address is already known
		if (isInBuffer(&client)) 
		{
			char tempbuff[110]; 
			//copy the username to tempbuff
			strcpy(tempbuff, getUserName(&client));

			//if the string received is /q the user wants to quit and is deleted from the address list
			if (strcmp(buffer,"/q") == 0){
				
				//get the address of the listening client
				struct sockaddr_in lcaddr = getAddrByAddress(&client);

				//send "/q" to the sender (listening client) to kill it
				if ((numbytes=sendto(sockfd, "/q", strlen("/q"), 0, 
						(struct sockaddr *) &lcaddr, sizeof(struct sockaddr))) == -1){
						perror("sendto");
						exit(1);
					}
				//deletes user from addresses
				deleteAddr(&client);
				current = first; 
				//tempbuff already has the username, add leaving string
				strcat(tempbuff, " just left the chat.");
				//send tempbuff to all the clients
				while (current)
				{
					if ((numbytes=sendto(sockfd, tempbuff, strlen(tempbuff), 0, 
						(struct sockaddr *)&(current->addr), sizeof(struct sockaddr))) == -1){
						perror("sendto");
						exit(1);
					}
					current = current->next; 
				}
			}
			else if (startsWith("/p ", buffer)){

			   char senderUsername[10];
			   strncpy(senderUsername, tempbuff, 10); //stores senders username 

			   const char s[2] = " ";
			   char *token;
			   
			   /* get the first token */
			   token = strtok(buffer, s);	
			   token = strtok(NULL, s);
			   char receiverUsername[10];
			   char tempmsg[70];
			   strncpy(receiverUsername, token, 10); //stores receivers username
			   token = strtok(NULL, s);

			   strcpy(tempmsg, "");

			   //loop words of private message and make message string
			   while( token != NULL ) 
			   {
			   		strcat(tempmsg, token);
			   		strcat(tempmsg, " ");
			   		token = strtok(NULL, s);
			   }

			   //create string to be sent
			   sprintf(tempbuff, "PrivateMSG from %s: %s", senderUsername, tempmsg);

			   client = getAddrByUsername(receiverUsername); //get the address of the receiver
			   //send to the receiver
			   if ((numbytes=sendto(sockfd, tempbuff, strlen(tempbuff), 0, 
						(struct sockaddr *)&client, sizeof(struct sockaddr))) == -1){
						perror("sendto");
						exit(1);
					}

				//create string to be send to sender
				sprintf(tempbuff, "PrivateMSG to %s: %s", receiverUsername, tempmsg);

				client = getAddrByUsername(senderUsername); //get the address of the sender
				//send to the sender
			   	if ((numbytes=sendto(sockfd, tempbuff, strlen(tempbuff), 0, 
						(struct sockaddr *)&client, sizeof(struct sockaddr))) == -1){
						perror("sendto");
						exit(1);
					}


			}
			else{
				//add message (buffer) to tempbuff which contains the username
				strcat(tempbuff, ": ");
				strcat(tempbuff, buffer);

				current = first; 
				//loop through all the addresses and send the message with the username
				while (current != NULL)
				{
					//printf("Send to %s/%d: %s\n",inet_ntoa((current->addr).sin_addr), ntohs((current->addr).sin_port), buffer);
					if ((numbytes=sendto(sockfd, tempbuff, strlen(tempbuff), 0, 
						(struct sockaddr *)&(current->addr), sizeof(struct sockaddr))) == -1){
						perror("sendto");
						exit(1);
					}

					current = current->next; 
				}
			}
		}
		else 
		{
			//executed if the IP address is new
			//check if username is available
			if (!isUsernameAvailable(buffer))
			{
				//send usrErr to sender to tell to give new username
				//asking the username again is handled in the listeningClient
				if ((numbytes=sendto(sockfd, "usrErr", strlen("usrErr"), 0, 
					(struct sockaddr *)&client, sizeof(struct sockaddr))) == -1){
					perror("sendto");
					exit(1);
				}
			}
			else{

				char *username;
				username = strdup(buffer); //store the username from the buffer

				//Add address & username to address buffer
				//returned value users is the number of users in chat
				users = addAddressToBuff(client, buffer);

				//Store message to the buffer
				sprintf(buffer, "Welcome to the chat! Currently we have %d users: ", users);
				//Loop through all the users and add their username to the message
				current = first; 
				while (current != NULL)
				{
					strcat(buffer, " ");
					strcat(buffer, current->username);
					current = current->next; 
				}
				//printf("Sending %s\n", buffer);

				//send the welcome message and the usernames (buffer) to the new user
				if ((numbytes=sendto(sockfd, buffer, strlen(buffer), 0, 
						(struct sockaddr *) &client, sizeof(struct sockaddr))) == -1){
						perror("sendto");
						exit(1);
				}

				//Write new message to the buffer
				sprintf(buffer, "%s joined the chat", username);
				
				//loop through all the users and send them message saying that new user joined
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

//add new address and username to address list, returns number of users
int addAddressToBuff(struct sockaddr_in addr, char *username){

	int users = 0; 

	//allocate memory for the new addrUsername structure
	struct addrUsername *newStruct = malloc(sizeof(struct addrUsername));

	newStruct->username = strdup(username); //store the username
	newStruct->addr = addr; //store address

	newStruct->next = NULL; //initialize next as empty

	//first is a global variable pointing to the first addrUsername structure of our buffer
	if (!first)
	{
		//if there is no first addrUsername structure, the new structure is set as the first
		first = newStruct; 
		users++; //count number of users
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
			users++; //count the users 
		}
		current->next = newStruct; 
		users++; 
	}
	return users; //return number of users
}

//returns username by IP address
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

//checks if given IP is in the address list and returns true (1) or false (0)
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

//checks if username is available 
bool isUsernameAvailable(char *username){
	struct addrUsername *current; 
	current = first; 
	while (current != NULL)
	{
		if (strcmp(current->username, username) == 0)
		{
			return 0;
		}
		current = current->next; 
	}
	return 1; 
}

//returns address when username is known
struct sockaddr_in getAddrByUsername(char *username){
	struct addrUsername *current; 
	current = first; 
	while (current != NULL)
	{
		//compares username in struct to given parameter username
		if (strcmp(current->username, username) == 0)
		{
			return current->addr; 
		}
		current = current->next; 
	}
}

//returns listener address by IP
struct sockaddr_in getAddrByAddress(struct sockaddr_in *addr){
	struct addrUsername *current; 
	current = first; 
	//loop to go thourgh all the addresses and check if the wanted address is found
	while (current != NULL)
	{
		if (((current->addr).sin_addr).s_addr == (&addr->sin_addr)->s_addr)
		{
			return current->addr; 
		}
		current = current->next; 
	}
}

//deletes address from the address list
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
		first = next; 
	}
	//free the memory allocated for the addrUsername structure
	free(current->username);
	free(current);
}

//check if string starts with some string
bool startsWith(const char *pre, const char *str)
{
   if(strncmp(pre, str, strlen(pre)) == 0) return 1;
   return 0;
}