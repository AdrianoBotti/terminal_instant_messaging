#include "../headers/msg_client.h"

int requestDeregister(char* command, int TCP_sock)
{
	int ret;
	char server_reply[BUFFER_SIZE];

	strcat(command, " ");
	strcat(command, username);
	// sending command <!deregister username>	
	ret = sendTCP(command, TCP_sock);
	if(ret != 0){
		printf("Error requestDeregister --> sendTCP()\n");
		return 1;		
	}
	// waiting for server to reply with the result
	ret = receiveTCP(server_reply, TCP_sock);
	if(ret != 0){
		printf("Error requestDeregister --> receiveTCP()\n");
		return 1;		
	}	
	printf("%s\n", server_reply);
	if( strcmp(server_reply, "Client deregistered!") != 0 ){
		printf("Error requestDeregister : %s\n", server_reply);		
		return 1;	
	}

	strcpy(username, "");		
	return 0;	
}
