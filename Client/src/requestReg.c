#include "../headers/msg_client.h"

int requestRegister(char* command, int TCP_sock, char* IP_address, char* UDP_port)
{
	int ret;
	char server_reply[BUFFER_SIZE];
	char* argument;

	//sending command <!register username>
	ret = sendTCP(command, TCP_sock);
	if(ret != 0){
		printf("Error requestRegister --> sendTCP(1)\n");
		return 1;		
	}
	//waiting for server to check the validity of the command
	ret = receiveTCP(server_reply, TCP_sock);
	if(ret != 0){
		printf("Error requestRegister --> receiveTCP(1)\n");
		return 1;		
	}	
	//if this is a new client or an offline client
	if( strcmp(server_reply, "Waiting for informations") == 0 )
	{
		//send ip 
		ret = sendTCP(IP_address, TCP_sock);
		if(ret != 0){
			printf("Error requestRegister --> sendTCP(2)\n");
			return 1;		
		}
		//send udp port 
		ret = sendTCP(UDP_port, TCP_sock);
		if(ret != 0){
			printf("Error requestRegister --> sendTCP(3)\n");
			return 1;		
		}
		ret = receiveTCP(server_reply, TCP_sock);
		if(ret != 0){
			printf("Error requestRegister --> receiveTCP(2)\n");
			return 1;	
		}
		// initializing global variable "username"
		strtok(command, "\n");
		strtok(command, " ");
		argument = strtok(NULL, " ");		
		strcpy(username, argument);				
	}
	printf("%s\n", server_reply);
	return 0;	
}
