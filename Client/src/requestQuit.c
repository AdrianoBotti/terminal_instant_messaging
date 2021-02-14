#include "../headers/msg_client.h"

int requestQuit(char* command, int TCP_sock)
{
	int ret;
	char server_reply[BUFFER_SIZE];

	strcat(command, " ");
	strcat(command, username);
	// sending command <!quit username>	
	ret = sendTCP(command, TCP_sock);
	if(ret != 0){
		printf("Error requestQuit --> sendTCP()\n");
		return 1;		
	}
	// waiting for server to be ready to close the connection
	ret = receiveTCP(server_reply, TCP_sock);
	if(ret != 0){
		printf("Error requestQuit --> receiveTCP()\n");
		return 1;		
	}		
	
	if( strcmp(server_reply, "ok") == 0){
		// return to main from executeCommand with 2, signaling the graceful
		// end of connection with the server
		printf("server ready to close connection\n");		
		return 2;	
	}
	
	printf("Error requestQuit()\n");
	return 1;
}
