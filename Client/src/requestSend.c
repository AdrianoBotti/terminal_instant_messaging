#include "../headers/msg_client.h"

int requestSend(char* command, int TCP_sock, int UDP_sock)
{
	int ret;
	char server_reply[BUFFER_SIZE];
		
	//sending command <!send username>
	ret = sendTCP(command, TCP_sock);
	if(ret != 0){
		printf("Error requestSend --> sendTCP()\n");
		return 1;		
	}
	
	// waiting for server reply: recipient could be online or offline.
	// if recipient is offline, message can be sent in offline mode.
	// recipient could not exist at all too.
	ret = receiveTCP(server_reply, TCP_sock);
	if(ret != 0){
		printf("Error requestSend--> receiveTCP()\n");
		return 1;		
	}	

	// username not found
	if( strcmp(server_reply, " !!! Error: username not found") == 0){
		printf("%s\n", server_reply);
		return 0;	
	}
	// online mode : recipient found and online
	if( strcmp(server_reply, "-- Sending recipient's infos...") == 0 )
	{
		printf("-- Receiving recipient's infos...\n");
		ret = onlineMode(TCP_sock, UDP_sock);
		if(ret != 0){
			printf("Error requestSend --> onlineMode()\n");
			return 1;		
		}
	}
	// offline mode : recipient found but offline
	if( strcmp(server_reply, "-- Recipient offline, do you want to send an offline message?[y/n]") == 0 )
	{
		printf("%s\n", server_reply);
		ret = offlineMode(TCP_sock);
		if(ret != 0){
			printf("Error requestSend --> offlineMode()\n");
			return 1;		
		}
	}
 	return 0;
}

int offlineMode(int TCP_sock)
{
	int ret;
	char text_buffer[BUFFER_SIZE];	
	char buffer_out[BUFFER_SIZE]; 
	char input[BUFFER_SIZE];
	
	strcpy(buffer_out, username);
	strcat(buffer_out, ":\n");
	
	// check for a valid input by the user
	while(1)
	{
		memset(input, 0, BUFFER_SIZE);
		fgets(input, BUFFER_SIZE, stdin);
		if( (strcmp(input, "y\n")!= 0) && (strcmp(input, "n\n")!= 0) )
			printf("Enter a valid input\n");		
		else 
			break;
	}	
	//sending user's reply [y/n]
	ret = sendTCP(input, TCP_sock);
	if(ret != 0){
		printf("Error offlineMode --> sendTCP(1)\n");
		return 1;		
	}	
	// if answer is "no", we abort the command
	if( strcmp(input,"n\n") == 0){
		printf("-- Offline mode refused\n");	
		return 0;
	}
	// if "yes", we get the actual text of the message
	if(strcmp(input, "y\n") == 0){
		printf("\nTo end the message, type <dot><ENTER> on a new line\n\nText:\n");
		while(1)
		{
			fgets(text_buffer, BUFFER_SIZE, stdin);
			if(text_buffer == NULL){
				printf("Error offlineMode --> fgets()\n ");	
				return 1;
			}
			if( strcmp(text_buffer, ".\n") == 0){				
				break;			
			}
			strcat(buffer_out, text_buffer);
		}
	}
	// message is now ready, we send it to the server
	ret = sendTCP(buffer_out, TCP_sock);
	if(ret != 0){
		printf("Error offlineMode --> sendTCP(2)\n");
		return 1;		
	}
return 0;
}
	
int onlineMode(int TCP_sock, int UDP_sock)
{
	int ret;
	char IP_address[BUFFER_SIZE];	
	char UDP_port[BUFFER_SIZE]; 
	char text_buffer[BUFFER_SIZE];
	char buffer_out[BUFFER_SIZE];
	struct sockaddr_in peer_addr;
	
	strcpy(buffer_out, username);
	strcat(buffer_out, " says (online) :\n");
	
	// receiving IP	
	ret = receiveTCP(IP_address, TCP_sock);
	if( ret != 0){
		printf("Error onlineMode --> receiveTCP(1)\n");
		return 1;		
	}
	// receiving UDP port
	ret = receiveTCP(UDP_port, TCP_sock);
	if( ret != 0){
		printf("Error onlineMode --> receiveTCP(2)\n");
		return 1;		
	}
	printf("Informations acquired: %s, %s\n", IP_address, UDP_port);
	
	printf("\nTo end the message, type <dot><ENTER> on a new line\n\nText:\n");
	while(1)
	{
		fgets(text_buffer, BUFFER_SIZE, stdin);
		if(text_buffer == NULL){
			printf("Error offlineMode --> fgets()\n ");	
			return 1;
		}
		if( strcmp(text_buffer, ".\n") == 0){	
			break;			
		}
		strcat(buffer_out, " - ");
		strcat(buffer_out, text_buffer);
	}	
	// sending on UDP port
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(atoi(UDP_port));
	inet_pton(AF_INET, IP_address, &peer_addr.sin_addr);
	
	sendUDP(buffer_out, UDP_sock, &peer_addr);
		
	return 0;
}
